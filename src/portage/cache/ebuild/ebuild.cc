/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ebuild.h"
#include <cache-utils/selectors.h>
#include <cache-utils/flat-reader.h>

#include <eixTk/stringutils.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <map>

#include <dirent.h>
#include <unistd.h>

#include <config.h>
#include <signal.h>
#include <sys/wait.h>

using namespace std;

const char *EBUILD_SH_EXEC     = "/usr/lib/portage/bin/ebuild.sh";
const char *EBUILD_EXEC        = "/usr/bin/ebuild";
const char *EBUILD_DEPEND_TEMP = "/var/cache/edb/dep/aux_db_key_temp";

EbuildCache *EbuildCache::handler_arg;
void
ebuild_sig_handler(int sig)
{
	EbuildCache::handler_arg->delete_cachefile();
}

// Take care:
// Since handler_arg is static, add_handler will not be reentrant,
// even for different instances of EbuildCache.
// However, as long as add_handler() and remove_handler() are
// only called internally within the same public function
// of EbuildCache (and EbuildCache does not call other instances),
// this is not a problem from "outside" this class.

void EbuildCache::add_handler()
{
	handler_arg = this;
	// Set the signals "empty" to avoid a race condition:
	// On a signal, we should cleanup only the signals actually set.
	handleHUP  = handleINT = handleTERM = ebuild_sig_handler;
	have_set_signals = true;
	handleHUP  = signal(SIGHUP, ebuild_sig_handler);
	if(handleHUP == SIG_IGN)
		signal(SIGHUP, SIG_IGN);
	handleINT  = signal(SIGINT, ebuild_sig_handler);
	if(handleINT == SIG_IGN)
		signal(SIGINT, SIG_IGN);
	handleTERM = signal(SIGTERM, ebuild_sig_handler);
	if(handleTERM == SIG_IGN)
		signal(SIGTERM, SIG_IGN);
}

void EbuildCache::remove_handler()
{
	if(!have_set_signals)
		return;
	signal(SIGHUP,  handleHUP);
	signal(SIGINT,  handleINT);
	signal(SIGTERM, handleTERM);
	have_set_signals = false;
}

// You should have called add_handler() in advance
bool EbuildCache::make_tempfile()
{
	char *temp = (char *)malloc(50 * sizeof(char));
	if(!temp)
		return false;
	strcpy(temp, "/tmp/ebuild-cache.XXXXXX");
	int fd = mkstemp(temp);
	if(fd == -1)
		return false;
	cachefile = new string(temp);
	free(temp);
	close(fd);
	return true;
}

void EbuildCache::delete_cachefile()
{
	if(!cachefile)
		return;
	if(unlink(cachefile->c_str())<0)
		m_error_callback("Can't unlink %s", cachefile->c_str());
	cachefile = NULL;
	remove_handler();
}

bool EbuildCache::make_cachefile(const char *name, const string &dir, const Package &package, const Version &version)
{
	map<string, string> env;
	add_handler();
	if(use_ebuild_sh)
	{
		if(!make_tempfile())
		{
			remove_handler();
			return false;
		}
		env_add_package(env, package, version, dir, name);
		env["dbkey"] = *cachefile;
	}
	else
		cachefile = new string(m_prefix_exec + EBUILD_DEPEND_TEMP);
	pid_t child = vfork();
	if(child == -1) {
		m_error_callback("Forking failed");
		return false;
	}
	if(child == 0)
	{
		if(!use_ebuild_sh)
		{
			string ebuild = m_prefix_exec + EBUILD_EXEC;
			execl(ebuild.c_str(), ebuild.c_str(), name, "depend", NULL);
			exit(2);
		}
		const char **myenv = (const char **)malloc((env.size() + 1) * sizeof(const char *));
		const char **ptr = myenv;
		for(map<string, string>::const_iterator it = env.begin();
			it != env.end(); ++it, ++ptr) {
			string *s = new string((it->first) + '=' + (it->second));
			*ptr = s->c_str();
		}
		*ptr = NULL;
		string ebuild_sh = m_prefix_exec + EBUILD_SH_EXEC;
		execle(ebuild_sh.c_str(), ebuild_sh.c_str(), "depend", NULL, myenv);
		exit(2);
	}
	int exec_status;
	while( waitpid( child, &exec_status, 0) != child );
	if(exec_status)
	{
		delete_cachefile();
		return false;
	}
	return true;
}

void EbuildCache::readPackage(Category &vec, char *pkg_name, string *directory_path, struct dirent **list, int numfiles) throw(ExBasic)
{
	bool have_onetime_info = false;

	Package *pkg = vec.findPackage(pkg_name);
	if( pkg )
		have_onetime_info = true;
	else
		pkg = vec.addPackage(pkg_name);

	for(int i = 0; i<numfiles; ++i)
	{
		/* Check if this is an ebuild  */
		char* dotptr = strrchr(list[i]->d_name, '.');
		if( !(dotptr) || strcmp(dotptr,".ebuild") )
			continue;

		/* For splitting the version, we cut off the .ebuild */
		*dotptr = '\0';
		/* Check if we can split it */
		char* ver = ExplodeAtom::split_version(list[i]->d_name);
		/* Restore the old value */
		*dotptr = '.';
		if(ver == NULL) {
			m_error_callback("Can't split filename of ebuild %s/%s.", directory_path->c_str(), list[i]->d_name);
			continue;
		}

		/* Make version and add it to package. */
		Version *version = new Version(ver);
		pkg->addVersionStart(version);

		/* Exectue the external program to generate cachefile */
		string full_path = *directory_path + '/' + list[i]->d_name;
		if(!make_cachefile(full_path.c_str(), *directory_path, *pkg, *version))
		{
			m_error_callback("Could not properly execute %s", full_path.c_str());
			continue;
		}

		/* For the latest version read/change corresponding data */
		bool read_onetime_info = true;
		if( have_onetime_info )
			if(*(pkg->latest()) != *version)
				read_onetime_info = false;
		version->overlay_key = m_overlay_key;
		string keywords, iuse;
		try {
			flat_get_keywords_slot_iuse(cachefile->c_str(), keywords, version->slot, iuse, m_error_callback);
			version->set(m_arch, keywords);
			version->set_iuse(iuse);
			if(read_onetime_info)
			{
				flat_read_file(cachefile->c_str(), pkg, m_error_callback);
				have_onetime_info = true;
			}
		}
		catch(ExBasic e) {
			cerr << "Executing " << full_path <<
				" did not produce all data.";
			// We keep the version anyway, even with wrong keywords/slots/infos:
			have_onetime_info = true;
		}
		pkg->addVersionFinalize(version);
		free(ver);
		delete_cachefile();
	}

	if(!have_onetime_info) {
		vec.deletePackage(pkg_name);
	}
}

bool EbuildCache::readCategory(Category &vec) throw(ExBasic)
{
	struct dirent **packages= NULL;

	string catpath = m_prefix + m_scheme + '/' + vec.name();
	int numpackages = scandir(catpath.c_str(),
			&packages, package_selector, alphasort);

	for(int i = 0; i < numpackages; ++i)
	{
		struct dirent **files = NULL;
		string pkg_path = catpath + '/' + packages[i]->d_name;

		int numfiles = scandir(pkg_path.c_str(),
				&files, ebuild_selector, alphasort);
		if(numfiles > 0)
		{
			readPackage(vec, (char *) packages[i]->d_name, &pkg_path, files, numfiles);
			for(int i=0; i<numfiles; i++ )
				free(files[i]);
			free(files);
		}
	}

	for(int i = 0; i < numpackages; i++ )
		free(packages[i]);
	if(numpackages > 0)
		free(packages);

	return true;
}

