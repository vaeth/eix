// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "cdb.h"
#include <cache/common/unpickle.h>

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <eixTk/stringutils.h>

using namespace std;

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"

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
				if (x != MAP_FAILED) {
					cdb_data_size = st.st_size;
					cdb_data = static_cast<uint32_t *>(x);
					return true;
				}
			}
			return false;
		}

		void init() {
			uint32_t record_end_offset;
			UINT32_UNPACK(reinterpret_cast<char *>(cdb_data), &record_end_offset);
			cdb_records_end = reinterpret_cast<uint32_t *>((reinterpret_cast<char *>(cdb_data)) + record_end_offset);
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
			UINT32_UNPACK(reinterpret_cast<char *>(current), &klen);
			current++;
			UINT32_UNPACK(reinterpret_cast<char *>(current), dlen);
			current++;
			string key(reinterpret_cast<char *>(current), klen);
			current = reinterpret_cast<uint32_t*>((reinterpret_cast<char *>(current)) + (klen));
			*data = current;
			current = reinterpret_cast<uint32_t*>((reinterpret_cast<char *>(current)) + (*dlen));
			return key;
		}

		bool end() {
			return current >= cdb_records_end;
		}

		bool isReady() {
			return is_ready;
		}
};

bool CdbCache::readCategory(Category &vec) throw(ExBasic)
{
	string cdbfile = m_prefix + PORTAGE_CACHE_PATH + m_scheme + vec.name() + ".cdb";
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
		if( ! unpickle_get_mapping(static_cast<char *>(data), dlen, mapping)) {
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
		version->set_full_keywords(mapping["KEYWORDS"]);
		version->slot = mapping["SLOT"];
		version->set_iuse(mapping["IUSE"]);
		version->set_restrict(mapping["RESTRICT"]);
		version->overlay_key = m_overlay_key;
		pkg->addVersionFinalize(version);

		/* Free split */
		free(aux[0]);
		free(aux[1]);
	}

	return true;
}
