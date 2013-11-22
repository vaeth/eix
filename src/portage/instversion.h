// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_INSTVERSION_H_
#define SRC_PORTAGE_INSTVERSION_H_ 1

#include <ctime>

#include "eixTk/eixint.h"
#include "eixTk/stringtypes.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/version.h"

/** InstVersion expands the BasicVersion class by data relevant for vardbpkg */
class InstVersion : public ExtendedVersion, public Keywords {
	public:
		/** For versions in vardbpkg we might not yet know the slot.
		    For caching, we mark this here: */
		bool know_slot, read_failed;
		/** Similarly for iuse and usedUse: */
		bool know_use;
		/** And the same for restricted: */
		bool know_restricted;
		/** and for deps: */
		bool know_deps;

		time_t instDate;    /**< Installation date according to vardbpkg */
		WordVec inst_iuse;  /**< Useflags in iuse according to vardbpkg  */
		WordSet usedUse;    /**< Those useflags in iuse actually used    */

		/** Similarly for overlay_keys */
		bool know_overlay, overlay_failed;

		void init() {
			know_slot = false;
			read_failed = false;
			know_use = false;
			know_restricted = false;
			know_deps = false;
			instDate = 0;
			know_overlay = false;
		}

		InstVersion() {
			init();
		}

		static eix::SignedBool compare(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;

		static eix::SignedBool compare(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;

		static eix::SignedBool compare(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
};

// Short compare-stuff
inline static bool operator<(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator<(const ExtendedVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) < 0;
}

inline static bool operator>(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator>(const ExtendedVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) > 0;
}

inline static bool operator==(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator==(const ExtendedVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) == 0;
}

inline static bool operator!=(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator!=(const ExtendedVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) != 0;
}

inline static bool operator>=(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator>=(const ExtendedVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) >= 0;
}

inline static bool operator<=(const ExtendedVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator<=(const ExtendedVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) <= 0;
}

inline static bool operator<(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;
inline static bool operator<(const InstVersion& left, const ExtendedVersion& right) {
	return InstVersion::compare(left, right) < 0;
}

inline static bool operator>(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;
inline static bool operator>(const InstVersion& left, const ExtendedVersion& right) {
	return InstVersion::compare(left, right) > 0;
}

inline static bool operator==(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;
inline static bool operator==(const InstVersion& left, const ExtendedVersion& right) {
	return InstVersion::compare(left, right) == 0;
}

inline static bool operator!=(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;
inline static bool operator!=(const InstVersion& left, const ExtendedVersion& right) {
	return InstVersion::compare(left, right) != 0;
}

inline static bool operator>=(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;
inline static bool operator>=(const InstVersion& left, const ExtendedVersion& right) {
	return InstVersion::compare(left, right) >= 0;
}

inline static bool operator<=(const InstVersion& left, const ExtendedVersion& right) ATTRIBUTE_PURE;
inline static bool operator<=(const InstVersion& left, const ExtendedVersion& right) {
	return InstVersion::compare(left, right) <= 0;
}

inline static bool operator<(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator<(const InstVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) < 0;
}

inline static bool operator>(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator>(const InstVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) > 0;
}

inline static bool operator==(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator==(const InstVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) == 0;
}

inline static bool operator!=(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator!=(const InstVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) != 0;
}

inline static bool operator>=(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator>=(const InstVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) >= 0;
}

inline static bool operator<=(const InstVersion& left, const InstVersion& right) ATTRIBUTE_PURE;
inline static bool operator<=(const InstVersion& left, const InstVersion& right) {
	return InstVersion::compare(left, right) <= 0;
}

#endif  // SRC_PORTAGE_INSTVERSION_H_
