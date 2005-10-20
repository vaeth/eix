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

#ifndef __DBBODY_H__
#define __DBBODY_H__

#include <map>
#include <string>

#include <portage/mask.h>

#include <database/header.h>
#include <database/basicio.h>

using namespace std;

class CategoryHeader {
	public:
		typedef unsigned int size_type;

		static size_type read(FILE *stream, string &name) {
			name = io::read_string(stream);
			return io::read<size_type>(stream);
		}

		static void write(FILE *stream, const string &name, const size_type size) {
			io::write_string(stream,  name);
			io::write<size_type>(stream, size);
		}
};

/** Body of a database. */
class PackageDatabase {

	private:
		/** Contains all categories. */
		map<string, vector<Package*> > _packages;

	public:
		typedef map<string, vector<Package*> >::iterator mapiter;
		typedef vector<Package*>::iterator               pkgiter;

		/** Build skeleton.  */
		PackageDatabase(vector<string> *categories) {
			for(vector<string>::iterator it = categories->begin(); it != categories->end(); ++it) {
				_packages[*it] = vector<Package*>(0);
			}
		}

		PackageDatabase() {
		}

		/** Free all packages. */
		~PackageDatabase() {
			for(mapiter m = _packages.begin(); m != _packages.end(); ++m) {
				for(pkgiter p = m->second.begin(); p != m->second.end(); ++p) {
					delete *p;
				}
			}
		}

		pkgiter find(pkgiter first, pkgiter last, const string& n) {
			while(first != last && (*first)->name != n) ++first;
			return first;
		}

		void checkMasks(MaskList *masks) {
			for(mapiter m = _packages.begin(); m != _packages.end(); ++m) {
				for(pkgiter p = m->second.begin(); p != m->second.end(); ++p) {
					for(MaskList::viterator vit = masks->get(*p)->begin(); vit != masks->get(*p)->end(); ++vit) {
						(*vit)->checkMask(**p, false, false);
					}
				}
			}
		}

		/** Find Package in Category. */
		Package *findPackage(string &category, string &name) {
			pkgiter i = _packages[category].begin();
			i = find(i, _packages[category].end(), name);
			if(i != _packages[category].end()) {
				return *i;
			}
			return NULL;
		}

		/** Make new Package in a category. */
		Package *newPackage(string category, string name) {
			Package *c = new Package(category, name) ;
			OOM_ASSERT(c);
			_packages[category].push_back(c);
			return c;
		}

		bool deletePackage(string &category, string &name) {
			pkgiter i = _packages[category].begin();
			i = find(i, _packages[category].end(), name);
			if(i != _packages[category].end()) {
				delete *i;
				_packages[category].erase(i);
				return true;
			}
			return false;
		}

		/** Return iterator pointed to begining of categories. */
		mapiter begin() {
			return _packages.begin();
		}

		/** Return iterator pointed to end of categories. */
		mapiter end() {
			return _packages.end();
		}

		/** Return number of Packages. */
		vector<Package*>::size_type countPackages() {
			vector<Package*>::size_type i = 0;
			for(mapiter it = _packages.begin(); it != _packages.end(); ++it) {
				i += it->second.size();
			}
			return i;
		}

		/** Return number of categories. */
		map<string, vector<Package*> >::size_type countCategories() {
			return _packages.size();
		}

		/** Write PackageDatabase to file pointed to by stream. */
		size_t write(FILE *stream) {
			for(mapiter c = _packages.begin(); c != _packages.end(); ++c) {
				/* Write category-header followed by a list of the packages. */
				vector<Package>::size_type s = c->second.size();
				CategoryHeader::write(stream, c->first, s);
#if 0
				io::write_string(stream,  c->first);
				DB_WRITE_GENERIC(stream, s, vector<Package>::size_type);
#endif
				for(pkgiter p = c->second.begin(); p != c->second.end(); ++p) {
					/* write package to stream */
					(*p)->write(stream);
				}
			}
			return countPackages();
		}

		unsigned int read(DBHeader *header, FILE *stream) {
			vector<Package*>::size_type size;
			string                      name;

			for(unsigned int i = 0; i < header->numcategories; i++) {
				size = CategoryHeader::read(stream, name);
#if 0
				io::read_string(stream,  name);
				DB_READ_GENERIC(stream, size, vector<Package>::size_type);
#endif
				for(unsigned int i = 0; i<size; i++) {
					fseeko(stream, sizeof(Package::offset_type) , SEEK_CUR);
					Package *pkg = new Package(stream);
					OOM_ASSERT(pkg);
					pkg->category = name;
					pkg->readMissing();
					_packages[name].push_back(pkg);
				}
			}
			return header->numcategories;
		}
};

#endif /* __DBBODY_H__ */
