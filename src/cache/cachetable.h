// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_CACHETABLE_H_
#define SRC_CACHE_CACHETABLE_H_ 1

#include <config.h>

#include <string>
#include <vector>

#include "cache/base.h"
#include "eixTk/attribute.h"
#include "eixTk/ptr_container.h"
#include "eixTk/stringtypes.h"

typedef eix::ptr_container<std::vector<BasicCache *> > CacheTableList;

class CacheTable : public CacheTableList {
	private:
		std::string m_appending;

	public:
		explicit CacheTable(const std::string& appending) : m_appending(appending) {
		}

		~CacheTable() {
			delete_and_clear();
		}

		ATTRIBUTE_NONNULL((4)) bool addCache(const char *eprefixcache, const char *eprefixport, const char *directory, const std::string& cache_name, const WordMap *override, std::string *errtext);
};

#endif  // SRC_CACHE_CACHETABLE_H_
