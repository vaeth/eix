// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __SQLITECACHE_H__
#define __SQLITECACHE_H__

#include <cache/base.h>
#include <eixTk/stringutils.h>
#include <vector>

class SqliteCache : public BasicCache {
	friend int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName);

	private:
		std::vector<int> trueindex;
		int maxindex;

		/** This variable is actually the this-parameter for our sqlite_callback function.
		    Note that this makes readCategories non-reentrant. */
		static SqliteCache *callback_arg;
		/** Our sqlite_callback() will set this to true in case of an error. */
		bool sqlite_callback_error;
		/** Parameter passing to sqlite_callback() */
		PackageTree *packagetree;
		Category *category;

	public:
		bool can_read_multiple_categories() const
		{ return true; }

		bool readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category = NULL) throw(ExBasic);

		const char *getType() const
		{ return "sqlite"; }
};

#endif /* __SQLITECACHE_H__ */
