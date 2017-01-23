// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_DATABASE_HEADER_H_
#define SRC_DATABASE_HEADER_H_ 1

#include <set>
#include <string>

#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/extendedversion.h"
#include "portage/overlay.h"

class PortageSettings;

/**
Representation of a database-header.
Contains your arch, the version of the db, the number of packages/categories
and a table of key->directory mappings.
**/
class DBHeader {
	private:
		/**
		The mapping from key->directory
		**/
		OverlayVec overlays;

	public:
		StringHash
			eapi_hash,
			license_hash,
			keywords_hash,
			iuse_hash,
			slot_hash,
			depend_hash;

		typedef  eix::UNumber SaveBitmask;
		static CONSTEXPR const SaveBitmask
			SAVE_BITMASK_NONE         = 0x00U,
			SAVE_BITMASK_DEP          = 0x01U,
			SAVE_BITMASK_REQUIRED_USE = 0x02U;

		bool use_depend, use_required_use;

		WordVec world_sets;

		typedef  eix::UNumber DBVersion;

		typedef  eix::UChar OverlayTest;
		static CONSTEXPR const OverlayTest
			OVTEST_NONE              = 0x00U,
			OVTEST_SAVED_PORTDIR     = 0x01U,
			OVTEST_PATH              = 0x02U,
			OVTEST_ALLPATH           = OVTEST_SAVED_PORTDIR|OVTEST_PATH,
			OVTEST_LABEL             = 0x04U,
			OVTEST_NUMBER            = 0x08U,
			OVTEST_NOT_SAVED_PORTDIR = OVTEST_PATH|OVTEST_LABEL|OVTEST_NUMBER,
			OVTEST_ALL               = OVTEST_ALLPATH|OVTEST_LABEL|OVTEST_NUMBER;

		static const char magic[];

		/**
		Current version of database-format and what we accept
		**/
		static CONSTEXPR const DBVersion current = 37;
		static const DBHeader::DBVersion accept[];

		/**
		Version of the db.
		**/
		DBVersion version;
		/**
		Number of categories
		**/
		eix::Catsize size;

		/**
		Get overlay for key from table
		**/
		const OverlayIdent& getOverlay(ExtendedVersion::Overlay key) const;

		/**
		Add overlay to directory-table and return key
		**/
		ExtendedVersion::Overlay addOverlay(const OverlayIdent& overlay);

		/**
		Set Priorities of overlays
		**/
		void set_priorities(PortageSettings *ps);

		/**
		Find first overlay-number >=minimal for name.
		Name might be either a label, a filename, or a number string.
		The special name portdir (if defined) matches 0 (if OVTEST_PATH)
		The special name '' matches everything but 0.
		**/
		bool find_overlay(ExtendedVersion::Overlay *num, const char *name, const char *portdir, ExtendedVersion::Overlay minimal, OverlayTest testmode) const ATTRIBUTE_NONNULL((2, 3));
		bool find_overlay(ExtendedVersion::Overlay *num, const char *name, const char *portdir) const ATTRIBUTE_NONNULL((2, 3)) {
			return find_overlay(num, name, portdir, 0, OVTEST_NOT_SAVED_PORTDIR);
		}

		/**
		Add all overlay-numbers >=minimal for name to vec (name might be a number string)
		**/
		void get_overlay_vector(std::set<ExtendedVersion::Overlay> *overlayset, const char *name, const char *portdir, ExtendedVersion::Overlay minimal, OverlayTest testmode) const ATTRIBUTE_NONNULL((2, 3));
		void get_overlay_vector(std::set<ExtendedVersion::Overlay> *overlayset, const char *name, const char *portdir) const ATTRIBUTE_NONNULL((2, 3)) {
			get_overlay_vector(overlayset, name, portdir, 0, OVTEST_NOT_SAVED_PORTDIR);
		}

		ExtendedVersion::Overlay countOverlays() const {
			return ExtendedVersion::Overlay(overlays.size());
		}

		bool isCurrent() const ATTRIBUTE_PURE;
};

#endif  // SRC_DATABASE_HEADER_H_
