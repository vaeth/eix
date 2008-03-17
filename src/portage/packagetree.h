// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PACKAGETREE_H__
#define __PACKAGETREE_H__

#include <map>
#include <string>
#include <list>
#include <vector>
#include <cstdlib>

#include <eixTk/ptr_list.h>
#include <database/types.h>

class Package;
class DBHeader;

class Category : public eix::ptr_list<Package> {

	public:
		const std::string name;

		Category(const std::string &category_name)
			: name(category_name)
		{ }

		~Category()
		{ delete_and_clear(); }

		Package *findPackage(const std::string &pkg_name) const;
		bool deletePackage(const std::string &pkg_name);
		Package *addPackage(const std::string &pkg_name);

		iterator find(const std::string &name);
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
