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
#include <eixTk/sysutils.h>
#include <portage/basicversion.h>
#include <portage/package.h>
#include <output/formatstring.h>
#include <output/formatstring-print.h>

#include <dirent.h>

using namespace std;

inline static void sort_installed(map<string,vector<InstVersion> > *maping);

/** Find installed versions of packet "name" in category "category".
 * @return NULL if not found .. else pointer to vector of versions. */
vector<InstVersion> *VarDbPkg::getInstalledVector(const string &category, const string &name)
{
	map<string, map<string, vector<InstVersion> >* >::iterator map_it = installed.find(category);
	/* Not yet read */
	if(map_it == installed.end()) {
		readCategory(category.c_str());
		return getInstalledVector(category, name);
	}

	map<string, vector<InstVersion> >* installed_cat = map_it->second;
	/* No such category in db-directory. */
	if(!installed_cat)
		return NULL;

	/* Find packet */
	map<string, vector<InstVersion> >::iterator cat_it = installed_cat->find(name);
	if(cat_it == installed_cat->end())
		return NULL; /* Not installed */
	return &(cat_it->second);
}

/** Returns true if a Package installed. */
bool VarDbPkg::isInstalled(const Package &p, const BasicVersion *v)
{
	vector<InstVersion> *vec = getInstalledVector(p);
	if(vec)
	{
		if(!v)
			return true;
		for(vector<InstVersion>::size_type i = 0; i < vec->size(); ++i)
			if((*vec)[i] == *v)
				return true;
	}
	return false;
}

/** Returns number of installed versions of this package
 * @param p Check for this Package. */
vector<InstVersion>::size_type VarDbPkg::numInstalled(const Package &p)
{
	vector<InstVersion> *vec = getInstalledVector(p);
	if(!vec)
		return 0;
	return vec->size();
}

bool VarDbPkg::readSlot(const Package &p, InstVersion &v) const
{
	if(v.know_slot)
		return true;
	if(!get_slots)
		return false;
	if(v.read_failed)
		return false;
	try {
		vector<string> lines;
		if(!pushback_lines(
			(_directory + p.category + "/" + p.name + "-" + v.getFull() + "/SLOT").c_str(),
			&lines, true, false))
		{
			v.read_failed = true;
			return false;
		}
		if(!lines.size())
			v.slot = "";
		else if(lines[0] == "0")
			v.slot = "";
		else
			v.slot = lines[0];
		v.know_slot = true;
		return true;
	}
	catch(ExBasic e) {
		cerr << e << endl;
		v.read_failed = true;
		return false;
	}
}

bool VarDbPkg::readUse(const Package &p, InstVersion &v) const
{
	if(v.know_use)
		return true;
	v.know_use=true;
	v.iuse.clear();
	v.usedUse.clear();
	set<string> iuse_set;
	vector<string> alluse;
	try {
		string dirname = _directory + p.category + "/" + p.name + "-" + v.getFull();
		vector<string> lines;
		if(!pushback_lines((dirname + "/IUSE").c_str(),
			&lines, true, false))
			return false;
		v.iuse = split_string(join_vector(lines, " "));
		sort(v.iuse.begin(), v.iuse.end());
		v.iuse.erase(unique(v.iuse.begin(), v.iuse.end()), v.iuse.end());
		make_set(&iuse_set, v.iuse);
		/* If you do not want the alphabetical order in v.iuse
		   but instead the original order in the IUSE file,
		   use the following code instead of the above 4 lines.

		vector<string> iuse = split_string(join_vector(lines, " "));
		for(vector<string>::iterator it = iuse.begin();
			it != iuse.end(); ++it)
		{
			if(iuse_set.find(*it) != iuse_set.end())
				continue;
			iuse_set.insert(*it);
			v.iuse.push_back(*it);
		}
		*/
		lines.clear();
		if(!pushback_lines((dirname + "/USE").c_str(),
			&lines, true, false))
			return false;
		alluse = split_string(join_vector(lines, " "));
	}
	catch(ExBasic e) {
		cerr << e << endl;
		return false;
	}
	for(vector<string>::iterator it = alluse.begin();
		it != alluse.end(); ++it)
	{
		if(iuse_set.find(*it) != iuse_set.end()) {
			v.usedUse.insert(*it);
		}
	}
	return true;
}

/** Read category from db-directory. */
void VarDbPkg::readCategory(const char *category)
{
	/* Pointers to db and category DIRectory */
	DIR *dir_category;
	struct dirent* package_entry;  /* current package dirent */

	/* Open category-directory */
	string dir_category_name = _directory + category;
	if( (dir_category = opendir(dir_category_name.c_str())) == NULL) {
		installed[category] = NULL;
		return;
	}
	dir_category_name.append("/");
	map<string, vector<InstVersion> >* category_installed;
	installed[category] = category_installed = new map<string,vector<InstVersion> >;
	OOM_ASSERT(category_installed);

	/* Cycle through this category */
	while( (package_entry = readdir(dir_category)) != NULL )
	{

		if(package_entry->d_name[0] == '.')
			continue; /* Don't want dot-stuff */
		char **aux = ExplodeAtom::split( package_entry->d_name);
		InstVersion *instver = NULL;
		if(aux == NULL)
			continue;
		try {
			instver = new InstVersion(aux[1]);
		}
		catch(ExBasic e) {
			cerr << e << endl;
		}
		if(instver)
		{
			instver->instDate = get_mtime((dir_category_name + package_entry->d_name).c_str());
			(*category_installed)[aux[0]].push_back(*instver);
			free(instver);
		}

		free(aux[0]);
		free(aux[1]);
	}
	closedir(dir_category);
	sort_installed(installed[category]);
}

inline static void sort_installed(map<string,vector<InstVersion> > *maping)
{
	map<string,vector<InstVersion> >::iterator it = maping->begin();
	while(it != maping->end())
	{
		sort(it->second.begin(), it->second.end());
		++it;
	}
}
