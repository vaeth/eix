// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_PACKAGETREE_H_
#define SRC_PORTAGE_PACKAGETREE_H_ 1

#include <map>
#include <string>

#include "eixTk/eixint.h"
#include "eixTk/null.h"
#include "eixTk/ptr_list.h"
#include "eixTk/stringtypes.h"

class Package;

class Category : public eix::ptr_list<Package> {
	public:
		Category() {
		}

		~Category() {
			delete_and_clear();
		}

		iterator find(const std::string& pkg_name) ATTRIBUTE_PURE;
		const_iterator find(const std::string& pkg_name) const ATTRIBUTE_PURE;

		Package *findPackage(const std::string& pkg_name) const {
			const_iterator i(find(pkg_name));
			return ((i == end()) ? NULLPTR : (*i));
		}

		void addPackage(Package *pkg) ATTRIBUTE_NONNULL_ {
			push_back(pkg);
		}

		Package *addPackage(const std::string cat_name, const std::string& pkg_name);
};

class PackageTree : public std::map<std::string, Category*> {
	public:
		typedef std::map<std::string, Category*> Categories;
		using Categories::begin;
		using Categories::end;

		PackageTree() {
		}

		explicit PackageTree(const WordVec& cat_vec) {
			insert(cat_vec);
		}

		~PackageTree();

		Category *find(const std::string& cat_name) const ATTRIBUTE_PURE;

		Category& insert(const std::string& cat_name);

		void insert(const WordVec& cat_vec);

		Category& operator[](const std::string& cat_name) {
			return insert(cat_name);
		}

		Package *findPackage(const std::string& cat_name, const std::string& pkg_name) const ATTRIBUTE_PURE;

		eix::Treesize countPackages() const ATTRIBUTE_PURE;

		eix::Catsize countCategories() const {
			return size();
		}
};

#endif  // SRC_PORTAGE_PACKAGETREE_H_
