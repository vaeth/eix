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

#include "vardbpkg.h"

#include <eixTk/stringutils.h>
#include <portage/basicversion.h>
#include <portage/package.h>

#include <dirent.h>

inline bool basicVersion_comparator(const BasicVersion &v1, const BasicVersion &v2);
inline static void sort_installed(map<string,vector<BasicVersion> > *maping);

/** Find installed versions of packet "name" in category "category".
 * @return NULL if not found .. else pointer to vector of versions. */
vector<BasicVersion> *VarDbPkg::getInstalledVector(string category, string name)
{
	map<string, map<string, vector<BasicVersion> >* >::iterator map_it = installed.find(category);
	/* Not yet read */
	if(map_it == installed.end()) {
		readCategory(category);
		return getInstalledVector(category, name);
	}

	/* No such category in db-directory. */
	if((*map_it).second == NULL)
		return NULL;

	map<string, vector<BasicVersion> >* installed_cat = (*map_it).second;
	/* Find paket */
	map<string, vector<BasicVersion> >::iterator cat_it = installed_cat->find(name);
	if(cat_it == installed_cat->end())
		return NULL; /* Not installed */
	return &(cat_it->second);
}

/** Return a pointer to internal string representing all installed versions.
 * @param p Get installed versions of this package.
 * @return all installed versions concatenated to a emptry string. */
string VarDbPkg::getInstalledString(Package *p)
{
	p->readNeeded(Package::NAME);
	vector<BasicVersion> *vec = getInstalledVector(p->category, p->name);
	if(vec == 0) {
		return (string(""));
	}

	string ret;
	vector<BasicVersion>::iterator it = vec->begin();
	if(it == vec->end()) {
		return "";
	}
	for(;;) {
		ret.append(it->toString());
		if(++it == vec->end()) {
			break;
		}
		ret.append(" ");
	}
	return ret;
}

/** Returns true if a Package installed. */
bool VarDbPkg::isInstalled(Package *p, BasicVersion *v)
{
	p->readNeeded(Package::NAME);
	if(v == NULL)
		return ( getInstalledVector( p->category, p->name ) );

	vector<BasicVersion> *vec = getInstalledVector( p->category, p->name );
	for(unsigned int i = 0; i < vec->size(); ++i)
		if((*vec)[i] == *v)
			return true;
	return false;
}

/** Read category from db-directory. */
void VarDbPkg::readCategory(string category)
{
	/* Pointers to db and category DIRectory */
	DIR *dir_category;
	struct dirent* package_entry;  /* current package dirent */

	/* Open category-directory */
	if( (dir_category = opendir((_directory + category).c_str())) == NULL) {
		installed[category] = NULL;
		return;
	}
	map<string, vector<BasicVersion> >* category_installed;
	installed[category] = category_installed = new map<string,vector<BasicVersion> >;
	OOM_ASSERT(category_installed);

	/* Cycle through this category */
	while( (package_entry = readdir(dir_category)) != NULL )
	{
		if(package_entry->d_name[0] == '.')
			continue; /* Don't want dot-stuff */
		char **aux = ExplodeAtom::split( package_entry->d_name);
		if(aux == NULL)
			continue;
		try {
			(*category_installed)[aux[0]].push_back(BasicVersion(aux[1]));
		}
		catch(ExBasic e) {
			cerr << e << endl;
		}

		free(aux[0]);
		free(aux[1]);
	}
	closedir(dir_category);
	sort_installed(installed[category]);
}

inline bool basicVersion_comparator(const BasicVersion &v1, const BasicVersion &v2)
{
	return ((BasicVersion)v1 < (BasicVersion)v2);
}

inline static void sort_installed(map<string,vector<BasicVersion> > *maping)
{
	map<string,vector<BasicVersion> >::iterator it = maping->begin();
	while(it != maping->end())
	{
		sort(it->second.begin(), it->second.end(), basicVersion_comparator);
		++it;
	}
}
