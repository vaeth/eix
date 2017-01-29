// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_SET_STABILITY_H_
#define SRC_PORTAGE_SET_STABILITY_H_ 1

#include "portage/version.h"

class Category;
class KeywordFlags;
class MaskFlags;
class Package;
class PackageTree;
class PortageSettings;

/*
Define this if some day the complexity in the calculation of
SetStability::keyword_index or SetStability::mask_index becomes so large
that it is not worth doing it in order to avoid some re-calculations
*/
// #define ALWAYS_RECALCULATE_STABILITY 1

class SetStability {
	private:
		const PortageSettings *portagesettings;
		bool m_local, m_filemask_is_profile, m_always_accept_keywords;

#ifndef ALWAYS_RECALCULATE_STABILITY
		/*
		Calculating the index manually makes it sometimes unnecessary
		to recalculate the stability setting of the whole package.
		Of course, this is clumsy, because we must take care about how
		the "saved" data is stored in Version, and we must make sure
		that our calculated index really is correct in all cases...
		*/

		ATTRIBUTE_PURE Version::SavedKeyIndex keyword_index(bool get_local) const;
		ATTRIBUTE_PURE Version::SavedKeyIndex keyword_index() const {
			return keyword_index(m_local);
		}

		ATTRIBUTE_PURE Version::SavedMaskIndex mask_index(bool get_local) const;
		ATTRIBUTE_PURE Version::SavedMaskIndex mask_index() const {
		return mask_index(m_local);
		}
#endif

	public:
		ATTRIBUTE_NONNULL_ SetStability(const PortageSettings *psettings, bool localsettings, bool filemask_is_profile, bool always_accept_keywords) {
			portagesettings = psettings;
			m_local = localsettings;
			m_filemask_is_profile = filemask_is_profile;
			m_always_accept_keywords = always_accept_keywords;
		}

		ATTRIBUTE_NONNULL_ void set_stability(bool get_local, Package *package) const;

		ATTRIBUTE_NONNULL_ void set_stability(Package *package) const {
			set_stability(m_local, package);
		}

		ATTRIBUTE_NONNULL((5, 6)) void calc_version_flags(bool get_local, MaskFlags *maskflags, KeywordsFlags *keyflags, const Version *v, Package *p) const;

#if 0
		ATTRIBUTE_NONNULL_ void set_stability(Category *category) const;

		ATTRIBUTE_NONNULL_ void set_stability(PackageTree *tree) const;
#endif
};

#endif  // SRC_PORTAGE_SET_STABILITY_H_
