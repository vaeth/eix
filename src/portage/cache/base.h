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

// Add package to vector
Package *addPackage(vector<Package*> &v, const string &cat, const string pkg);

// Find Package and return pointer to it.
Package *findPackage(vector<Package*> &v, const char *pkg);

// Remove and delete Package. */
bool deletePackage(vector<Package*> &v, string pkg);

// Parent class of every cache that eix can use. */
class BasicCache {

	public:
		// Virtual deconstructor. */
		virtual ~BasicCache() {}

		// Set scheme for this cache. */
		void setScheme(string scheme) {
			m_scheme = scheme;
		}

		// Set overlay-key. */
		void setKey(short key) {
			m_overlay_key = key;
		}

		// Set arch for system. */
		void setArch(string &arch) {
			m_arch = arch;
		}

		// Return name of Cache.*/
		virtual const char *getType() const = 0;

		// Set arch for system. */
		void setErrorCallback(void (*error_callback)(const char *fmt, ...)) {
			m_error_callback = error_callback;
		}

		// Read Cache for a category with a little from portageif. */
		virtual int readCategory(vector<Package*> &vec, const string &cat) = 0;

		// Get scheme for this cache. */
		string getScheme() const {
			return m_scheme;
		}

	protected:
		string m_scheme;
		short  m_overlay_key;
		string m_arch;
		void (*m_error_callback)(const char *fmt, ...);

	public:
};

#endif /* __BASICCACHE_H__ */
