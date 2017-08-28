// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

// #define SQLITE_ONLY_DEBUG

#include "cache/sqlite/sqlite.h"
#include <config.h>

#ifdef WITH_SQLITE
#include <sqlite3.h>

#include <string>

#include "eixTk/attribute.h"
#include "eixTk/formated.h"
#endif

#include "eixTk/i18n.h"

#ifdef WITH_SQLITE
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixTk/unordered_map.h"
#include "portage/basicversion.h"
#include "portage/depend.h"
#include "portage/package.h"
#include "portage/packagetree.h"
#include "portage/version.h"

using std::string;

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"



/**
All stuff related to our sqlite_callback()
Essentially, this is just a C-wrapper to get the implicit this argument in
*/
SqliteCache *SqliteCache::callback_arg;

ATTRIBUTE_CONST inline static const char *welldefine(const char *s);

inline static const char *welldefine(const char *s) {
	return ((s != NULLPTR) ? s : "");
}

int sqlite_callback(void * /* NotUsed */, int argc, char **argv, char **azColName) {
#ifdef SQLITE_ONLY_DEBUG
	for(int i(0); likely(i < argc); ++i) {
		eix::say("%s: %s = %s")
			% i % azColName[i] % welldefine(argv[i]);
	}
#else
	SqliteCache::callback_arg->sqlite_callback_cpp(argc, const_cast<const char *const *>(argv), const_cast<const char *const *>(azColName));
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

The class TrueIndex and the static (and only) instance *true_index
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
**/

typedef SqliteCache::TrueIndexMap::size_type TrueIndexRes;
typedef UNORDERED_MAP<string, TrueIndexRes> TrueIndexMapper;

class TrueIndex : public TrueIndexMapper {
	public:
		typedef enum {
			NAME,
			SLOT,
			RESTRICT,
			HOMEPAGE,
			LICENSE,
			DESCRIPTION,
			EAPI,
			KEYWORDS,
			IUSE,
			REQUIRED_USE,
			PROPERTIES,
			DEPEND,
			RDEPEND,
			PDEPEND,
			HDEPEND,
			LAST
		} Names;
		SqliteCache::TrueIndexMap default_trueindex;

	private:
		ATTRIBUTE_NONNULL_ void mapinit(int true_index, TrueIndexRes my_index, const char *s) {
			(*this)[s] = my_index;
			default_trueindex[my_index] = true_index;
		}

	public:
		TrueIndex() : default_trueindex(LAST, -1) {
			//  0 "internal_db_package_id"
			mapinit( 1, NAME,         "portage_package_key");
			//  2 "DEFINED_PHASES"
			mapinit( 3, DEPEND,       "DEPEND");
			mapinit( 4, DESCRIPTION,  "DESCRIPTION");
			mapinit( 5, EAPI,         "EAPI");
			mapinit( 6, HDEPEND,      "HDEPEND");
			mapinit( 7, HOMEPAGE,     "HOMEPAGE");
			//  8 "INHERITED"
			mapinit( 9, IUSE,         "IUSE");
			mapinit(10, KEYWORDS,     "KEYWORDS");
			mapinit(11, LICENSE,      "LICENSE");
			mapinit(12, PDEPEND,      "PDEPEND");
			mapinit(13, PROPERTIES,   "PROPERTIES");
			// 14 "PROVIDE"
			mapinit(15, RDEPEND,      "RDEPEND");
			mapinit(16, REQUIRED_USE, "REQUIRED_USE");
			mapinit(17, RESTRICT,     "RESTRICT");
			mapinit(18, SLOT,         "SLOT");
			// 19 "SRC_URI"
			// 20 "_eclasses_"
			// 21 "_mtime_"
		}

		ATTRIBUTE_NONNULL_ int calc(int argc, const char *const *azColName, SqliteCache::TrueIndexMap *trueindex) const {
			*trueindex = default_trueindex;
			for(int i(0); i < argc; ++i) {
				TrueIndexMapper::const_iterator it(find(azColName[i]));
				if(it != end()) {
					(*trueindex)[it->second] = i;
				}
			}
			int max_index(-1);
			for(TrueIndexRes i(0); likely(i < TrueIndex::LAST); ++i) {
				int curr((*trueindex)[i]);
				// Shortcut if we have not reached the maximum
				if(max_index >= curr) {
					continue;
				}
				// Is the true index out of range?
				if(argc <= curr) {
					(*trueindex)[i] = -1;
					// PROPERTIES is not mandatory
					if(i == TrueIndex::PROPERTIES) {
						continue;
					}
				}
				max_index = curr;
			}
			return max_index;
		}

		ATTRIBUTE_NONNULL((2)) static const char *c_str(const char *const *argv, SqliteCache::TrueIndexMap *trueindex, const TrueIndexRes i) {
			int t((*trueindex)[i]);
			if(t < 0) {
				return "";
			}
			return welldefine(argv[t]);
		}
};

TrueIndex *SqliteCache::true_index = NULLPTR;

void SqliteCache::sqlite_callback_cpp(int argc, const char *const *argv, const char *const *azColName) {
	// If an earlier error occurred, we ignore later calls:
	if(unlikely(sqlite_callback_error)) {
		return;
	}

	if(maxindex == 0) {
		if(unlikely(true_index == NULLPTR)) {
			true_index = new TrueIndex;
		}
		maxindex = true_index->calc(argc, azColName, &trueindex);
	}

	if(argc <= trueindex[TrueIndex::NAME]) {
		sqlite_callback_error = true;
		m_error_callback(_("sqlite dataset does not contain a package name"));
		return;
	}
	string catarg(TrueIndex::c_str(argv, &trueindex, TrueIndex::NAME));
	if(argc <= maxindex) {
		sqlite_callback_error = true;
		m_error_callback(eix::format(_("sqlite dataset for %s is too small")) % catarg);
		return;
	}
	string::size_type pos(catarg.find('/'));
	if(pos == string::npos) {
		sqlite_callback_error = true;
		m_error_callback(eix::format(_("\"%s\" not of the form package/category-version")) % catarg);
		return;
	}
	string name_ver(catarg, pos + 1);
	catarg.resize(pos);
	// Does the catarg match category?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(unlikely(packagetree == NULLPTR)) {
		if(cat_name != catarg) {
			return;
		}
		dest_cat = category;
	} else if(never_add_categories) {
		dest_cat = packagetree->find(catarg);
		if(unlikely(dest_cat == NULLPTR)) {
			return;
		}
	} else {
		dest_cat = &((*packagetree)[catarg]);
	}
	string curr_name, curr_version;
	if(unlikely(!ExplodeAtom::split(&curr_name, &curr_version, name_ver.c_str()))) {
		m_error_callback(eix::format(_("cannot split \"%s\" into package and version")) % name_ver);
		return;
	}
	/* Search for existing package */
	Package *pkg(dest_cat->findPackage(curr_name));

	/* If none was found create one */
	if(pkg == NULLPTR) {
		pkg = dest_cat->addPackage(catarg, curr_name);
	}

	/* Create a new version and add it to package */
	Version *version(new Version);
	string errtext;
	BasicVersion::ParseResult r(version->parseVersion(curr_version, &errtext));
	if(unlikely(r != BasicVersion::parsedOK)) {
		m_error_callback(errtext);
	}
	if(unlikely(r == BasicVersion::parsedError)) {
		delete version;
	} else {
		// reading slots and stability
		version->set_slotname(TrueIndex::c_str(argv, &trueindex, TrueIndex::SLOT));
		version->set_restrict(TrueIndex::c_str(argv, &trueindex, TrueIndex::RESTRICT));
		version->set_properties(TrueIndex::c_str(argv, &trueindex, TrueIndex::PROPERTIES));
		version->set_full_keywords(TrueIndex::c_str(argv, &trueindex, TrueIndex::KEYWORDS));
		version->set_iuse(TrueIndex::c_str(argv, &trueindex, TrueIndex::IUSE));
		version->set_required_use(TrueIndex::c_str(argv, &trueindex, TrueIndex::REQUIRED_USE));
		version->eapi.assign(TrueIndex::c_str(argv, &trueindex, TrueIndex::EAPI));
		version->depend.set(TrueIndex::c_str(argv, &trueindex, TrueIndex::DEPEND),
			TrueIndex::c_str(argv, &trueindex, TrueIndex::RDEPEND),
			TrueIndex::c_str(argv, &trueindex, TrueIndex::PDEPEND),
			TrueIndex::c_str(argv, &trueindex, TrueIndex::HDEPEND),
			false);
		version->overlay_key = m_overlay_key;
		pkg->addVersion(version);

		/* For the newest version, add all remaining data */
		if(*(pkg->latest()) == *version) {
			pkg->homepage = TrueIndex::c_str(argv, &trueindex, TrueIndex::HOMEPAGE);
			pkg->licenses = TrueIndex::c_str(argv, &trueindex, TrueIndex::LICENSE);
			pkg->desc     = TrueIndex::c_str(argv, &trueindex, TrueIndex::DESCRIPTION);
		}
	}
}

bool SqliteCache::readCategories(PackageTree *pkgtree, const char *catname, Category *cat) {
	char *errormessage(NULLPTR);
	string sqlitefile(m_prefix + PORTAGE_CACHE_PATH + m_scheme);
	// Cut all trailing '/' and append ".sqlite" to the name
	string::size_type pos(sqlitefile.find_last_not_of('/'));
	if(unlikely(pos == string::npos)) {
		m_error_callback(_("database path incorrect"));
		return false;
	}
	sqlitefile.resize(pos + 1);
	sqlitefile.append(".sqlite");

	sqlite3 *db(NULLPTR);
	if(sqlite3_open(sqlitefile.c_str(), &db) != SQLITE_OK) {
		sqlite3_close(db);
		m_error_callback(eix::format(_("cannot open cache file %s")) % sqlitefile);
		return false;
	}
	callback_arg = this;
	sqlite_callback_error = false;
	maxindex = 0;
	packagetree = pkgtree;
	category = cat;
	cat_name = catname;
	int rc(sqlite3_exec(db, "select * from portage_packages", sqlite_callback, NULLPTR, &errormessage));
	sqlite3_close(db);
	trueindex.clear();
	if(rc != SQLITE_OK) {
		sqlite_callback_error = true;
		m_error_callback(eix::format(_("sqlite error: %s")) % errormessage);
	}
	return !sqlite_callback_error;
}

#else  // Not WITH_SQLITE

bool SqliteCache::readCategories(PackageTree * /* pkgtree */, const char * /* catname */, Category * /* cat */) {
	m_error_callback(_("cache method sqlite is not compiled in.\n"
		"Recompile eix, using configure option --with-sqlite to add sqlite support"));
	return false;
}

#endif
