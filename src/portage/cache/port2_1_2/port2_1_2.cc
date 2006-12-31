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

#include "port2_1_2.h"
#include <cache-utils/unpickle.h>

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <config.h>

using namespace std;

/* Path to portage cache */
#define PORTAGE_PICKLE "/var/cache/edb/vdb_metadata.pickle";

class MapFile {
	private:
		void *pf_data;
		size_t pf_data_size;

		bool mapData(int fd) {
			struct stat st;
			if (fstat(fd,&st) == 0) {
				void *x = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
				if (x != (void*)-1) {
					pf_data_size = st.st_size;
					pf_data = x;
					return true;
				}
			}
			return false;
		}

	public:
		MapFile(const char *file) {
			pf_data = NULL;
			pf_data_size = 0;
			int fd = open(file, O_RDONLY);
			if(fd == -1) {
				return;
			}
			if( ! mapData(fd))  {
				close(fd);
				return;
			}
			close(fd);
		}

		~MapFile() {
			if(pf_data != NULL) {
				munmap(pf_data, pf_data_size);
			}
		}

		bool isReady(const char **data, const char **end) {
			if(!pf_data)
				return false;
			*data = (const char *)pf_data;
			*end = ((const char *)pf_data) + pf_data_size * sizeof(char);
			return true;
		}
};


bool Port2_1_2_Cache::readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category) throw(ExBasic)
{
	string filename = PORTAGE_PICKLE;
	const char *data, *end;
	map<string,string> unpickled;

	MapFile picklefile(filename.c_str());
	if( ! picklefile.isReady(&data, &end) )
	{
		m_error_callback("Can't read cache file %s",
				filename.c_str());
		return true;
	}
	while( data < end ) {
		if(!unpickle_get(&data, end, unpickled, false)) {
			m_error_callback("Unpickle of %s failed",
				filename.c_str());
			return false;
		}
	}
	return true;
}
