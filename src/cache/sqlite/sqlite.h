// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_SQLITE_SQLITE_H_
#define SRC_CACHE_SQLITE_SQLITE_H_ 1

#include <vector>

#include "cache/base.h"
#include "eixTk/dialect.h"

class Category;
class PackageTree;
class TrueIndex;

class SqliteCache : public BasicCache {
		friend int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName);

	public:  // actually private, but this is too clumsy...
		typedef std::vector<int> TrueIndexMap;

	private:
		bool never_add_categories;
		ATTRIBUTE_NONNULL((4)) void sqlite_callback_cpp(int argc, const char *const *argv, const char *const *azColName);
		TrueIndexMap trueindex;
		int maxindex;

		/**
		This variable is actually the this-parameter for our sqlite_callback function.
		Note that this makes readCategories non-reentrant.
		**/
		static SqliteCache *callback_arg;
		/**
		Our sqlite_callback() will set this to true in case of an error
		**/
		bool sqlite_callback_error;
		/**
		Parameter passing to sqlite_callback()
		**/
		PackageTree *packagetree;
		Category *category;
		const char *cat_name;
		static TrueIndex *true_index;

	public:
		SqliteCache() : BasicCache(), never_add_categories(true) {
		}

		explicit SqliteCache(bool add_categories) : BasicCache(), never_add_categories(!add_categories) {
		}

		ATTRIBUTE_CONST_VIRTUAL bool can_read_multiple_categories() const OVERRIDE {
			return true;
		}

		bool readCategories(PackageTree *packagetree, const char *catname, Category *cat) OVERRIDE;

		const char *getType() const OVERRIDE {
			return (never_add_categories ? "sqlite" : "sqlite*");
		}
};

#endif  // SRC_CACHE_SQLITE_SQLITE_H_
