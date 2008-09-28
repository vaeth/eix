// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "sqlite.h"

#include <config.h>

#if defined(WITH_SQLITE)

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <eixTk/formated.h>

#include <sqlite3.h>

#include <map>

using namespace std;

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"


/** All stuff related to our  sqlite_callback() */

SqliteCache *SqliteCache::callback_arg;
#define THIS SqliteCache::callback_arg


/** The following is all related to get the proper index for the lookups */

typedef enum {
	SQ_NAME,
	SQ_SLOT,
	SQ_RESTRICT,
	SQ_HOMEPAGE,
	SQ_LICENSE,
	SQ_DESCRIPTION,
	SQ_KEYWORDS,
	SQ_IUSE,
	SQ_PROVIDE,
	SQ_PROPERTIES,
	SQ_LAST
} mynames;

static map<string,int> namemap;
static vector<int> default_trueindex(SQ_LAST, 0);

inline static void
init_data(int true_index, int my_index, const char *s)
{
	namemap[s] = my_index;
	default_trueindex[my_index] = true_index;
}

inline static void
init_namemap()
{
	init_data( 1, SQ_NAME,        "portage_package_key");
	init_data( 6, SQ_SLOT,        "SLOT");
	init_data( 8, SQ_RESTRICT,    "RESTRICT");
	init_data( 9, SQ_HOMEPAGE,    "HOMEPAGE");
	init_data(10, SQ_LICENSE,     "LICENSE");
	init_data(11, SQ_DESCRIPTION, "DESCRIPTION");
	init_data(12, SQ_KEYWORDS,    "KEYWORDS");
	init_data(14, SQ_IUSE,        "IUSE");
	init_data(17, SQ_PROVIDE,     "PROVIDE");
	init_data(19, SQ_PROPERTIES,  "PROPERTIES");
}

inline static int
calc_trueindex(int argc, char **azColName, vector<int> &trueindex)
{
	if(namemap.empty())
		init_namemap();
	trueindex = default_trueindex;
	for(int i = 0; i < argc; ++i) {
		map<string,int>::const_iterator it = namemap.find(azColName[i]);
		if(it != namemap.end())
			trueindex[it->second] = i;
	}
	int maxindex = -1;
	for(int i = 0; i < SQ_LAST; ++i) {
		int curr = trueindex[i];
		// Shortcut if we have not reached the maximum
		if(maxindex >= curr)
			continue;
		// Is the true index out of range?
		if(argc <= curr) {
			trueindex[i] = -1;
			// PROPERTIES may be missing - this is ok
			if(i == SQ_PROPERTIES)
				continue;
		}
		maxindex = trueindex[i];
	}
	return maxindex;
}

inline static const char *
sq_getarg(char **argv, vector<int> &trueindex, const int i)
{
	int t = trueindex[i];
	if(t >= 0) { // index in range?
		const char *r = argv[t];
		if(r)
			return r;
	}
	return "";
}
#define ARGV(i) sq_getarg(argv,trueindex,i)

int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	UNUSED(NotUsed);
#if 0
	for(int i = 0; i<argc; ++i) {
		cout << i << ": " << azColName[i] << " = " <<  ARGV(i) << "\n";
	}
	return 0;
#endif
	// If an earlier error occurred, we ignore later calls:
	if(THIS->sqlite_callback_error)
		return 0;

	vector<int> &trueindex = THIS->trueindex;
	int maxindex = THIS->maxindex;
	if(!maxindex) {
		maxindex = calc_trueindex(argc, azColName, trueindex);
		THIS->maxindex = maxindex;
	}

	if(argc <= trueindex[SQ_NAME]) {
		THIS->sqlite_callback_error = true;
		THIS->m_error_callback("Dataset does not contain a package name");
		return 0;
	}
	string category = ARGV(SQ_NAME);
	if(argc <= maxindex) {
		THIS->sqlite_callback_error = true;
		THIS->m_error_callback(eix::format("Dataset for %s is too small") % category);
		return 0;
	}
	string::size_type pos = category.find_first_of('/');
	if(pos == string::npos) {
		THIS->sqlite_callback_error = true;
		THIS->m_error_callback(eix::format("%r not of the form package/category-version") % category);
		return 0;
	}
	string name_ver = category.substr(pos + 1);
	category.resize(pos);
	// Does the category match?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(THIS->category) {
		dest_cat = THIS->category;
		if(dest_cat->name != category)
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
		THIS->m_error_callback(eix::format("Can't split %r into package and version") % name_ver);
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
	version->slotname = ARGV(SQ_SLOT);
	version->set_restrict(ARGV(SQ_RESTRICT));
	version->set_properties(ARGV(SQ_PROPERTIES));
	string keywords = ARGV(SQ_KEYWORDS);
	version->set_full_keywords(keywords);
	string iuse = ARGV(SQ_IUSE);
	version->set_iuse(iuse);
	pkg->addVersion(version);

	/* For the newest version, add all remaining data */
	if(*(pkg->latest()) == *version)
	{
		pkg->homepage = ARGV(SQ_HOMEPAGE);
		pkg->licenses = ARGV(SQ_LICENSE);
		pkg->desc     = ARGV(SQ_DESCRIPTION);
		pkg->provide  = ARGV(SQ_PROVIDE);
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
		m_error_callback(eix::format("Can't open cache file %s") % sqlitefile);
		return false;
	}
	if(pkgtree)
		pkgtree->need_fast_access(categories);
	callback_arg = this;
	sqlite_callback_error = false;
	maxindex = 0;
	packagetree = pkgtree;
	category = cat;
	rc = sqlite3_exec(db, "select * from portage_packages", sqlite_callback, 0, &errormessage);
	sqlite3_close(db);
	if(pkgtree)
		pkgtree->finish_fast_access();
	if(rc != SQLITE_OK) {
		sqlite_callback_error = true;
		m_error_callback(eix::format("sqlite error: %s") % errormessage);
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
	"Recompile eix, using configure option --with-sqlite to add sqlite support");
	return false;
}

#endif
