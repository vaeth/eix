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

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <portage/version.h>

#include <eixTk/exceptions.h>

using namespace std;

/** A class to represent a package in portage It contains various information
 * about a package, including a list of versions. */
class Package : public vector<Version*> {

	public:
		typedef off_t          offset_type;
		typedef unsigned short size_type;

	public:
		/** True if duplicated versions are found in for this package.
		 * That means i.e. the version 0.2 is found in two overlays. */
		bool have_duplicate_versions;

		unsigned short overlay_key;  /**< Key for Portagedb.overlays/overlaylist from header. */

		bool have_same_overlay_key;  /**< True if all versions come from one overlay. */
		bool is_system_package;      /**< True if every version is in the system-profile. */

		/** Package properties (stored in db) */
		string category, name, desc, homepage, licenses, installed_versions, provide;
	
	public:
		/// Preset with defaults
		Package()
		{ defaults(); }

		/// Fill in name and category and preset with defaults
		Package(string c, string n) 
			: category(c), name(n)
		{ defaults(); }

		void defaults() {
			is_system_package = false;
			have_same_overlay_key = true;
			have_duplicate_versions = false;
		}

		/** De-constructor, delete content of Version-vector. */
		~Package();

		/** Write the package to an eix db.
		 * @param os An FILE-stream to which the package entry should be written */
		void write(FILE *os) throw (ExBasic);

		/** Adds a version to "the versions" vector, */
		void addVersion(Version *vex);

		/** Check if a package has duplicated versions. */
		bool checkDuplicates(Version *version = NULL);

		Version *best() {
			Version *ret = NULL;
			for(reverse_iterator ri = rbegin();
				ri != rend();
				++ri)
			{
				if((*ri)->isStable() && !(*ri)->isHardMasked()) {
					ret = *ri;
					break;
				}
			}
			return ret;
		}
};

#endif /* __PACKAGE_H__ */
