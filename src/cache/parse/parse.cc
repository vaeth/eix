// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "parse.h"

#include <cache/common/selectors.h>
#include <cache/common/flat-reader.h>
#include <varsreader.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>
#include <eixTk/stringutils.h>
#include <eixTk/formated.h>

#include <dirent.h>

#include <config.h>

using namespace std;

void
ParseCache::set_checking(string &str, const char *item, const VarsReader &ebuild, bool *ok)
{
	bool check = (ebuild_exec && ok && (*ok));
	const string *s = ebuild.find(item);
	if(!s) {
		str.clear();
		if(check)
			*ok = false;
		return;
	}
	str = *s;
	if(!check)
		return;
	if((str.find_first_of('`') != string::npos) ||
		(str.find("$(") != string::npos))
		*ok = false;
}

void
ParseCache::readPackage(Category &vec, const char *pkg_name, string *directory_path, struct dirent **list, size_t numfiles) throw(ExBasic)
{
	bool have_onetime_info = false;

	Package *pkg = vec.findPackage(pkg_name);
	if( pkg )
		have_onetime_info = true;
	else
		pkg = vec.addPackage(pkg_name);

	for(size_t i = 0; i < numfiles; ++i)
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
		string ebuild_name = (*directory_path) + '/' + list[i]->d_name;

		/* Make version and add it to package. */
		Version *version = new Version(ver);
		pkg->addVersionStart(version);
		/* For the latest version read/change corresponding data */
		bool read_onetime_info = true;
		if( have_onetime_info )
			if(*(pkg->latest()) != *version)
				read_onetime_info = false;
		/* read the ebuild */
		VarsReader::Flags flags = VarsReader::NONE;
		if(!read_onetime_info)
			flags |= VarsReader::ONLY_KEYWORDS_SLOT;
		map<string, string> env;
		if(!nosubst)
		{
			flags |= VarsReader::INTO_MAP | VarsReader::SUBST_VARS;
			env_add_package(env, *pkg, *version, *directory_path, ebuild_name.c_str());
		}
		VarsReader ebuild(flags);
		if(flags & VarsReader::INTO_MAP)
			ebuild.useMap(&env);
		version->overlay_key = m_overlay_key;
		try {
			ebuild.read(ebuild_name.c_str());
		}
		catch(const ExBasic &e) {
			m_error_callback(eix::format("Could not properly execute %s") % ebuild_name
				% ebuild_name % ":\n" % e);
		}

		bool ok = true;
		string keywords, restr, iuse;
		set_checking(keywords, "KEYWORDS", ebuild, &ok);
		set_checking(version->slotname, "SLOT", ebuild, &ok);
		set_checking(restr, "RESTRICT", ebuild);
		set_checking(iuse, "IUSE", ebuild, &ok);
		if(read_onetime_info)
		{
			set_checking(pkg->homepage, "HOMEPAGE",    ebuild, &ok);
			set_checking(pkg->licenses, "LICENSE",     ebuild, &ok);
			set_checking(pkg->desc,     "DESCRIPTION", ebuild, &ok);
			set_checking(pkg->provide,  "PROVIDE",     ebuild);

			have_onetime_info = true;
		}
		if(!ok) {
			string *cachefile = ebuild_exec->make_cachefile(ebuild_name.c_str(), *directory_path, *pkg, *version);
			if(cachefile) {
				flat_get_keywords_slot_iuse_restrict(cachefile->c_str(), keywords, version->slotname, iuse, restr, m_error_callback);
				flat_read_file(cachefile->c_str(), pkg, m_error_callback);
				ebuild_exec->delete_cachefile();
			}
			else
				m_error_callback(eix::format("Could not properly execute %s") % ebuild_name);
		}

		version->set_full_keywords(keywords);
		version->set_restrict(restr);
		version->set_iuse(iuse);
		pkg->addVersionFinalize(version);

		free(ver);
	}

	if(!have_onetime_info) {
		vec.deletePackage(pkg_name);
	}
}

bool ParseCache::readCategory(Category &vec) throw(ExBasic)
{
	struct dirent **packages= NULL;

	string catpath = m_prefix + m_scheme + "/" + vec.name();
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
			try {
				readPackage(vec, packages[i]->d_name, &pkg_path, files, numfiles);
			}
			catch(const ExBasic &e) {
				cerr << "Error while reading " << pkg_path << ":\n" << e << endl;
			}
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
