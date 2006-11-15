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

#include "cdb.h"

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <map>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#if !defined(__OpenBSD__)
#include <stdint.h>
#endif

#include <unistd.h>

#include <config.h>

using namespace std;

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"

#if defined(WORDS_BIGENDIAN)
/* For the big-endian machines */
#define UINT32_PACK(out,in)   uint32_pack(out,in)
#define UINT32_UNPACK(in,out) uint32_unpack(in,out)

/* Adopted from python-cdb (which adopted it from libowfat :) */
inline static void uint32_pack(char *out, uint32_t in);
inline static void uint32_unpack(const char *in, uint32_t *out);

inline static void uint32_pack(char *out, uint32_t in) {
	*out=in&0xff; in>>=8;
	*++out=in&0xff; in>>=8;
	*++out=in&0xff; in>>=8;
	*++out=in&0xff;
}

inline static void uint32_unpack(const char *in, uint32_t *out) {
	*out = (((uint32_t)(unsigned char)in[3])<<24) |
		(((uint32_t)(unsigned char)in[2])<<16) |
		(((uint32_t)(unsigned char)in[1])<<8) |
		(uint32_t)(unsigned char)in[0];
}

#else  /* defined(WORDS_BIGENDIAN) */
/* This is for the little-endian pppl */

/* Adopted from python-cdb (which adopted it from libowfat) */
#define UINT32_PACK(out,in) (*(uint32_t*)(out)=(*(uint32_t*)&in))
#define UINT32_UNPACK(in,out) (*((uint32_t*)out)=*(uint32_t*)(in))

#endif /* defined(WORDS_BIGENDIAN) */

#define STARTING_HASH 5381

class Cdb {
	private:
		bool is_ready;

		uint32_t *cdb_data;
		size_t cdb_data_size;
		uint32_t *cdb_records_end;
		uint32_t *current;

		bool mapData(int fd) {
			struct stat st;
			if (fstat(fd,&st) == 0) {
				void *x = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
				if (x != (void*)-1) {
					cdb_data_size = st.st_size;
					cdb_data = (uint32_t *)x;
					return true;
				}
			}
			return false;
		}

		void init() {
			uint32_t record_end_offset;
			UINT32_UNPACK((char *)cdb_data, &record_end_offset);
			cdb_records_end = (uint32_t *)((char *)cdb_data + record_end_offset);
			current = cdb_data + (2 * 256);
			is_ready = true;
		}

	public:
		Cdb(const char *file) {
			cdb_data = NULL;
			cdb_data_size = 0;
			is_ready = false;
			int fd = open(file, O_RDONLY);
			if(fd == -1) {
				return;
			}
			if( ! mapData(fd))  {
				close(fd);
				return;
			}
			close(fd);

			init();
		}

		~Cdb() {
			if(cdb_data != NULL) {
				munmap(cdb_data, cdb_data_size);
			}
		}

		string get(uint32_t *dlen, void **data) {
			uint32_t klen;
			UINT32_UNPACK((char *)current, &klen);
			current++;
			UINT32_UNPACK((char *)current, dlen);
			current++;
			string key((char *)current, klen);
			current = (uint32_t*)(  (char *)current + (klen));
			*data = current;
			current = (uint32_t*)(  (char *)current + (*dlen));
			return key;
		}

		bool end() {
			return current >= cdb_records_end;
		}

		bool isReady() {
			return is_ready;
		}
};

#define BINSTRING       'T'
#define SHORT_BINSTRING 'U'
#define BINPUT          'q'
#define BININT          'J'
#define SETITEMS        'u'
#define STOP            '.'
#define MARK            '('
#define EMPTY_DICT      '}'

#define PROTO	 ((char)0x80) /* identify pickle protocol */

#define NEXT        (*data++)
#define MOVE(x)     (data += x)
#define IS_END      (data >= data_end)

void unpickle_check_dict(bool &waiting_for_value, map<string,string> &unpickled, string &buf, string &key) {
	if(waiting_for_value) {
		unpickled[key] = buf;
		waiting_for_value = false;
	}
	else {
		key = buf;
		waiting_for_value = true;
	}
}

bool unpickle_get_mapping(char *data, unsigned int data_len, map<string,string> &unpickled) {
	int ret = 1;
	char *data_end = data + data_len;
	bool waiting_for_value = false;
	string key;

	if(NEXT != PROTO)
		return false;

	if(NEXT != 0x2)
		return false;

	if(NEXT != EMPTY_DICT)
		return false;

	while( ! IS_END && (ret = NEXT) ) {
		switch(ret) {
			case BINPUT:
				MOVE(1);
				continue;
			case BININT:
				{
					string buf;
					MOVE(sizeof(int));
					unpickle_check_dict(waiting_for_value, unpickled, buf, key);
				}
				continue;
			case SHORT_BINSTRING:
				{
					unsigned char len;
					len = *(data)++;
					string buf(data, len);
					MOVE(len);
					unpickle_check_dict(waiting_for_value, unpickled, buf, key);
				}
				continue;
			case BINSTRING:
				{
					uint32_t len;
					UINT32_UNPACK(data, &len);
					MOVE(4);
					string buf(data, len);
					MOVE(len);
					unpickle_check_dict(waiting_for_value, unpickled, buf, key);
				}
				continue;
			case SETITEMS:
				ret = 0;
				continue;
		}
	}
	return true;
}

bool CdbCache::readCategory(Category &vec) throw(ExBasic)
{
	string cdbfile = PORTAGE_CACHE_PATH + m_scheme + vec.name() + ".cdb";
	uint32_t dlen;
	void *data;
	string key;

	Cdb cdb(cdbfile.c_str());
	if( ! cdb.isReady() )
	{
		m_error_callback("Can't read cache file %s",
				cdbfile.c_str());
		return true;
	}
	while( ! cdb.end() ) {
		key = cdb.get(&dlen, &data);
		map<string,string> mapping;
		if( ! unpickle_get_mapping((char *)data, dlen, mapping)) {
			m_error_callback("Problems with %s .. skipping.", key.c_str());
			continue;
		}

		/* Split string into package and version, and catch any errors. */
		char **aux = ExplodeAtom::split(key.c_str());
		if(aux == NULL) {
			m_error_callback("Can't split '%s' into package and version.", key.c_str());
			continue;
		}
		/* Search for existing package */
		Package *pkg = vec.findPackage(aux[0]);
		/* If none was found create one */
		if(pkg == NULL) {
			pkg = vec.addPackage(aux[0]);
		}

		/* Make version and add it to package. */
		Version *version = new Version(aux[1]);
		pkg->addVersionStart(version);
		/* For the latest version read/change corresponding data */
		if(*(pkg->latest()) == *version)
		{
			pkg->desc     = mapping["DESCRIPTION"];
			pkg->homepage = mapping["HOMEPAGE"];
			pkg->licenses = mapping["LICENSE"];
			pkg->provide  = mapping["PROVIDE"];
		}

		/* Read stability */
		version->set(m_arch, mapping["KEYWORDS"]);
		version->slot = mapping["SLOT"];
		version->overlay_key = m_overlay_key;
		pkg->addVersionFinalize(version);

		/* Free split */
		free(aux[0]);
		free(aux[1]);
	}

	return true;
}
