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

using namespace std;

Package *
Category::findPackage(const string &name) const
{
	for(const_iterator i = begin(); i != end(); ++i)
	{
		if(i->name == name)
			return i.ptr();
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
			delete i.ptr();
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

Package *
PackageTree::findPackage(const string &category, const string &name) const 
{
	for(const_iterator i = begin(); i != end(); ++i)
	{
		if(i->name() == category)
			return i->findPackage(name);
	}

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

Category *
PackageTree::operator [] (const string name)
{
	for(iterator i = begin();
		i != end();
		++i)
	{
		if(i->name() == name)
			return i.ptr();
	}

	Category *p = new Category(name);
	push_back(p);
	return p;
}
