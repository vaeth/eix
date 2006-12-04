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

#ifndef __DBHEADER_H__
#define __DBHEADER_H__

#include <set>
#include <vector>
#include <string>

#include <portage/version.h>

/** Representation of a database-header.
 * Contains your arch, the version of the db, the number of packages/categories
 * and a table of key->directory mappings. */
class DBHeader {

	private:
		/** The mapping from key->directory. */
		std::vector<std::string> overlays;

	public:
		/** Current version of database-format. */
		static const int current = 18;

		int version; /**< Version of the db. */
		unsigned int size; /**< Number of categories. */

		/** Get string for key from directory-table. */
		std::string getOverlay(Version::Overlay key) const;

		/** Add overlay to directory-table and return key. */
		Version::Overlay addOverlay(std::string overlay);

		/** Find first overlay-number >=minimal for name.
		    Name might be either a filename or a number string.
		    The special name portdir (if defined) matches 0.
		    The special name '' matches everything but 0. */
		bool find_overlay(Version::Overlay *num, const char *name, const char *portdir, Version::Overlay minimal = 0, bool test_saved_portdir = false) const;

		/** Add all overlay-numbers >=minimal for name to vec (name might be a number string). */
		void get_overlay_vector(std::set<Version::Overlay> *overlays, const char *name, const char *portdir, Version::Overlay minimal = 0, bool test_saved_portdir = false) const;

		Version::Overlay countOverlays() const
		{ return (Version::Overlay)(overlays.size()); }

		bool isCurrent() const
		{ return version == DBHeader::current; }
};
#endif /* __DBHEADER_H__ */
