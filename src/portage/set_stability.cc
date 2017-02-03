// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/set_stability.h"
#include <config.h>

#ifndef ALWAYS_RECALCULATE_STABILITY
#ifndef NDEBUG
#ifdef EIX_PARANOIC_ASSERT
#include <cstdlib>

#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#endif
#endif
#endif

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "portage/conf/cascadingprofile.h"
#include "portage/conf/portagesettings.h"
#include "portage/keywords.h"
#include "portage/package.h"
#include "portage/packagetree.h"
#include "portage/version.h"

#ifndef ALWAYS_RECALCULATE_STABILITY

/*
Calculating the index manually makes it sometimes unnecessary
to recalculate the stability setting of the whole package.
Of course, this is clumsy, because we must take care about how
the "saved" data is stored in Version, and we must make sure
that our calculated index really is correct in all cases...
*/

Version::SavedKeyIndex SetStability::keyword_index(bool get_local) const {
	if(get_local) {
		return Version::SAVEKEY_ACCEPT;
	}
	if(m_always_accept_keywords) {
		return Version::SAVEKEY_ACCEPT;
	}
	return Version::SAVEKEY_ARCH;
}

Version::SavedMaskIndex SetStability::mask_index(bool get_local) const {
	if(m_filemask_is_profile) {
		return Version::SAVEMASK_FILE;
	}
	if(get_local) {
		return Version::SAVEMASK_USERPROFILE;
	}
	return Version::SAVEMASK_PROFILE;
}

#endif

void SetStability::set_stability(bool get_local, Package *package) const {
	if(get_local) {
		portagesettings->user_config->setMasks(package, m_filemask_is_profile);
		portagesettings->user_config->setKeyflags(package);
	} else {
		portagesettings->setMasks(package, m_filemask_is_profile);
		portagesettings->setKeyflags(package, m_always_accept_keywords);
	}
}

void SetStability::calc_version_flags(bool get_local, MaskFlags *maskflags, KeywordsFlags *keyflags, const Version *v, Package *p) const {
#ifndef ALWAYS_RECALCULATE_STABILITY
	// Can we avoid the calculation by getting the saved flags?
	Version::SavedMaskIndex mi(mask_index(get_local));
	Version::SavedKeyIndex ki(keyword_index(get_local));
	if(likely(v->have_saved_masks[mi] && v->have_saved_keywords[ki])) {
		if(maskflags != NULLPTR) {
			*maskflags = v->saved_masks[mi];
		}
		if(keyflags != NULLPTR) {
			*keyflags = v->saved_keywords[ki];
		}
		return;
	}
	// No, the flags are not saved yet, we must calculate them:
#endif
	PackageSave saved(p);
	set_stability(get_local, p);
	if(maskflags != NULLPTR) {
		*maskflags = v->maskflags;
	}
	if(keyflags != NULLPTR) {
		*keyflags = v->keyflags;
	}
	saved.restore(p);
#ifndef ALWAYS_RECALCULATE_STABILITY
#ifndef NDEBUG
#ifdef EIX_PARANOIC_ASSERT
	/*
	The next test should actually be unnecessary.
	But in the above calculation of keyword_index or mask_index
	there might easily be a forgotten case (in particular, since
	it might have been forgotten to update these functions when
	another change was made).
	So we test at least at run-time that for the current version
	the correct index with the correct result was set.
	*/
	if(!(v->have_saved_masks[mi]) || !(v->have_saved_keywords[ki]) ||
		((maskflags != NULLPTR) && (v->saved_masks[mi]) != *maskflags) ||
		((keyflags != NULLPTR) && ((v->saved_keywords[ki]) != *keyflags))) {
		eix::say_error(_("internal error: SetStability calculates wrong index"));
		exit(EXIT_FAILURE);
	}
#endif
#endif
#endif
}

#if 0
void SetStability::set_stability(Category *category) const {
	for(Category::iterator it(category->begin());
		likely(it != category->end()); ++it) {
		set_stability(*it);
	}
}

void SetStability::set_stability(PackageTree *tree) const {
	for(PackageTree::iterator it(tree->begin());
		likely(it != tree->end()); ++it) {
		set_stability(it->second);
	}
}
#endif
