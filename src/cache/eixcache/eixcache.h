// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_EIXCACHE_EIXCACHE_H_
#define SRC_CACHE_EIXCACHE_EIXCACHE_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>
#include <vector>

#include "cache/base.h"
#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/ptr_container.h"
#include "portage/extendedversion.h"

class Category;
class DBHeader;
class Package;
class PackageTree;

class EixCache FINAL : public BasicCache {
	private:
		typedef eix::ptr_container<std::vector<EixCache *> > CachesList;
		static CachesList *all_eixcaches;

		bool slavemode;
		std::string err_msg;
		std::string m_name, m_file, m_overlay, m_full;
		bool m_only_overlay;
		ExtendedVersion::Overlay m_get_overlay;
		bool never_add_categories;
		Category *dest_cat;

		void setSchemeFinish() OVERRIDE;
		void allerrors(const CachesList& slaves, const std::string& msg);
		void thiserror(const std::string& msg);
		bool get_overlaydat(const DBHeader& header);
		bool get_destcat(PackageTree *packagetree, const char *cat_name, Category *category, const std::string& pcat);
		ATTRIBUTE_NONNULL_ void get_package(Package *p);

	public:
		~EixCache();

		// @return true if successful
		bool initialize(const std::string& name);

		ATTRIBUTE_CONST_VIRTUAL bool can_read_multiple_categories() const OVERRIDE {
			return true;
		}

		bool readCategories(PackageTree *packagetree, const char *name, Category *category) OVERRIDE;

		const char *getType() const OVERRIDE {
			return m_name.c_str();
		}
};

#endif  // SRC_CACHE_EIXCACHE_EIXCACHE_H_
