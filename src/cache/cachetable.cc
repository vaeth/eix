// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "cachetable.h"
#include <cache/base.h>
#include <cache/cache_map.h>
#include <eixTk/exceptions.h>
#include <eixTk/filenames.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>

#include <map>
#include <string>

#include <cstddef>

using namespace std;

void
CacheTable::addCache(const char *eprefixcache, const char *eprefixport, const char *directory, const string &cache_name, const map<string, string> *override)
{
	for(CacheTable::iterator it(begin()); likely(it != end()); ++it) {
		if(same_filenames(directory, (it->getPath()).c_str()))
			return;
	}
	const char *cache_method(cache_name.c_str());
	if(unlikely(override != NULL)) {
		// If we would look for identical names, we could use the much faster
		// map<string, string>::const_iterator found = override->find(directory);
		for(map<string, string>::const_reverse_iterator it(override->rbegin());
			unlikely(it != override->rend()); ++it) {
			if(same_filenames((it->first).c_str(), directory, true)) {
				cache_method = (it->second).c_str();
				break;
			}
		}
	}
	BasicCache *cache(get_cache(cache_method, m_appending));
	if(unlikely(cache == NULL)) {
		throw ExBasic(_("Unknown cache %r for directory %r"))
			% cache_method % directory;
	}

	cache->setScheme(eprefixcache, eprefixport, directory);
	push_back(cache);
}
