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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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

#include <eixTk/ptr_list.h>

#include <string>

#include <portage/version.h>

/** A class to represent a package in portage It contains various information
 * about a package, including a list of versions. */
class Package
	: public eix::ptr_list<Version>
{
	public:
		/** True if duplicated versions are found in for this package.
		 * That means e.g. that version 0.2 is found in two overlays. */
		typedef char Duplicates;
		static const Duplicates
			DUP_NONE,
			DUP_SOME,    /* Duplicate versions are somewhere */
			DUP_OVERLAYS;/* Duplicate versions are both in overlays */

		Duplicates have_duplicate_versions;

		/** True if all versions come from one overlay. */
		bool have_same_overlay_key;

		/** True if all versions come from at most one overlay. */
		bool at_least_two_overlays;

		/** The largest overlay from which one of the version comes. */
		Version::Overlay largest_overlay;

		/** True if every version is in the system-profile. */
		bool is_system_package;

		/** Package properties (stored in db) */
		std::string category, name, desc, homepage, licenses, installed_versions, provide;

		/// Preset with defaults
		Package()
		{ defaults(); }

		/// Fill in name and category and preset with defaults
		Package(std::string c, std::string n)
			: category(c), name(n)
		{ defaults(); }

		/** De-constructor, delete content of Version-list. */
		~Package();

		/** Adds a version to "the versions" list, */
		void addVersion(Version *vex);

		Version *best() const;

		Version *latest() const
		{ return *rbegin(); }

		void deepcopy(const Package &p);

	protected:
		/** Check if a package has duplicated versions. */
		void checkDuplicates(Version *version);

		void sortedPushBack(Version *v);

		void defaults()
		{
			is_system_package = false;
			have_same_overlay_key = true;
			at_least_two_overlays = false;
			have_duplicate_versions = DUP_NONE;
		}
};

#endif /* __PACKAGE_H__ */
