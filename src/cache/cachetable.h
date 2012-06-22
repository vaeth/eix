// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__CACHETABLE_H__
#define EIX__CACHETABLE_H__ 1

#include <cache/base.h>
#include <eixTk/ptr_list.h>

#include <map>
#include <string>

class CacheTable : public eix::ptr_list<BasicCache>
{
	private:
		std::string m_appending;
	public:
		CacheTable(const std::string &appending) : m_appending(appending)
		{ }

		~CacheTable()
		{ delete_and_clear(); }

		bool addCache(const char *eprefixcache, const char *eprefixport, const char *directory, const std::string &cache_name, const std::map<std::string, std::string> *override, std::string *errtext);
};

#endif /* EIX__CACHETABLE_H__ */
