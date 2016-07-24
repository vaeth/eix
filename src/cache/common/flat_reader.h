// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_COMMON_FLAT_READER_H_
#define SRC_CACHE_COMMON_FLAT_READER_H_ 1

#include <fstream>
#include <string>

#include "cache/common/reader.h"
#include "eixTk/eixint.h"

class BasicCache;
class Depend;
class Package;

class FlatReader : public BasicReader {
	public:
		explicit FlatReader(BasicCache *cache) : BasicReader(cache) {
		}

		void get_keywords_slot_iuse_restrict(const std::string& filename, std::string *eapi, std::string *keywords, std::string *slotname, std::string *iuse, std::string *required_use, std::string *restr, std::string *props, Depend *dep) ATTRIBUTE_NONNULL_;
		void read_file(const char *filename, Package *pkg) ATTRIBUTE_NONNULL_;

	private:
		bool skip_lines(const eix::TinyUnsigned nr, std::ifstream *is, const std::string& filename) const ATTRIBUTE_NONNULL_;
};

#endif  // SRC_CACHE_COMMON_FLAT_READER_H_
