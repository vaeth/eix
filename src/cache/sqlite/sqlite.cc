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



/** All stuff related to our sqlite_callback()
    Essentially, this is just a C-wrapper to get the implicit this argument in */

SqliteCache *SqliteCache::callback_arg;

inline static const char *
welldefine(const char *s)
{
	if(s)
		return s;
	return "";
}

int
sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	UNUSED(NotUsed);
#if 0
	for(int i = 0; i<argc; ++i) {
		cout << i << ": " << azColName[i] << " = " <<  welldefine(argv[i]) << "\n";
	}
#else
	SqliteCache::callback_arg->sqlite_callback_cpp(argc, const_cast<const char **>(argv), const_cast<const char **>(azColName));
#endif
	return 0;
}

/**
    The following is all related to get the proper index for the lookups.
    The main idea is the following: We let
    SqliteCache::trueindex[TrueIndex::Names] = actual index or negative
    where the negative value indicates that the data is not available
    because the sqlite dataset is too short; the minimal dataset length
    for all mandatory data is stored in SqliteCache::maxindex.

    The class TrueIndex and the static (and only) instance handle_trueindex
    is used to calculate the initial value of trueindex/maxindex
    at the first database access by first filling it with default parameters
    and - for the case that appropriate data is stored in azColName -
    modifying this correspondingly: This has the advantage that if some
    portage versions use different names, we still have (hopefully correct)
    default values.

    The class TrueIndex itself is independent of SqliteCache and gets passed
    all required data by parameters. Note that it stores nothing except for
    the static default data, and so, except for the first initialization in
    the constructor, all functions should be const or static.
*/

static class TrueIndex : public map<string,int> {
	public:
		typedef enum {
			NAME,
			SLOT,
			RESTRICT,
			HOMEPAGE,
			LICENSE,
			DESCRIPTION,
			KEYWORDS,
			IUSE,
			PROVIDE,
			PROPERTIES,
			LAST
		} Names;
		vector<int> default_trueindex;

	private:
		void mapinit(int true_index, int my_index, const char *s)
		{
			(*this)[s] = my_index;
			default_trueindex[my_index] = true_index;
		}

	public:
		TrueIndex() : default_trueindex(LAST, -1)
		{
			mapinit( 1, NAME,        "portage_package_key");
			mapinit( 6, SLOT,        "SLOT");
			mapinit( 8, RESTRICT,    "RESTRICT");
			mapinit( 9, HOMEPAGE,    "HOMEPAGE");
			mapinit(10, LICENSE,     "LICENSE");
			mapinit(11, DESCRIPTION, "DESCRIPTION");
			mapinit(12, KEYWORDS,    "KEYWORDS");
			mapinit(14, IUSE,        "IUSE");
			mapinit(17, PROVIDE,     "PROVIDE");
			mapinit(19, PROPERTIES,  "PROPERTIES");
		}

		int calc(int argc, const char **azColName, vector<int> &trueindex) const
		{
			trueindex = default_trueindex;
			for(int i = 0; i < argc; ++i) {
				map<string,int>::const_iterator it = find(azColName[i]);
				if(it != end())
					trueindex[it->second] = i;
			}
			int maxindex = -1;
			for(int i = 0; i < TrueIndex::LAST; ++i) {
				int curr = trueindex[i];
				// Shortcut if we have not reached the maximum
				if(maxindex >= curr)
					continue;
				// Is the true index out of range?
				if(argc <= curr) {
					trueindex[i] = -1;
					// PROPERTIES is not mandatory
					if(i == TrueIndex::PROPERTIES)
						continue;
				}
				maxindex = curr;
			}
			return maxindex;
		}

		static const char *c_str(const char **argv, vector<int> &trueindex, const int i)
		{
			int t = trueindex[i];
			if(t < 0)
				return "";
			return welldefine(argv[t]);
		}
} handle_trueindex;

void
SqliteCache::sqlite_callback_cpp(int argc, const char **argv, const char **azColName)
{
	// If an earlier error occurred, we ignore later calls:
	if(sqlite_callback_error)
		return;

	if(!maxindex)
		maxindex = handle_trueindex.calc(argc, azColName, trueindex);

	if(argc <= trueindex[TrueIndex::NAME]) {
		sqlite_callback_error = true;
		m_error_callback("Dataset does not contain a package name");
		return;
	}
	string catarg = TrueIndex::c_str(argv, trueindex, TrueIndex::NAME);
	if(argc <= maxindex) {
		sqlite_callback_error = true;
		m_error_callback(eix::format("Dataset for %s is too small") % catarg);
		return;
	}
	string::size_type pos = catarg.find_first_of('/');
	if(pos == string::npos) {
		sqlite_callback_error = true;
		m_error_callback(eix::format("%r not of the form package/category-version") % catarg);
		return;
	}
	string name_ver = catarg.substr(pos + 1);
	catarg.resize(pos);
	// Does the catarg match category?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(category) {
		dest_cat = category;
		if(dest_cat->name != catarg)
			return;
	}
	else {
		dest_cat = packagetree->find(catarg);
		if(!dest_cat)
			return;
	}
	char **aux = ExplodeAtom::split(name_ver.c_str());
	if(aux == NULL) {
		m_error_callback(eix::format("Can't split %r into package and version") % name_ver);
		return;
	}
	/* Search for existing package */
	Package *pkg = dest_cat->findPackage(aux[0]);

	/* If none was found create one */
	if(pkg == NULL)
		pkg = dest_cat->addPackage(aux[0]);

	/* Create a new version and add it to package */
	Version *version = new Version(aux[1]);
	// reading slots and stability
	version->slotname = TrueIndex::c_str(argv, trueindex, TrueIndex::SLOT);
	version->set_restrict(TrueIndex::c_str(argv, trueindex, TrueIndex::RESTRICT));
	version->set_properties(TrueIndex::c_str(argv, trueindex, TrueIndex::PROPERTIES));
	string keywords = TrueIndex::c_str(argv, trueindex, TrueIndex::KEYWORDS);
	version->set_full_keywords(keywords);
	string iuse = TrueIndex::c_str(argv, trueindex, TrueIndex::IUSE);
	version->set_iuse(iuse);
	pkg->addVersion(version);

	/* For the newest version, add all remaining data */
	if(*(pkg->latest()) == *version)
	{
		pkg->homepage = TrueIndex::c_str(argv, trueindex, TrueIndex::HOMEPAGE);
		pkg->licenses = TrueIndex::c_str(argv, trueindex, TrueIndex::LICENSE);
		pkg->desc     = TrueIndex::c_str(argv, trueindex, TrueIndex::DESCRIPTION);
		pkg->provide  = TrueIndex::c_str(argv, trueindex, TrueIndex::PROVIDE);
	}
	/* Free old split */
	free(aux[0]);
	free(aux[1]);
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
	trueindex.clear();
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
