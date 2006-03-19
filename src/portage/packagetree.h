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

#ifndef __PACKAGETREE_H__
#define __PACKAGETREE_H__

#include <map>
#include <string>
#include <list>

#include <eixTk/ptr_list.h>

class Package;
class DBHeader;

class Category : public eix::ptr_list<Package> {

	public:
		Category(std::string name)
		{ m_name = name; }

		Package *findPackage(const std::string &name) const;
		bool deletePackage(const std::string &name);
		Package *addPackage(std::string name);

		const std::string &name() const
		{ return m_name; }

	protected:
		std::string m_name;
};

class PackageTree : public eix::ptr_list<Category> {

	public:
		Package *findPackage(const std::string &category, const std::string &name) const;
		bool deletePackage(const std::string &category, const std::string &name);

		Category *operator [] (const std::string name);

		unsigned int countPackages() const;
		unsigned int countCategories() const
		{ return size(); }
};

#endif /* __PACKAGETREE_H__ */
