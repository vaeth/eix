// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__PORT2_1_2_H__
#define EIX__PORT2_1_2_H__ 1

#include <config.h>
#include <cache/base.h>
#include <eixTk/exceptions.h>

#include <map>
#include <string>

class Category;
class PackageTree;

class Port2_1_2_Cache : public BasicCache {
	private:
		bool readEntry(std::map<std::string,std::string> &mapper, PackageTree *packagetree, const char *cat_name, Category *category);

	public:
		bool can_read_multiple_categories() const ATTRIBUTE_CONST_VIRTUAL
		{ return true; }

		bool readCategories(PackageTree *packagetree, const char *cat_name, Category *category) throw(ExBasic);

		const char *getType() const
		{ return "portage-2.1"; }
};

#endif /* EIX__PORT2_1_2_H__ */
