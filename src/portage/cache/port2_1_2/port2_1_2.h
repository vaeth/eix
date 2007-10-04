// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PORT2_1_2_H__
#define __PORT2_1_2_H__

#include <portage/cache/base.h>
#include <map>
#include <string>

class Port2_1_2_Cache : public BasicCache {
	private:
		bool readEntry(std::map<std::string,std::string> &mapper, PackageTree *packagetree, std::vector<std::string> *categories, Category *category = NULL);

	public:
		bool can_read_multiple_categories() const
		{ return true; }

		bool readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category = NULL) throw(ExBasic);

		const char *getType() const
		{ return "portage-2.1*"; }
};

#endif /* __PORT2_1_2_H__ */
