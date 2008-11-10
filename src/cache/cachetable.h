// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__CACHETABLE_H__)
#define EIX__CACHETABLE_H__

#include <cache/cache_map.h>
#include <eixTk/ptr_list.h>

#include <string>
#include <map>

class CacheTable : public eix::ptr_list<BasicCache>
{
	public:
		~CacheTable()
		{ delete_and_clear(); }

		void addCache(const char *eprefixcache, const char *eprefixport, const char *directory, const std::string &cache_name, const std::map<std::string, std::string> *override);
};

#endif /* EIX__CACHETABLE_H__ */
