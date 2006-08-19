/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "packagetree.h"

#include <portage/package.h>
#include <database/header.h>
#include <database/package_reader.h>
#include <database/io.h>
#include <algorithm>

using namespace std;

Package *
Category::findPackage(const string &name) const
{
	for(const_iterator i = begin(); i != end(); ++i)
	{
		if(i->name == name)
			return *i;
	}
	return NULL;
}

bool
Category::deletePackage(const string &name)
{
	for(iterator i = begin(); i != end(); ++i)
	{
		if(i->name == name)
		{
			delete *i;
			erase(i);
			return true;
		}
	}
	return false;
}

Package *
Category::addPackage(string name)
{
	Package *p = new Package(m_name, name);
	push_back(p);
	return p;
}

Category::iterator 
Category::find(const std::string &name)
{
	iterator i = begin(); 
	for(;
		i != end();
		++i)
	{
		if(i->name == name)
			return i;
	}
	return i;
}

Package *
PackageTree::findPackage(const string &category, const string &name) const
{
	Category *f = find(category);
	if(f)
		return f->findPackage(name);
	return NULL;
}

bool
PackageTree::deletePackage(const string &category, const string &name)
{
	iterator i = begin();
	for(; i != end(); ++i)
	{
		if(i->name() == category)
			break;
	}

	if(i == end())
		return false;

	bool ret = i->deletePackage(name);

	// Check if the category is empty after deleting the
	// package.
	if(i->empty())
		erase(i);

	return ret;
}

unsigned int
PackageTree::countPackages() const
{
	Category::size_type ret = 0;
	for(const_iterator i = begin(); i != end(); ++i)
	{
		ret += i->size();
	}
	return ret;
}

Category *PackageTree::find(const string name) const
{
	if(fast_access)
	{
		map<string,Category*>::const_iterator f = fast_access->find(name);
		if(f == fast_access->end())
			return NULL;
		return f->second;
	}
	for(const_iterator i = begin();
		i != end();
		++i)
	{
		if(i->name() == name)
			return *i;
	}
	return NULL;
}

Category &PackageTree::insert(const string name)
{
	Category *p = new Category(name);
	bool inserted = false;
	for(iterator i = begin(); i != end(); ++i)
	{
		if(name <= i->name())
		{
			if(name == i->name()) { // We already had this category
				delete p;
				return **i;
			}
			((eix::ptr_list<Category> *)this)->insert(i,p);
			inserted = true;
			break;
		}
	}
	if(!inserted)
		((eix::ptr_list<Category> *)this)->push_back(p);
	if(fast_access)
		(*fast_access)[name] = p;
	return *p;
}

void PackageTree::add_missing_categories(vector<string> &categories) const
{
	for(const_iterator i = begin();
		i != end();
		++i)
	{
		categories.push_back(i->name());
	}
	std::sort(categories.begin(), categories.end());
	categories.erase(std::unique(categories.begin(), categories.end()),
			categories.end());
}

void PackageTree::need_fast_access(const vector<string> *add_cat)
{
	if(fast_access)
		return;
	fast_access = new std::map<std::string, Category*>;
	for(iterator i = begin(); i != end(); ++i)
		(*fast_access)[i->name()] = *i;
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

