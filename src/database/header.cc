// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "database/header.h"
#include <config.h>  // IWYU pragma: keep

#include <set>
#include <string>

#include "eixTk/dialect.h"
#include "eixTk/filenames.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "portage/extendedversion.h"

using std::set;
using std::string;

const DBHeader::SaveBitmask
	DBHeader::SAVE_BITMASK_NONE,
	DBHeader::SAVE_BITMASK_DEP,
	DBHeader::SAVE_BITMASK_REQUIRED_USE;

const DBHeader::OverlayTest
	DBHeader::OVTEST_NONE,
	DBHeader::OVTEST_SAVED_PORTDIR,
	DBHeader::OVTEST_PATH,
	DBHeader::OVTEST_ALLPATH,
	DBHeader::OVTEST_LABEL,
	DBHeader::OVTEST_NUMBER,
	DBHeader::OVTEST_NOT_SAVED_PORTDIR,
	DBHeader::OVTEST_ALL;

const DBHeader::DBVersion DBHeader::current;

/**
Which version of database-format we can read. The list must end with 0.
It should be linear, ordered downward (we check successively, and
practically sure the first match should be it; if not, the second.
The remainder is meant for museum systems.)
**/
const DBHeader::DBVersion DBHeader::accept[] = {
	DBHeader::current, 36, 35, 34, 33, 32, 31,
	0
};

const char DBHeader::magic[] = "eix\n";

/**
Get overlay for key from table
**/
const OverlayIdent& DBHeader::getOverlay(ExtendedVersion::Overlay key) const {
	static const OverlayIdent *not_found = NULLPTR;
	if(key > countOverlays()) {
		if(unlikely(not_found == NULLPTR)) {
			not_found = new OverlayIdent("", "");
		}
		return *not_found;
	}
	return overlays[key];
}

/**
Add overlay to directory-table and return key
**/
ExtendedVersion::Overlay DBHeader::addOverlay(const OverlayIdent& overlay) {
	overlays.PUSH_BACK(overlay);
	return countOverlays() - 1;
}

bool DBHeader::find_overlay(ExtendedVersion::Overlay *num, const char *name, const char *portdir, ExtendedVersion::Overlay minimal, OverlayTest testmode) const {
	if(unlikely(minimal > countOverlays())) {
		return false;
	}
	if(*name == '\0') {
		if(countOverlays() == 1) {
			return false;
		}
		*num = (minimal != 0) ? minimal : 1;
		return true;
	}
	if(testmode & OVTEST_LABEL) {
		for(ExtendedVersion::Overlay i(minimal); likely(i != countOverlays()); ++i) {
			if(getOverlay(i).label == name) {
				*num = i;
				return true;
			}
		}
	}
	if(testmode & OVTEST_PATH) {
		if(minimal == 0) {
			if(portdir != NULLPTR) {
				if(same_filenames(name, portdir, true)) {
					*num = 0;
					return true;
				}
			}
		}
		for(ExtendedVersion::Overlay i(minimal); i != countOverlays(); ++i) {
			if(same_filenames(name, getOverlay(i).path.c_str(), true)) {
				if((i == 0) && ((testmode & OVTEST_SAVED_PORTDIR) == OVTEST_NONE)) {
					continue;
				}
				*num = i;
				return true;
			}
		}
	}
	if((testmode & OVTEST_NUMBER) == OVTEST_NONE)
		return false;
	// Is name a number?
	ExtendedVersion::Overlay number;
	if(!is_numeric(name)) {
		return false;
	}
	number = my_atou(name);
	if(number >= countOverlays()) {
		return false;
	}
	if(number < minimal) {
		return false;
	}
	*num = number;
	return true;
}

void DBHeader::get_overlay_vector(set<ExtendedVersion::Overlay> *overlayset, const char *name, const char *portdir, ExtendedVersion::Overlay minimal, OverlayTest testmode) const {
	for(ExtendedVersion::Overlay curr(minimal);
		find_overlay(&curr, name, portdir, curr, testmode); ++curr) {
		overlayset->INSERT(curr);
	}
}

bool DBHeader::isCurrent() const {
	for(const DBVersion *acc(accept); *acc != 0; ++acc) {
		if(version == *acc) {
			return true;
		}
	}
	return false;
}
