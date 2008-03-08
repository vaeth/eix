// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "packagetree.h"

#include <eixTk/stringutils.h> // sort_uniquify

using namespace std;

void PackageTree::add_missing_categories(vector<string> &categories) const
{
	categories.reserve(size());
	for(const_iterator i = begin();
			i != end();
			++i)
	{
		categories.push_back(i->name);
	}
	sort_uniquify(categories);
}

io::Treesize
PackageTree::countPackages() const
{
	io::Treesize ret = 0;
	for(const_iterator i = begin(); i != end(); ++i)
	{
		ret += i->size();
	}
	return ret;
}
