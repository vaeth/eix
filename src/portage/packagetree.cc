// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "packagetree.h"

#include <portage/package.h>
#include <algorithm>
#include <cstdlib>

using namespace std;

Package *
Category::findPackage(const string &pkg_name) const
{
	for(const_iterator i = begin(); i != end(); ++i)
	{
		if(i->name == pkg_name)
			return *i;
	}
	return NULL;
}

bool
Category::deletePackage(const string &pkg_name)
{
	for(iterator i = begin(); i != end(); ++i)
	{
		if(i->name == pkg_name)
		{
			delete *i;
			erase(i);
			return true;
		}
	}
	return false;
}

Package *
Category::addPackage(const string &pkg_name)
{
	Package *p = new Package(name, pkg_name);
	push_back(p);
	return p;
}

Category::iterator
Category::find(const std::string &pkg_name)
{
	iterator i = begin();
	for(;
		i != end();
		++i)
	{
		if(i->name == pkg_name)
			return i;
	}
	return i;
}

Package *
PackageTree::findPackage(const string &category, const string &pkg_name) const
{
	Category *f = find(category);
	if(f)
		return f->findPackage(pkg_name);
	return NULL;
}

bool
PackageTree::deletePackage(const string &category, const string &pkg_name)
{
	iterator i = begin();
	for(; i != end(); ++i)
	{
		if(i->name == category)
			break;
	}

	if(i == end())
		return false;

	bool ret = i->deletePackage(pkg_name);

	// Check if the category is empty after deleting the
	// package.
	if(i->empty())
		erase(i);

	return ret;
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

Category *PackageTree::find(const string pkg_name) const
{
	if(fast_access)
	{
		map<string,Category*>::const_iterator f = fast_access->find(pkg_name);
		if(f == fast_access->end())
			return NULL;
		return f->second;
	}
	for(const_iterator i = begin();
		i != end();
		++i)
	{
		if(i->name == pkg_name)
			return *i;
	}
	return NULL;
}

Category &PackageTree::insert(const string pkg_name)
{
	Category *p = new Category(pkg_name);
	bool inserted = false;
	for(iterator i = begin(); i != end(); ++i)
	{
		if(pkg_name <= i->name)
		{
			if(pkg_name == i->name) { // We already had this category
				delete p;
				return **i;
			}
			eix::ptr_list<Category>::insert(i, p);
			inserted = true;
			break;
		}
	}
	if(!inserted)
		eix::ptr_list<Category>::push_back(p);
	if(fast_access)
		(*fast_access)[pkg_name] = p;
	return *p;
}

void PackageTree::add_missing_categories(vector<string> &categories) const
{
	for(const_iterator i = begin();
		i != end();
		++i)
	{
		categories.push_back(i->name);
	}
	sort_uniquify(categories);
}

void PackageTree::need_fast_access(const vector<string> *add_cat)
{
	if(fast_access)
		return;
	fast_access = new std::map<std::string, Category*>;
	for(iterator i = begin(); i != end(); ++i)
		(*fast_access)[i->name] = *i;
	for(vector<string>::const_iterator it = add_cat->begin();
		it != add_cat->end(); ++it)
		(*this)[*it];
}

void PackageTree::finish_fast_access()
{
	if(!fast_access)
		return;
	delete fast_access;
	fast_access = NULL;
	iterator i = begin();
	while(i != end())
	{
		if(i->empty()) {
			erase(i);
			i=begin(); // We must restart after an erase
		}
		else
			++i;
	}
}

