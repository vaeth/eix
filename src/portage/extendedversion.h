// vim:set noet cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_EXTENDEDVERSION_H_
#define SRC_PORTAGE_EXTENDEDVERSION_H_ 1

#include <config.h>

#include <string>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/inttypes.h"
#include "eixTk/stringutils.h"
#include "portage/basicversion.h"
#include "portage/depend.h"
#include "portage/eapi.h"
#include "portage/overlay.h"

class Package;
class PortageSettings;

class ExtendedVersion : public BasicVersion {
	public:
		typedef uint16_t CountBinPkg;

	private:
		typedef eix::UChar HaveBinPkg;
		static CONSTEXPR const HaveBinPkg
			HAVEBINPKG_UNKNOWN = 0x00U,
			HAVEBINPKG_TBZ_NO  = 0x01U,
			HAVEBINPKG_TBZ_YES = 0x02U,
			HAVEBINPKG_TBZ     = HAVEBINPKG_TBZ_NO|HAVEBINPKG_TBZ_YES,
			HAVEBINPKG_PAK     = 0x04U;
		mutable HaveBinPkg have_bin_pkg_m;  // mutable: it is just a cache
		mutable CountBinPkg count_pak_m;

	public:
		typedef uint16_t Restrict;
		static CONSTEXPR const Restrict  // order according to frequency...
			RESTRICT_NONE           = 0x0000U,
			RESTRICT_BINCHECKS      = 0x0001U,
			RESTRICT_STRIP          = 0x0002U,
			RESTRICT_TEST           = 0x0004U,
			RESTRICT_USERPRIV       = 0x0008U,
			RESTRICT_INSTALLSOURCES = 0x0010U,
			RESTRICT_FETCH          = 0x0020U,
			RESTRICT_MIRROR         = 0x0040U,
			RESTRICT_PRIMARYURI     = 0x0080U,
			RESTRICT_BINDIST        = 0x0100U,
			RESTRICT_PARALLEL       = 0x0200U,
			RESTRICT_ALL            = 0x03FFU;

		typedef eix::UChar Properties;
		static CONSTEXPR const Properties  // order according to frequency...
			PROPERTIES_NONE        = 0x00U,
			PROPERTIES_INTERACTIVE = 0x01U,
			PROPERTIES_LIVE        = 0x02U,
			PROPERTIES_VIRTUAL     = 0x04U,
			PROPERTIES_SET         = 0x08U,
			PROPERTIES_ALL         = 0x0FU;

		Restrict restrictFlags;
		Properties propertiesFlags;
		Eapi eapi;

		/**
		The slot, the version represents.
		For saving space, the default "0" is always stored as ""
		**/
		std::string slotname;
		std::string subslotname;

		/**
		The repository name
		**/
		std::string reponame;

		/**
		The dependencies
		**/
		Depend depend;

		typedef eix::UNumber Overlay;
		/**
		Key for Portagedb.overlays/overlaylist from header.
		**/
		Overlay overlay_key;

		typedef OverlayIdent::Priority Priority;
		/**
		Priority of overlay
		**/
		Priority priority;

		ExtendedVersion() :
			have_bin_pkg_m(HAVEBINPKG_UNKNOWN),
			restrictFlags(RESTRICT_NONE),
			propertiesFlags(PROPERTIES_NONE),
			overlay_key(0), priority(0) {
		}

		static Restrict calcRestrict(const std::string& str);

		static Properties calcProperties(const std::string& str);

		static void init_static();

		void set_restrict(const std::string& str) {
			restrictFlags = calcRestrict(str);
		}

		void set_properties(const std::string& str) {
			propertiesFlags = calcProperties(str);
		}

		void set_slotname(const std::string& str) {
			slot_subslot(str, &slotname, &subslotname);
		}

		std::string get_shortfullslot() const {
			return (subslotname.empty() ? slotname : (slotname + "/" + subslotname));
		}

		std::string get_longfullslot() const;

		std::string get_longslot() const {
			return (slotname.empty() ? "0" : slotname);
		}

		void assign_basic_version(const BasicVersion& b) {
			*static_cast<BasicVersion *>(this) = b;
		}

		bool have_bin_pkg(const PortageSettings *ps, const Package *pkg, CountBinPkg minimal) const;
		bool have_bin_pkg(const PortageSettings *ps, const Package *pkg) const;
		bool have_tbz_pkg(const PortageSettings *ps, const Package *pkg) const;
		CountBinPkg num_pak_pkg(const PortageSettings *ps, const Package *pkg) const;

		ATTRIBUTE_PURE static eix::SignedBool compare(const ExtendedVersion& left, const ExtendedVersion& right);
};


// Short compare-stuff
ATTRIBUTE_PURE inline static bool operator<(const ExtendedVersion& left, const ExtendedVersion& right);
inline static bool operator<(const ExtendedVersion& left, const ExtendedVersion& right) {
	return ExtendedVersion::compare(left, right) < 0;
}

ATTRIBUTE_PURE inline static bool operator>(const ExtendedVersion& left, const ExtendedVersion& right);
inline static bool operator>(const ExtendedVersion& left, const ExtendedVersion& right) {
	return ExtendedVersion::compare(left, right) > 0;
}

ATTRIBUTE_PURE inline static bool operator==(const ExtendedVersion& left, const ExtendedVersion& right);
inline static bool operator==(const ExtendedVersion& left, const ExtendedVersion& right) {
	return ExtendedVersion::compare(left, right) == 0;
}

ATTRIBUTE_PURE inline static bool operator!=(const ExtendedVersion& left, const ExtendedVersion& right);
inline static bool operator!=(const ExtendedVersion& left, const ExtendedVersion& right) {
	return ExtendedVersion::compare(left, right) != 0;
}

ATTRIBUTE_PURE inline static bool operator>=(const ExtendedVersion& left, const ExtendedVersion& right);
inline static bool operator>=(const ExtendedVersion& left, const ExtendedVersion& right) {
	return ExtendedVersion::compare(left, right) >= 0;
}

ATTRIBUTE_PURE inline static bool operator<=(const ExtendedVersion& left, const ExtendedVersion& right);
inline static bool operator<=(const ExtendedVersion& left, const ExtendedVersion& right) {
	return ExtendedVersion::compare(left, right) <= 0;
}


#endif  // SRC_PORTAGE_EXTENDEDVERSION_H_
