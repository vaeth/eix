/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __CACHETABLE_H__
#define __CACHETABLE_H__

#include <portage/cache/cache-map.h>
#include <eixTk/ptr_list.h>
#include <eixTk/filenames.h>

#include <string>
#include <map>

class CacheTable
	: public eix::ptr_list<BasicCache>
{
	public:
		~CacheTable()
		{ delete_and_clear(); }

		void addCache(const char *prefix, const char *directory, std::string cache_name, const std::map<std::string, std::string> *override)
		{
			for(CacheTable::iterator it=begin(); it != end(); ++it)
				if(same_filenames(directory, (it->getPath()).c_str()))
					return;
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
					cache_name = found->second;
			}
			BasicCache *cache = get_cache(cache_name);
			if(cache == NULL)
			{
				throw(ExBasic("Unknown cache '%s' for directory '%s'!", cache_name.c_str(), directory));
			}

			cache->setScheme(prefix, directory);
			push_back(cache);
		}
};

#endif /* __CACHETABLE_H__ */
