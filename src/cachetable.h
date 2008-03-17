// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __CACHETABLE_H__
#define __CACHETABLE_H__

#include <cache/cache-map.h>
#include <eixTk/ptr_list.h>
#include <eixTk/filenames.h>

#include <string>
#include <map>
#include <cstdlib>

class CacheTable
	: public eix::ptr_list<BasicCache>
{
	public:
		~CacheTable()
		{ delete_and_clear(); }

		void addCache(const char *eprefixcache, const char *eprefixport, const char *eprefixexec, const char *directory, const std::string &cache_name, const std::map<std::string, std::string> *override)
		{
			for(CacheTable::iterator it=begin(); it != end(); ++it)
				if(same_filenames(directory, (it->getPath()).c_str()))
					return;
			const char *cache_method = cache_name.c_str();
			if(override)
			{
				// If we would look for identical names, we could use the much faster
				// std::map<std::string, std::string>::const_iterator found = override->find(directory);
				std::map<std::string, std::string>::const_reverse_iterator found;
				for(found = override->rbegin();
					found != override->rend(); ++found)
					if(same_filenames((found->first).c_str(), directory, true))
						break;
				if(found != override->rend())
					cache_method = (found->second).c_str();
			}
			BasicCache *cache = get_cache(cache_method);
			if(cache == NULL)
			{
				throw ExBasic("Unknown cache %r for directory %r")
					% cache_method % directory;
			}

			cache->setScheme(eprefixcache, eprefixport, eprefixexec, directory);
			push_back(cache);
		}
};

#endif /* __CACHETABLE_H__ */
