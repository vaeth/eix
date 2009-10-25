// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "packagetree.h"

#include <database/types.h>
#include <eixTk/likely.h>
#include <portage/package.h>

#include <string>
#include <vector>

#include <cstddef>

using namespace std;

Category::iterator
Category::find(const std::string &pkg_name)
{
	for(iterator i = begin(); likely(i != end()); ++i) {
		if(unlikely(i->name == pkg_name))
			return i;
	}
	return end();
}

Category::const_iterator
Category::find(const std::string &pkg_name) const
{
	for(const_iterator i = begin(); likely(i != end()); ++i) {
		if(unlikely(i->name == pkg_name))
			return i;
	}
	return end();
}

#if 0
bool
Category::deletePackage(const std::string &pkg_name)
{
	iterator i(find(pkg_name));
	if(i == end())
		return false;
	delete *i;
	erase(i);
	return true;
}
#endif

Package *
Category::addPackage(const string cat_name, const string &pkg_name)
{
	Package *p(new Package(cat_name, pkg_name));
	addPackage(p);
	return p;
}


Category *PackageTree::find(const string &cat_name) const
{
	const_iterator f = Categories::find(cat_name);
	if(unlikely(f == end()))
		return NULL;
	return f->second;
}

Category &PackageTree::insert(const string &pkg_name)
{
	pair<Categories::iterator,bool> n(Categories::insert(Categories::value_type(pkg_name, NULL)));
	Category *&catpoint = (n.first)->second;
	if(n.second) {
		return *(catpoint = new Category);
	}
	return *catpoint;
}

Package *
PackageTree::findPackage(const string &cat_name, const string &pkg_name) const
{
	const_iterator f = Categories::find(cat_name);
	if(unlikely(f == end()))
		return NULL;
	return f->second->findPackage(pkg_name);
}

#if 0
bool
PackageTree::deletePackage(const string &cat_name, const string &pkg_name)
{
	iterator i = Categories::find(cat_name);
	if(i == end());
		return false;

	if(i->deletePackage(pkg_name))
		return false;
	// Check if the category is empty after deleting the package.
	if(unlikely(i->empty())) {
		erase(i);
	}
	return true;
}
#endif

io::Treesize
PackageTree::countPackages() const
{
	io::Treesize ret(0);
	for(const_iterator i(begin()); likely(i != end()); ++i) {
		ret += i->second->size();
	}
	return ret;
}

PackageTree::~PackageTree()
{
	for(Categories::const_iterator it(Categories::begin());
		likely(it != Categories::end()); ++it) {
		delete it->second;
	}
}
