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

#include <config.h>  // IWYU pragma: keep

#include <string>
#include <utility>
#include <vector>

#include "cache/base.h"
#include "eixTk/attribute.h"
#include "eixTk/ptr_container.h"

typedef eix::ptr_container<std::vector<BasicCache *> > CacheTableList;
typedef std::pair<std::string, std::string> OverridePair;
typedef std::vector<OverridePair> OverrideVector;

class CacheTable : public CacheTableList {
	private:
		std::string m_appending;

	public:
		explicit CacheTable(const std::string& appending) : m_appending(appending) {
		}

		~CacheTable() {
			delete_and_clear();
		}

		ATTRIBUTE_NONNULL((4)) bool addCache(const char *eprefixcache, const char *eprefixport, const char *directory, const std::string& cache_name, const OverrideVector *override_vector, std::string *errtext);
};

#endif  // SRC_CACHE_CACHETABLE_H_
