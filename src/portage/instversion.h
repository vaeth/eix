// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __INSTVERSION_H__
#define __INSTVERSION_H__

#include <portage/basicversion.h>
#include <portage/version.h>
#include <set>

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

		time_t instDate;                   /**< Installation date according to vardbpkg */
		std::vector<std::string> inst_iuse;/**< Useflags in iuse according to vardbpkg  */
		std::set<std::string> usedUse;     /**< Those useflags in iuse actually used    */

		/** Similarly for overlay_keys */
		bool know_overlay, overlay_failed;
		Version::Overlay overlay_key;
		/** overlay_keytext is at most set if overlay_failed */
		std::string overlay_keytext;

		void init() {
			know_slot = false;
			read_failed = false;
			know_use = false;
			know_restricted = false;
			instDate = 0;
			know_overlay = false;
		}

		InstVersion()
		{ init(); }

		/** Constructor, calls BasicVersion::parseVersion( str ) */
		InstVersion(const char* str) : ExtendedVersion(str)
		{ init(); }

		/** The equality operator does not test the additional data */
		bool operator == (const InstVersion &v) const
		{ return (*(dynamic_cast<const BasicVersion *>(this)) == dynamic_cast<const BasicVersion&>(v)); }

		bool operator == (const BasicVersion &v) const
		{ return (*(dynamic_cast<const BasicVersion *>(this)) == v); }

		bool operator != (const InstVersion &v) const
		{ return !((*this) == v); }

		bool operator != (const BasicVersion &v) const
		{ return !((*this) == v); }

};

#endif /* __INSTVERSION_H__ */
