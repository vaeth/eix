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

#include "vardbpkg.h"

#include <eixTk/stringutils.h>
#include <eixTk/utils.h>
#include <portage/basicversion.h>
#include <portage/package.h>
#include <output/formatstring.h>
#include <output/formatstring-print.h>

#include <dirent.h>

using namespace std;

inline static void sort_installed(map<string,vector<BasicVersion> > *maping);

/** Find installed versions of packet "name" in category "category".
 * @return NULL if not found .. else pointer to vector of versions. */
vector<BasicVersion> *VarDbPkg::getInstalledVector(const string &category, const string &name)
{
	map<string, map<string, vector<BasicVersion> >* >::iterator map_it = installed.find(category);
	/* Not yet read */
	if(map_it == installed.end()) {
		readCategory(category.c_str());
		return getInstalledVector(category, name);
	}

	map<string, vector<BasicVersion> >* installed_cat = map_it->second;
	/* No such category in db-directory. */
	if(!installed_cat)
		return NULL;

	/* Find packet */
	map<string, vector<BasicVersion> >::iterator cat_it = installed_cat->find(name);
	if(cat_it == installed_cat->end())
		return NULL; /* Not installed */
	return &(cat_it->second);
}

/** Returns true if a Package installed. */
bool VarDbPkg::isInstalled(const Package &p, const BasicVersion *v)
{
	vector<BasicVersion> *vec = getInstalledVector(p);
	if(vec)
	{
		if(!v)
			return true;
		for(unsigned int i = 0; i < vec->size(); ++i)
			if((*vec)[i] == *v)
				return true;
	}
	return false;
}

/** Returns number of installed versions of this package
 * @param p Check for this Package. */
vector<BasicVersion>::size_type VarDbPkg::numInstalled(const Package &p)
{
	vector<BasicVersion> *vec = getInstalledVector(p);
	if(!vec)
		return 0;
	return vec->size();
}

/** Read category from db-directory. */
void VarDbPkg::readCategory(const char *category)
{
	/* Pointers to db and category DIRectory */
	DIR *dir_category;
	struct dirent* package_entry;  /* current package dirent */

	/* Open category-directory */
	string dirname = _directory + category;
	if( (dir_category = opendir(dirname.c_str())) == NULL) {
		installed[category] = NULL;
		return;
	}
	map<string, vector<BasicVersion> >* category_installed;
	installed[category] = category_installed = new map<string,vector<BasicVersion> >;
	OOM_ASSERT(category_installed);

	dirname.append("/");

	/* Cycle through this category */
	while( (package_entry = readdir(dir_category)) != NULL )
	{

		if(package_entry->d_name[0] == '.')
			continue; /* Don't want dot-stuff */
		char **aux = ExplodeAtom::split( package_entry->d_name);
		if(aux == NULL)
			continue;
		try {
			BasicVersion version(aux[1]);
			if(_have_slots)
			{
				vector<string> lines;
				if(pushback_lines(
					(dirname + package_entry->d_name + "/SLOT").c_str(),
					&lines, true, false))
				{
					if(lines.size())
					{
						if(lines[0] != "0")
							version.slot = lines[0];
					}
					(*category_installed)[aux[0]].push_back(version);
				}
			}
			else
				(*category_installed)[aux[0]].push_back(version);
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

inline static void sort_installed(map<string,vector<BasicVersion> > *maping)
{
	map<string,vector<BasicVersion> >::iterator it = maping->begin();
	while(it != maping->end())
	{
		sort(it->second.begin(), it->second.end());
		++it;
	}
}
