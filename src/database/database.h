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

#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

class Package;
class DBHeader;

using namespace std;

class Category : public vector<Package*> {

	public:
		~Category();

		iterator find(const string &name);

		const_iterator find(const string &name) const
		{ return const_iterator(find(name)); }
};

/** Body of a database. */
class PackageDatabase : public map<string, Category> {

	public:
		/** Find Package in Category. */
		Package *findPackage(const string &category, const string &name) const;

		bool deletePackage(const string &category, const string &name);

		/** Return number of Packages. */
		Category::size_type countPackages() const;

		/** Return number of categories. */
		size_type countCategories() const
		{ return size(); }

		// Write package-tree to fp
		size_t write(FILE *fp) const;

		unsigned int read(DBHeader *header, FILE *fp);
};

#endif /* __DATABASE_H__ */
