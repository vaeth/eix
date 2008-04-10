// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __DBHEADER_H__
#define __DBHEADER_H__

#include <eixTk/stringutils.h>
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
		StringHash
			provide_hash,
			license_hash,
			keywords_hash,
			iuse_hash,
			slot_hash;

		typedef  io::UNumber DBVersion;

		typedef  io::UChar OverlayTest;
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
		static const DBVersion current = 26;

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
