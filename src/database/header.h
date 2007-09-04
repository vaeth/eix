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
#include <portage/overlay.h>

/** Representation of a database-header.
 * Contains your arch, the version of the db, the number of packages/categories
 * and a table of key->directory mappings. */
class DBHeader {

	private:
		/** The mapping from key->directory. */
		std::vector<OverlayIdent> overlays;

	public:
		typedef  io::Int DBVersion;
		static const unsigned short DBVersionsize = io::Intsize;

		typedef  io::Char OverlayTest;
		static const OverlayTest
			OVTEST_NONE              = 0x00,
			OVTEST_SAVED_PORTDIR     = 0x01,
			OVTEST_PATH              = 0x02,
			OVTEST_ALLPATH           = OVTEST_SAVED_PORTDIR|OVTEST_PATH,
			OVTEST_LABEL             = 0x04,
			OVTEST_NUMBER            = 0x08,
			OVTEST_NOT_SAVED_PORTDIR = OVTEST_PATH|OVTEST_LABEL|OVTEST_NUMBER,
			OVTEST_ALL               = OVTEST_ALLPATH|OVTEST_LABEL|OVTEST_NUMBER;

		/** Current version of database-format. */
		static const DBVersion current = 23;

		DBVersion version; /**< Version of the db. */
		io::Catsize  size; /**< Number of categories. */

		/** Get overlay for key from directory-table. */
		const OverlayIdent& getOverlay(Version::Overlay key) const;

		/** Add overlay to directory-table and return key. */
		Version::Overlay addOverlay(const OverlayIdent& overlay);

		/** Find first overlay-number >=minimal for name.
		    Name might be either a label, a filename, or a number string.
		    The special name portdir (if defined) matches 0 (if OVTEST_PATH)
		    The special name '' matches everything but 0. */
		bool find_overlay(Version::Overlay *num, const char *name, const char *portdir, Version::Overlay minimal = 0, OverlayTest testmode = OVTEST_NOT_SAVED_PORTDIR) const;

		/** Add all overlay-numbers >=minimal for name to vec (name might be a number string). */
		void get_overlay_vector(std::set<Version::Overlay> *overlays, const char *name, const char *portdir, Version::Overlay minimal = 0, OverlayTest testmode = OVTEST_NOT_SAVED_PORTDIR) const;

		Version::Overlay countOverlays() const
		{ return Version::Overlay(overlays.size()); }

		bool isCurrent() const
		{ return version == DBHeader::current; }
};
#endif /* __DBHEADER_H__ */
