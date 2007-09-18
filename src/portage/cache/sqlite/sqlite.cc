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


/** All stuff related to our  sqlite_callback() */

SqliteCache *SqliteCache::callback_arg;
#define THIS SqliteCache::callback_arg

#define ARGV(i) (argv[i] ? argv[i] : "")

int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	UNUSED(NotUsed); UNUSED(azColName);
#if 0
	for(int i = 0; i<argc; i++) {
		cout << i << ": " << azColName[i] << " = " <<  ARGV(i) << "\n";
	}
	return 0;
#endif
	// If an earlier error occurred, we ignore later calls:
	if(THIS->sqlite_callback_error)
		return 0;

	if(argc <= 1) {
		THIS->sqlite_callback_error = true;
		THIS->m_error_callback("Dataset does not contain a package name");
		return 0;
	}
	string category = ARGV(1);
	if(argc <= 17) {
		THIS->sqlite_callback_error = true;
		THIS->m_error_callback("Dataset for %s is too small", category.c_str());
		return 0;
	}
	string::size_type pos = category.find_first_of('/');
	if(pos == string::npos) {
		THIS->sqlite_callback_error = true;
		THIS->m_error_callback("'%s' not of the form package/category-version", category.c_str());
		return 0;
	}
	string name_ver = category.substr(pos + 1);
	category.resize(pos);
	// Does the category match?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(THIS->category) {
		dest_cat = THIS->category;
		if(dest_cat->name() != category)
			return 0;
	}
	else {
		dest_cat = THIS->packagetree->find(category);
		if(!dest_cat)
			return 0;
	}
	char **aux = ExplodeAtom::split(name_ver.c_str());
	if(aux == NULL)
	{
		THIS->m_error_callback("Can't split '%s' into package and version.", name_ver.c_str());
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
	version->slot = ARGV(6);
	string keywords = ARGV(12);
	version->set_full_keywords(keywords);
	string iuse = ARGV(14);
	version->set_iuse(iuse);
	pkg->addVersion(version);

	/* For the newest version, add all remaining data */
	if(*(pkg->latest()) == *version)
	{
		pkg->homepage = ARGV(9);
		pkg->licenses = ARGV(10);
		pkg->desc     = ARGV(11);
		pkg->provide  = ARGV(17);
	}
	/* Free old split */
	free(aux[0]);
	free(aux[1]);
	return 0;
}

bool SqliteCache::readCategories(PackageTree *pkgtree, vector<string> *categories, Category *cat) throw(ExBasic)
{
	if(cat)
	{
		pkgtree = NULL;
		categories = NULL;
	}
	char *errormessage = NULL;
	string sqlitefile = m_prefix + PORTAGE_CACHE_PATH + m_scheme;
	// Cut all trailing '/' and append ".sqlite" to the name
	string::size_type pos = sqlitefile.find_last_not_of('/');
	if(pos == string::npos)
	{
		m_error_callback("Database path incorrect");
		return false;
	}
	sqlitefile.resize(pos + 1);
	sqlitefile.append(".sqlite");

	sqlite3 *db;
	int rc = sqlite3_open(sqlitefile.c_str(), &db);
	if(rc)
	{
		sqlite3_close(db);
		m_error_callback("Can't open cache file %s",
			sqlitefile.c_str());
		return false;
	}
	if(pkgtree)
		pkgtree->need_fast_access(categories);
	callback_arg = this;
	sqlite_callback_error = false;
	packagetree = pkgtree;
	category = cat;
	rc = sqlite3_exec(db, "select * from portage_packages", sqlite_callback, 0, &errormessage);
	sqlite3_close(db);
	if(pkgtree)
		pkgtree->finish_fast_access();
	if(rc != SQLITE_OK) {
		sqlite_callback_error = true;
		m_error_callback("sqlite error: %s", errormessage);
	}
	return !sqlite_callback_error;
}

#else /* Not WITH_SQLITE */

using namespace std;

bool SqliteCache::readCategories(PackageTree *packagetree, vector<string> *categories, Category *category) throw(ExBasic)
{
	UNUSED(packagetree);
	UNUSED(category);
	UNUSED(categories);
	m_error_callback("Cache method sqlite is not compiled in.\n"
	"Recompile eix, using configure option --with-sqlite to add sqlite support.");
	return false;
}

#endif
