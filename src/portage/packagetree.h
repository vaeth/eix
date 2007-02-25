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

#ifndef __PACKAGETREE_H__
#define __PACKAGETREE_H__

#include <map>
#include <string>
#include <list>
#include <vector>

#include <eixTk/ptr_list.h>
#include <database/io.h>

class Package;
class DBHeader;

class Category : public eix::ptr_list<Package> {

	public:
		Category(std::string pkg_name)
		{ m_name = pkg_name; }

		~Category()
		{ delete_and_clear(); }

		Package *findPackage(const std::string &pkg_name) const;
		bool deletePackage(const std::string &pkg_name);
		Package *addPackage(std::string pkg_name);

		const std::string &name() const
		{ return m_name; }

		iterator find(const std::string &name);

	protected:
		std::string m_name;
};

class PackageTree : public eix::ptr_list<Category> {

	public:
		PackageTree() : fast_access(NULL)
		{ }

		~PackageTree()
		{ delete_and_clear(); }

		Package *findPackage(const std::string &category, const std::string &name) const;
		bool deletePackage(const std::string &category, const std::string &name);

		Category *find(const std::string name) const;
		Category &insert(const std::string name);

		Category &operator [] (const std::string name)
		{
			Category *p=find(name);
			if(p)
				return *p;
			return insert(name);
		}
		void need_fast_access(const std::vector<std::string> *add_cat);
		void finish_fast_access();

		void add_missing_categories(std::vector<std::string> &categories) const;

		io::Treesize countPackages() const;
		io::Catsize countCategories() const
		{ return size(); }
	protected:
		std::map<std::string, Category*> *fast_access;
};

#endif /* __PACKAGETREE_H__ */
