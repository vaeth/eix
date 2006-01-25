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

#include <varsreader.h>
#include <eixTk/stringutils.h>

#include <dirent.h>

#include <config.h>

using namespace std;

static int package_seclector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strcmp(dent->d_name, "CVS") != 0);
}

static int ebuild_seclector (SCANDIR_ARG3 dent)
{
		return package_seclector(dent);
}

void NoneCache::readPackage(vector<Package*> &vec, const string &cat_name, char *pkg_name, string *directory_path, struct dirent **list, int numfiles, void (*error_callback)(const char *fmt, ...))
{
	bool have_onetime_info = false;

	Package *pkg = findPackage(vec, pkg_name);
	if( (pkg) )
		have_onetime_info = true;
	else
		pkg = addPackage(vec, cat_name, pkg_name);

	for(int i = 0; i<numfiles; ++i)
	{
		/* Check if this is an ebuild  */
		char* dotptr = strrchr(list[i]->d_name, '.');
		if( !(dotptr) || strcmp(dotptr,".ebuild") )
			continue;

		/* read the ebuild */
		VarsReader ebuild((have_onetime_info ? VarsReader::ONLY_KEYWORDS : VarsReader::NONE));
		ebuild.read((*directory_path + "/" + list[i]->d_name).c_str());


		/* now we don't need the filename anymore, so we can cut off the .ebuild */
		*dotptr = '\0';

		/* Check if we can split it */
		char* ver = ExplodeAtom::split_version(list[i]->d_name);
		if(ver == NULL) {
			error_callback("Can't split filename of ebuild %s/%s.", directory_path->c_str(), list[i]->d_name);
			continue;
		}

		/* only copy ebuild infos once */
		if( !(have_onetime_info) )
		{
			pkg->homepage = ebuild["HOMEPAGE"];
			pkg->licenses = ebuild["LICENSE"];
			pkg->desc     = ebuild["DESCRIPTION"];
			pkg->provide  = ebuild["PROVIDE"];

			have_onetime_info = true;
		}

		/* Make version and add it to package. */
		Version *version = new Version(ver);
		version->overlay_key = _overlay_key;
		version->set(_arch, ebuild["KEYWORDS"]);
		pkg->addVersion(version);
		free(ver);
	}

	if(!have_onetime_info) {
		deletePackage(vec, pkg_name);
	}
}

int NoneCache::readCategory(vector<Package*> &vec, const string &cat_name, void (*error_callback)(const char *fmt, ...))
{
	struct dirent **packages= NULL;

	string catpath = _scheme + "/" + cat_name; 
	int numpackages = scandir(catpath.c_str(),
			&packages, package_seclector, alphasort);

	for(int i = 0; i<numpackages; ++i)
	{
		struct dirent **files = NULL;
		string pkg_path = catpath + "/" + packages[i]->d_name;

		int numfiles = scandir(pkg_path.c_str(),
				&files, ebuild_seclector, alphasort);
		if(numfiles > 0)
		{
			readPackage(vec, cat_name, (char *) packages[i]->d_name, &pkg_path, files, numfiles, error_callback);
			for(int i=0; i<numfiles; i++ )
				free(files[i]);
			free(files);
		}
	}

	for(int i=0; i<numpackages; i++ )
		free(packages[i]);
	if(numpackages > 0)
		free(packages);

	return 1;
}
