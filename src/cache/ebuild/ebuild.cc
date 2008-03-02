// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "ebuild.h"
#include <cache/common/selectors.h>
#include <cache/common/flat-reader.h>

#include <eixTk/stringutils.h>
#include <eixTk/formated.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <map>

#include <dirent.h>
#include <unistd.h>

using namespace std;

void
EbuildCache::readPackage(Category &vec, const char *pkg_name, string *directory_path, struct dirent **list, int numfiles) throw(ExBasic)
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
			m_error_callback(eix::format("Can't split filename of ebuild %s/%s") %
				(*directory_path) % list[i]->d_name);
			continue;
		}

		/* Make version and add it to package. */
		Version *version = new Version(ver);
		pkg->addVersionStart(version);

		/* Exectue the external program to generate cachefile */
		string full_path = *directory_path + '/' + list[i]->d_name;
		string *cachefile = ebuild_exec.make_cachefile(full_path.c_str(), *directory_path, *pkg, *version);
		if(!cachefile)
		{
			m_error_callback(eix::format("Could not properly execute %s") % full_path);
			continue;
		}

		/* For the latest version read/change corresponding data */
		bool read_onetime_info = true;
		if( have_onetime_info )
			if(*(pkg->latest()) != *version)
				read_onetime_info = false;
		version->overlay_key = m_overlay_key;
		string keywords, iuse, restr;
		try {
			flat_get_keywords_slot_iuse_restrict(cachefile->c_str(), keywords, version->slotname, iuse, restr, m_error_callback);
			version->set_full_keywords(keywords);
			version->set_iuse(iuse);
			version->set_restrict(restr);
			if(read_onetime_info)
			{
				flat_read_file(cachefile->c_str(), pkg, m_error_callback);
				have_onetime_info = true;
			}
		}
		catch(const ExBasic &e) {
			cerr << "Executing " << full_path <<
				" did not produce all data.";
			// We keep the version anyway, even with wrong keywords/slots/infos:
			have_onetime_info = true;
		}
		pkg->addVersionFinalize(version);
		free(ver);
		ebuild_exec.delete_cachefile();
	}

	if(!have_onetime_info) {
		vec.deletePackage(pkg_name);
	}
}

bool
EbuildCache::readCategory(Category &vec) throw(ExBasic)
{
	struct dirent **packages= NULL;

	string catpath = m_prefix + m_scheme + '/' + vec.name();
	int numpackages = my_scandir(catpath.c_str(),
			&packages, package_selector, alphasort);

	for(int i = 0; i < numpackages; ++i)
	{
		struct dirent **files = NULL;
		string pkg_path = catpath + '/' + packages[i]->d_name;

		int numfiles = my_scandir(pkg_path.c_str(),
				&files, ebuild_selector, alphasort);
		if(numfiles > 0)
		{
			readPackage(vec, packages[i]->d_name, &pkg_path, files, numfiles);
			for(int j=0; j<numfiles; j++ )
				free(files[j]);
			free(files);
		}
	}

	for(int i = 0; i < numpackages; i++ )
		free(packages[i]);
	if(numpackages > 0)
		free(packages);

	return true;
}

