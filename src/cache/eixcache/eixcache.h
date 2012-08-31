// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_CACHE_EIXCACHE_EIXCACHE_H_
#define SRC_CACHE_EIXCACHE_EIXCACHE_H_ 1

#include <string>
#include <vector>

#include "cache/base.h"
#include "eixTk/ptr_list.h"
#include "portage/extendedversion.h"

class Category;
class DBHeader;
class Package;
class PackageTree;

class EixCache : public BasicCache {
	private:
		static eix::ptr_list<EixCache> *all_eixcaches;
		bool slavemode;
		std::string err_msg;
		std::string m_name, m_file, m_overlay, m_full;
		bool m_only_overlay;
		ExtendedVersion::Overlay m_get_overlay;
		bool never_add_categories;
		Category *dest_cat;

		void setSchemeFinish();
		void allerrors(const std::vector<EixCache*> &slaves, const std::string &msg);
		void thiserror(const std::string &msg);
		bool get_overlaydat(const DBHeader &header);
		bool get_destcat(PackageTree *packagetree, const char *cat_name, Category *category, const std::string &pcat) ATTRIBUTE_NONNULL_;
		void get_package(Package *p) ATTRIBUTE_NONNULL_;

	public:
		~EixCache();

		// @return true if successful
		bool initialize(const std::string &name);

		bool can_read_multiple_categories() const ATTRIBUTE_CONST_VIRTUAL
		{ return true; }

		bool readCategories(PackageTree *packagetree, const char *name, Category *category);

		const char *getType() const
		{ return m_name.c_str(); }
};

#endif  // SRC_CACHE_EIXCACHE_EIXCACHE_H_
