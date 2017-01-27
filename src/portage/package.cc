// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include "eixTk/likely.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/keywords.h"
#include "portage/package.h"
#include "portage/version.h"

Package::~Package() {
	delete_and_clear();
}

const Package::Duplicates
	Package::DUP_NONE,
	Package::DUP_SOME,
	Package::DUP_OVERLAYS;

const Package::Versioncollects
	Package::COLLECT_NONE,
	Package::COLLECT_HAVE_NONTRIVIAL_SLOTS,
	Package::COLLECT_HAVE_SAME_OVERLAY_KEY,
	Package::COLLECT_AT_LEAST_TWO_OVERLAYS,
	Package::COLLECT_HAVE_MAIN_REPO_KEY,
	Package::COLLECT_DEFAULT;

void Package::defaults() {
	know_upgrade_slots = m_has_cached_slotlist =
		m_has_cached_subslots = false;
	have_duplicate_versions = DUP_NONE;
	version_collects = COLLECT_DEFAULT;
	local_collects.set(MaskFlags::MASK_NONE);
#ifdef HAVE_ARRAY_CLASS
	saved_collects.fill(MaskFlags(MaskFlags::MASK_NONE));
#endif
}

void Package::addVersionStart(Version *version) {
	Duplicates check_duplicates((have_duplicate_versions == DUP_OVERLAYS) ?
		DUP_NONE : ((version->overlay_key != 0) ? DUP_OVERLAYS :
				((have_duplicate_versions == DUP_SOME) ?
					DUP_NONE : DUP_SOME)));
	bool have_inserted(false);
	for(iterator i(begin()); likely(i != end()); ++i) {
		if(!have_inserted && (*version < **i)) {
			insert(i, version);
			if(check_duplicates == DUP_NONE) {
				return;
			}
			have_inserted = true;
		}
		if((check_duplicates == DUP_NONE) ||
			likely(BasicVersion::compare(**i, *version) != 0)) {
			continue;
		}
		if(check_duplicates == DUP_SOME) {
			have_duplicate_versions = DUP_SOME;
			if(likely(have_inserted)) {
				return;
			}
			check_duplicates = DUP_NONE;
			continue;
		}
		// checkDuplicates == DUP_OVERLAYS
		if(i->overlay_key) {
			have_duplicate_versions = DUP_OVERLAYS;
			if(likely(have_inserted)) {
				return;
			}
			check_duplicates = DUP_NONE;
		} else {
			have_duplicate_versions = DUP_SOME;
		}
	}
	if(unlikely(have_inserted)) {
		return;
	}
	push_back(version);
}

void Package::collect_iuse(Version *version) {
	if(version->iuse.empty()) {
		return;
	}
	/**
	collect iuse
	**/
	iuse.insert(version->iuse);
}

/**
Finishes addVersionStart() after the remaining data have been filled
**/
void Package::addVersionFinalize(Version *version) {
	ExtendedVersion::Overlay key(version->overlay_key);

	if(key == 0) {
		version_collects |= COLLECT_HAVE_MAIN_REPO_KEY;
	}

	/* This guarantees that we pushed our first version */
	if(size() != 1) {
		if(largest_overlay != key) {
			version_collects &= ~COLLECT_HAVE_SAME_OVERLAY_KEY;
			if(largest_overlay && key)
				version_collects |= COLLECT_AT_LEAST_TWO_OVERLAYS;
			if(largest_overlay < key)
				largest_overlay = key;
		}
		local_collects.setbits(version->maskflags.get());
	} else {
		largest_overlay       = key;
		version_collects      = COLLECT_DEFAULT;
		local_collects        = version->maskflags;
	}
	if(!(version->slotname).empty()) {
		version_collects |= COLLECT_HAVE_NONTRIVIAL_SLOTS;
	}

	collect_iuse(version);

	// We must recalculate the complete slotlist after each modification.
	// The reason is that the pointers might go into nirvana, because
	// a push_back might move the whole list.

	// Mark current slotlist as invalid.
	m_has_cached_slotlist = m_has_cached_subslots = false;
}

/**
Call this after modifying system or world state of versions
**/
void Package::finalize_masks() {
	local_collects.set(MaskFlags::MASK_NONE);
	for(iterator i(begin()); likely(i != end()); ++i) {
		local_collects.setbits(i->maskflags.get());
	}
}
