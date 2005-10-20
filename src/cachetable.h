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

#ifndef __CACHEDIRECTORY_H__
#define __CACHEDIRECTORY_H__

#include <portage/cache/cache-map.h>
#include <string>
#include <map>

class CacheTable {

	public:
		char *arch;

	private:
		vector<BasicCache*> cache_table;

	public:
		int addCache(string directory, string cache_name) {
			BasicCache *cache = get_cache(cache_name);
			if(cache) {
				cache->setScheme(directory);
				cache_table.push_back(cache);
			}
			else {
				throw(ExBasic("Unknown cache '%s' for scheme '%s'!", cache_name.c_str(), directory.c_str()));
			}
			return 1;
		}

		vector<BasicCache*>::iterator end() {
			return cache_table.end();
		}

		vector<BasicCache*>::iterator begin() {
			return cache_table.begin();
		}
};

#endif /* __CACHEDIRECTORY_H__ */
