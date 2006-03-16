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

#include "database.h"

#include <portage/package.h>
#include <database/header.h>
#include <database/package_reader.h>
#include <database/io.h>

Category::~Category()
{
	for(iterator p = begin();
		p != end();
		++p) 
	{
		delete *p;
	}
}

Category::iterator
Category::find(const string &name)
{
	iterator i = begin();
	for(; i != end(); ++i)
	{
		if((*i)->name == name)
			break;
	}
	return i;
}

Category::const_iterator
Category::find(const string &name) const
{
	const_iterator i = begin();
	for(; i != end(); ++i)
	{
		if((*i)->name == name)
			break;
	}
	return i;
}

Package *
PackageDatabase::findPackage(const string &category, const string &name) const 
{
	const_iterator i = begin();
	for(; i != end(); ++i)
	{
		if(i->first == category)
			break;
	}

	if(i != end())
	{
		Category::const_iterator p = i->second.find(name);
		if(p != i->second.end()) 
		{
			return *p;
		}
	}
	return NULL;
}

bool 
PackageDatabase::deletePackage(const string &category, const string &name) 
{
	iterator i = begin();
	for(; i != end(); ++i)
	{
		if(i->first == category)
			break;
	}

	if(i == end())
		return false;

	Category::iterator p = i->second.find(name);

	if(p == i->second.end()) 
		return false;

	i->second.erase(p);

	// Check if the category is empty after deleting the
	// package.
	if(i->second.size() == 0) 
	{
		erase(i);
	}
	return true;
}

Category::size_type
PackageDatabase::countPackages() const
{
	Category::size_type i = 0;
	for(const_iterator it = begin(); it != end(); ++it) 
	{
		i += it->second.size();
	}
	return i;
}

size_t
PackageDatabase::write(FILE *fp) const
{
	for(const_iterator ci = begin(); ci != end(); ++ci) 
	{
		// Write category-header followed by a list of the packages.
		io::write_category_header(fp, ci->first, ci->second.size());

		for(Category::const_iterator p = ci->second.begin();
			p != ci->second.end();
			++p) 
		{
			// write package to fp
			(*p)->sort();
			io::write_package(fp, **p);
		}
	}
	return countPackages();
}

unsigned int 
PackageDatabase::read(DBHeader *header, FILE *fp) 
{
	PackageReader reader(fp, header->numcategories);
	Package *p = NULL;

	while(reader.next())
	{
		p = reader.release();
		(*this)[p->category].push_back(p);
	}
	return header->numcategories;
}
