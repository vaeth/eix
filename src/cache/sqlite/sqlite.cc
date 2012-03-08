// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "sqlite.h"

#ifdef WITH_SQLITE

#include <eixTk/exceptions.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>
#include <eixTk/unused.h>
#include <portage/depend.h>
#include <portage/package.h>
#include <portage/packagetree.h>
#include <portage/version.h>

#ifdef SQLITE_ONLY_DEBUG
#include <iostream>
#endif
#include <map>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdlib>

#include <sqlite3.h>

using namespace std;

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"



/** All stuff related to our sqlite_callback()
    Essentially, this is just a C-wrapper to get the implicit this argument in */

SqliteCache *SqliteCache::callback_arg;

inline static const char *
welldefine(const char *s) ATTRIBUTE_CONST;
inline static const char *
welldefine(const char *s)
{
	if(s != NULL)
		return s;
	return "";
}

int
sqlite_callback(void *NotUsed ATTRIBUTE_UNUSED, int argc, char **argv, char **azColName)
{
	UNUSED(NotUsed);
#ifdef SQLITE_ONLY_DEBUG
	for(int i(0); likely(i < argc); ++i) {
		cout << eix::format("%s: %s = %s\n")
			% i % azColName[i] % welldefine(argv[i]);
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

static class TrueIndex : public map<string,vector<int>::size_type> {
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
			PROPERTIES,
			DEPEND,
			RDEPEND,
			PDEPEND,
			LAST
		} Names;
		vector<int> default_trueindex;

	private:
		void mapinit(int true_index, vector<int>::size_type my_index, const char *s)
		{
			(*this)[s] = my_index;
			default_trueindex[my_index] = true_index;
		}

	public:
		TrueIndex() : default_trueindex(LAST, -1)
		{
			mapinit( 1, NAME,        "portage_package_key");
			mapinit( 3, DEPEND,      "DEPEND");
			mapinit( 4, DESCRIPTION, "DESCRIPTION");
			mapinit( 6, HOMEPAGE,    "HOMEPAGE");
			mapinit( 8, IUSE,        "IUSE");
			mapinit( 9, KEYWORDS,    "KEYWORDS");
			mapinit(10, LICENSE,     "LICENSE");
			mapinit(11, PDEPEND,     "PDEPEND");
			mapinit(12, PROPERTIES,  "PROPERTIES");
			mapinit(14, RDEPEND,     "RDEPEND");
			mapinit(16, RESTRICT,    "RESTRICT");
			mapinit(17, SLOT,        "SLOT");
		}

		int calc(int argc, const char **azColName, vector<int> &trueindex) const
		{
			trueindex = default_trueindex;
			for(int i(0); i < argc; ++i) {
				map<string,vector<int>::size_type>::const_iterator it(find(azColName[i]));
				if(it != end())
					trueindex[it->second] = i;
			}
			int maxindex(-1);
			for(vector<int>::size_type i(0); likely(i < TrueIndex::LAST); ++i) {
				int curr(trueindex[i]);
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

		static const char *c_str(const char **argv, vector<int> &trueindex, const vector<int>::size_type i)
		{
			int t(trueindex[i]);
			if(t < 0)
				return "";
			return welldefine(argv[t]);
		}
} handle_trueindex;

void
SqliteCache::sqlite_callback_cpp(int argc, const char **argv, const char **azColName)
{
	// If an earlier error occurred, we ignore later calls:
	if(unlikely(sqlite_callback_error))
		return;

	if(!maxindex)
		maxindex = handle_trueindex.calc(argc, azColName, trueindex);

	if(argc <= trueindex[TrueIndex::NAME]) {
		sqlite_callback_error = true;
		m_error_callback(_("sqlite dataset does not contain a package name"));
		return;
	}
	string catarg(TrueIndex::c_str(argv, trueindex, TrueIndex::NAME));
	if(argc <= maxindex) {
		sqlite_callback_error = true;
		m_error_callback(eix::format(_("sqlite dataset for %s is too small")) % catarg);
		return;
	}
	string::size_type pos(catarg.find('/'));
	if(pos == string::npos) {
		sqlite_callback_error = true;
		m_error_callback(eix::format(_("%r not of the form package/category-version")) % catarg);
		return;
	}
	string name_ver(catarg, pos + 1);
	catarg.resize(pos);
	// Does the catarg match category?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(unlikely(packagetree == NULL)) {
		if(cat_name != catarg)
			return;
		dest_cat = category;
	}
	else if(never_add_categories) {
		dest_cat = packagetree->find(catarg);
		if(unlikely(dest_cat == NULL))
			return;
	}
	else {
		dest_cat =  &((*packagetree)[catarg]);
	}
	char **aux(ExplodeAtom::split(name_ver.c_str()));
	if(unlikely(aux == NULL)) {
		m_error_callback(eix::format(_("Can't split %r into package and version")) % name_ver);
		return;
	}
	/* Search for existing package */
	Package *pkg(dest_cat->findPackage(aux[0]));

	/* If none was found create one */
	if(pkg == NULL)
		pkg = dest_cat->addPackage(catarg, aux[0]);

	/* Create a new version and add it to package */
	Version *version(new Version(aux[1]));
	// reading slots and stability
	version->slotname = TrueIndex::c_str(argv, trueindex, TrueIndex::SLOT);
	version->set_restrict(TrueIndex::c_str(argv, trueindex, TrueIndex::RESTRICT));
	version->set_properties(TrueIndex::c_str(argv, trueindex, TrueIndex::PROPERTIES));
	version->set_full_keywords(TrueIndex::c_str(argv, trueindex, TrueIndex::KEYWORDS));
	version->set_iuse(TrueIndex::c_str(argv, trueindex, TrueIndex::IUSE));
	version->depend.set(TrueIndex::c_str(argv, trueindex, TrueIndex::DEPEND),
		TrueIndex::c_str(argv, trueindex, TrueIndex::RDEPEND),
		TrueIndex::c_str(argv, trueindex, TrueIndex::PDEPEND),
		false);
	pkg->addVersion(version);

	/* For the newest version, add all remaining data */
	if(*(pkg->latest()) == *version)
	{
		pkg->homepage = TrueIndex::c_str(argv, trueindex, TrueIndex::HOMEPAGE);
		pkg->licenses = TrueIndex::c_str(argv, trueindex, TrueIndex::LICENSE);
		pkg->desc     = TrueIndex::c_str(argv, trueindex, TrueIndex::DESCRIPTION);
	}
	/* Free old split */
	free(aux[0]);
	free(aux[1]);
}

bool
SqliteCache::readCategories(PackageTree *pkgtree, const char *catname, Category *cat) throw(ExBasic)
{
	char *errormessage(NULL);
	string sqlitefile(m_prefix + PORTAGE_CACHE_PATH + m_scheme);
	// Cut all trailing '/' and append ".sqlite" to the name
	string::size_type pos(sqlitefile.find_last_not_of('/'));
	if(unlikely(pos == string::npos)) {
		m_error_callback(_("Database path incorrect"));
		return false;
	}
	sqlitefile.resize(pos + 1);
	sqlitefile.append(".sqlite");

	sqlite3 *db;
	int rc(sqlite3_open(sqlitefile.c_str(), &db));
	if(rc)
	{
		sqlite3_close(db);
		m_error_callback(eix::format(_("Can't open cache file %s")) % sqlitefile);
		return false;
	}
	callback_arg = this;
	sqlite_callback_error = false;
	maxindex = 0;
	packagetree = pkgtree;
	category = cat;
	cat_name = catname;
	rc = sqlite3_exec(db, "select * from portage_packages", sqlite_callback, 0, &errormessage);
	sqlite3_close(db);
	trueindex.clear();
	if(rc != SQLITE_OK) {
		sqlite_callback_error = true;
		m_error_callback(eix::format(_("sqlite error: %s")) % errormessage);
	}
	return !sqlite_callback_error;
}

#else /* Not WITH_SQLITE */

#include <eixTk/i18n.h>
#include <eixTk/unused.h>

using namespace std;

bool
SqliteCache::readCategories(PackageTree *pkgtree ATTRIBUTE_UNUSED, const char *catname ATTRIBUTE_UNUSED, Category *cat ATTRIBUTE_UNUSED) throw(ExBasic)
{
	UNUSED(pkgtree);
	UNUSED(catname);
	UNUSED(cat);
	m_error_callback(_("Cache method sqlite is not compiled in.\n"
		"Recompile eix, using configure option --with-sqlite to add sqlite support"));
	return false;
}

#endif
