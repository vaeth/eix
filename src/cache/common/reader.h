// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_COMMON_READER_H_
#define SRC_CACHE_COMMON_READER_H_ 1

#include <ctime>

#include <string>

#include "cache/base.h"
#include "eixTk/null.h"
#include "eixTk/unused.h"

class BasicCache;
class Depend;
class Package;

/**
Parent class of all readers
**/
class BasicReader {
	public:
		explicit BasicReader(BasicCache *cache) : m_cache(cache) {
		}

		/**
		Virtual deconstructor
		**/
		virtual ~BasicReader() {
		}

		virtual const char *get_md5sum(const std::string& filename ATTRIBUTE_UNUSED) ATTRIBUTE_NONNULL_ {
			UNUSED(filename);
			return NULLPTR;
		}

		virtual bool get_mtime(time_t *t ATTRIBUTE_UNUSED, const std::string& filename ATTRIBUTE_UNUSED) ATTRIBUTE_NONNULL_ {
			UNUSED(t);
			UNUSED(filename);
			return false;
		}

		virtual void get_keywords_slot_iuse_restrict(const std::string& filename, std::string *eapi, std::string *keywords, std::string *slotname, std::string *iuse, std::string *required_use, std::string *restr, std::string *props, Depend *dep) ATTRIBUTE_NONNULL_ = 0;

		virtual void read_file(const std::string& filename, Package *pkg) ATTRIBUTE_NONNULL_ = 0;

	public:
		BasicCache *m_cache;
};

#endif  // SRC_CACHE_COMMON_READER_H_
