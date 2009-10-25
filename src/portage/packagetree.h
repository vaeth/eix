// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__PACKAGETREE_H__
#define EIX__PACKAGETREE_H__ 1

#include <database/types.h>
#include <eixTk/ptr_list.h>

#include <map>
#include <string>
#include <vector>

#include <cstddef>

class Package;

class Category : public eix::ptr_list<Package> {

	public:
		Category()
		{ }

		~Category()
		{ delete_and_clear(); }

		iterator find(const std::string &pkg_name);
		const_iterator find(const std::string &pkg_name) const;

		Package *findPackage(const std::string &pkg_name) const
		{
			const_iterator i(find(pkg_name));
			return ((i == end()) ? NULL : (*i));
		}

		void addPackage(Package *pkg)
		{ push_back(pkg); }

		Package *addPackage(const std::string cat_name, const std::string &pkg_name);
};

class PackageTree : public std::map<std::string, Category*> {
	public:
		typedef std::map<std::string, Category*> Categories;
		using Categories::begin;
		using Categories::end;

		PackageTree()
		{ }

		PackageTree(const std::vector<std::string> &cat_vec)
		{ insert(cat_vec); }

		~PackageTree();

		Category *find(const std::string &cat_name) const;
		Category &insert(const std::string &cat_name);
		void insert(const std::vector<std::string> &cat_vec);

		Category &operator[](const std::string &cat_name)
		{ return insert(cat_name); }

		Package *findPackage(const std::string &cat_name, const std::string &pkg_name) const;

		io::Treesize countPackages() const;
		io::Catsize countCategories() const
		{ return size(); }
};

#endif /* EIX__PACKAGETREE_H__ */
