// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_COMMON_ASSIGN_READER_H_
#define SRC_CACHE_COMMON_ASSIGN_READER_H_ 1

#include <ctime>

#include <string>

#include "cache/common/reader.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"

class BasicCache;
class Depend;
class Package;

class AssignReader : public BasicReader {
	public:
		explicit AssignReader(BasicCache *cache) :
			BasicReader(cache), currfile(NULLPTR) {
		}

		~AssignReader() {
			if(likely(currfile != NULLPTR)) {
				delete currfile;
				delete cf;
			}
		}

		const char *get_md5sum(const char *filename) ATTRIBUTE_NONNULL_;
		bool get_mtime(time_t *t, const char *filename) ATTRIBUTE_NONNULL_;
		void get_keywords_slot_iuse_restrict(const std::string& filename, std::string *eapi, std::string *keywords, std::string *slotname, std::string *iuse, std::string *required_use, std::string *restr, std::string *props, Depend *dep) ATTRIBUTE_NONNULL_;
		void read_file(const char *filename, Package *pkg) ATTRIBUTE_NONNULL_;

	private:
		bool get_map(const char *file) ATTRIBUTE_NONNULL_;

		std::string *currfile;
		WordMap *cf;
		bool currstate;
};

#endif  // SRC_CACHE_COMMON_ASSIGN_READER_H_
