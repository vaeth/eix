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

#ifndef __BASICCACHE_H__
#define __BASICCACHE_H__

#include <portage/package.h>

/** Parent class of every cache that eix can use. */
class BasicCache {

	protected:
		string _scheme;
		short  _overlay_key;
		string _arch;

	public:
		/** Set scheme for this cache. */
		void setScheme(string scheme) {
			_scheme = scheme;
		}

		/** Set overlay-key. */
		void setKey(short key) {
			_overlay_key = key;
		}

		/** Set arch for system. */
		void setArch(string &arch) {
			_arch = arch;
		}

		Package *addPackage(vector<Package*> &v, const string &cat_name, const string name) {
			Package *p = new Package(cat_name, name);
			OOM_ASSERT(p);
			v.push_back(p);
			return p;
		}

		/** Find Package and return pointer to it.
		 *  Returns NULL if not found. */
		Package *findPackage(vector<Package*> &v, const char *name) {
			Package *ret = NULL;
			for(vector<Package*>::iterator i = v.begin(); i != v.end(); ++i) {
				if((*i)->name == name) {
					ret = *i;
				}
			}
			return ret;
		}

		/** Remove and delete Package. */
		bool deletePackage(vector<Package*> &v, string name) {
			bool ret = false;
			for(vector<Package*>::iterator i = v.begin(); i != v.end(); ++i) {
				if((*i)->name == name) {
					delete *i;
					v.erase(i);
					ret = true;
					break;
				}
			}
			return false;
		}

		/* Virtual deconstructor. */
		virtual ~BasicCache() {}

		/** Read Cache for a category with a little from portageif. */
		virtual int readCategory(vector<Package*> &vec, const string &cat_name, void (*error_callback)(const char *fmt, ...)) = 0;

		/** Return name of Cache.*/
		virtual const char *getType() = 0;

		/** Get scheme for this cache. */
		string getScheme() {
			return _scheme;
		}
};

#endif /* __BASICCACHE_H__ */
