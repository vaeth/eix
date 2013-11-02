// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_CACHE_CACHETABLE_H_
#define SRC_CACHE_CACHETABLE_H_ 1

#include <string>

#include "cache/base.h"
#include "eixTk/ptr_list.h"

typedef eix::ptr_list<BasicCache> CacheTableList;

class CacheTable : public CacheTableList {
	private:
		std::string m_appending;

	public:
		explicit CacheTable(const std::string& appending) : m_appending(appending) {
		}

		~CacheTable() {
			delete_and_clear();
		}

		bool addCache(const char *eprefixcache, const char *eprefixport, const char *directory, const std::string& cache_name, const WordMap *override, std::string *errtext) ATTRIBUTE_NONNULL((4));
};

#endif  // SRC_CACHE_CACHETABLE_H_
