// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __EIXCACHE_H__
#define __EIXCACHE_H__

#include <cache/base.h>

class EixCache : public BasicCache {
	private:
		std::string m_name, m_file, m_overlay;
		bool m_only_overlay;
		Version::Overlay m_get_overlay;
		bool never_add_categories;

	public:
		// @return true if successful
		bool initialize(const std::string &name);

		bool can_read_multiple_categories() const
		{ return true; }

		bool readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category = NULL) throw(ExBasic);

		const char *getType() const
		{ return m_name.c_str(); }
};

#endif /* __EIXCACHE_H__ */
