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

#include <string>

#include <database/io.h>
#include <eixTk/pointer_map.h>
#include <portage/package.h>

namespace eix
{
	template<typename T>
	struct get_name : public std::unary_function<T*, const std::string*>
	{
		const std::string* operator()(T* value)
		{ return &value->name; }
	};
}

class Category :
	public eix::pointer_map<std::string, Package, eix::get_name<Package> >
{
	public:
		Category(const std::string& name)
			: name(name)
		{ }

		~Category()
		{ eix::delete_all(begin(), end()); }

		Package* findPackage(const std::string& name)
		{ return get(name); }

		Package* addPackage(const std::string& name)
		{ return insert(new Package(this->name, name)); }

		void deletePackage(const std::string& name)
		{
			iterator it = find(name);
			delete *it;
			erase(it);
		}

		const std::string name;
};

class PackageTree :
	public eix::pointer_map<std::string, Category, eix::get_name<Category> >
{
	public:
		PackageTree(const std::set<std::string> &categories)
		{
			for(std::set<std::string>::const_iterator it(categories.begin());
				it != categories.end();
				++it)
			{
				insert(new Category(*it));
			}
		}

		PackageTree()
		{ }

		void add_missing_categories(std::vector<std::string> &categories) const;

		io::Treesize countPackages() const;

		io::Catsize countCategories() const
		{ return size(); }
};

#endif /* __PACKAGETREE_H__ */
