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

#include "none.h"

#include <cache-utils/selectors.h>
#include <cache-utils/flat-reader.h>
#include <varsreader.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <dirent.h>

#include <config.h>

using namespace std;

void NoneCache::readPackage(Category &vec, const char *pkg_name, string *directory_path, struct dirent **list, int numfiles) throw(ExBasic)
{
	bool have_onetime_info = false;

	Package *pkg = vec.findPackage(pkg_name);
	if( pkg )
		have_onetime_info = true;
	else
		pkg = vec.addPackage(pkg_name);

	for(int i = 0; i < numfiles; ++i)
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
		catch(ExBasic e) {
			cerr << "Problems with reading " << ebuild_name <<
				":\n" << e << endl;
		}

		version->set(m_arch, ebuild["KEYWORDS"]);
		version->slot = ebuild["SLOT"];
		version->set_iuse(ebuild["IUSE"]);
		pkg->addVersionFinalize(version);
		if(read_onetime_info)
		{
			pkg->homepage = ebuild["HOMEPAGE"];
			pkg->licenses = ebuild["LICENSE"];
			pkg->desc     = ebuild["DESCRIPTION"];
			pkg->provide  = ebuild["PROVIDE"];

			have_onetime_info = true;
		}
		free(ver);
	}

	if(!have_onetime_info) {
		vec.deletePackage(pkg_name);
	}
}

bool NoneCache::readCategory(Category &vec) throw(ExBasic)
{
	struct dirent **packages= NULL;

	string catpath = m_prefix + m_scheme + "/" + vec.name();
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
			try {
				readPackage(vec, packages[i]->d_name, &pkg_path, files, numfiles);
			}
			catch(ExBasic e) {
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
