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

#include "sqlite.h"

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <config.h>


#if defined(WITH_SQLITE)

#include <sqlite3.h>

using namespace std;

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"

/** This variable is actually the parameter for the sqlite3-callback function */

SqliteCache *SqliteCache::callback_arg;

inline const char *has_category(const string &category, const char *cat_name)
{
	for(const char *s = category.c_str(); *s; ++s, ++cat_name) {
		if(*s != *cat_name)
			return NULL;
	}
	if(*cat_name == '/')
		return ++cat_name;
	return NULL;
}

int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
#if 0
	for(int i = 0; i<argc; i++) {
		cout << i << ": " << azColName[i] << " = " <<  ( argv[i] ? argv[i] : "NULL" ) << "\n";
	}
	return 0;
#endif
	if(!argv[1])
		return 0;
	string category = argv[1];
	string::size_type pos = category.find_first_of('/');
	if(pos == string::npos)
		return 0;
	string name_ver = category.substr(pos + 1);
	category.resize(pos);
	// Does the category match?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(SqliteCache::callback_arg->category) {
		dest_cat = SqliteCache::callback_arg->category;
		if(dest_cat->name() != category)
			return 0;
	}
	else {
		dest_cat = SqliteCache::callback_arg->packagetree->find(category);
		if(!dest_cat)
			return 0;
	}
	char **aux = ExplodeAtom::split(name_ver.c_str());
	if(aux == NULL)
	{
		SqliteCache::callback_arg->m_error_callback("Can't split '%s' into package and version.", name_ver.c_str());
		return 0;
	}
	/* Search for existing package */
	Package *pkg = dest_cat->findPackage(aux[0]);

	/* If none was found create one */
	if(pkg == NULL)
		pkg = dest_cat->addPackage(aux[0]);

	/* Create a new version and add it to package */
	Version *version = new Version(aux[1]);
	// reading slots and stability
	version->slot = argv[6];
	string keywords = argv[12];
	version->set(SqliteCache::callback_arg->m_arch, keywords);
	pkg->addVersion(version);

	/* For the newest version, add all remaining data */
	if(*(pkg->latest()) == *version)
	{
		pkg->homepage = argv[9];
		pkg->licenses = argv[10];
		pkg->desc     = argv[11];
		pkg->provide  = argv[17];
	}
	/* Free old split */
	free(aux[0]);
	free(aux[1]);
	return 0;
}

int SqliteCache::readCategories(PackageTree *pkgtree, vector<string> *categories, Category *cat) throw(ExBasic)
{
	if(cat)
	{
		pkgtree = NULL;
		categories = NULL;
	}
	char *errormessage = NULL;
	string sqlitefile = PORTAGE_CACHE_PATH + m_scheme;
	// Cut all trailing '/' and append ".sqlite" to the name
	string::size_type pos = sqlitefile.find_last_not_of('/');
	if(pos == string::npos)
		return -1;
	sqlitefile.resize(pos + 1);
	sqlitefile.append(".sqlite");

	sqlite3 *db;
	int rc = sqlite3_open(sqlitefile.c_str(), &db);
	if(rc)
	{
		sqlite3_close(db);
		return -1;
	}
	if(pkgtree)
		pkgtree->need_fast_access(categories);
	callback_arg = this;
	packagetree = pkgtree;
	category = cat;
	rc = sqlite3_exec(db, "select * from portage_packages", sqlite_callback, 0, &errormessage);
	sqlite3_close(db);
	if(pkgtree)
		pkgtree->finish_fast_access();
	if(rc != SQLITE_OK)
		throw(ExBasic("sqlite error: %s", errormessage));
	return 1;
}

#else /* Not WITH_SQLITE */

using namespace std;

int SqliteCache::readCategories(PackageTree *packagetree, vector<string> *categories, Category *category) throw(ExBasic)
{
	throw(ExBasic("Cache method sqlite is not compiled in.\n"
	"Recompile eix, using configure option --with-sqlite to add sqlite support."));
	return -1;
}

#endif
