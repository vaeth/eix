// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_VARDBPKG_H_
#define SRC_PORTAGE_VARDBPKG_H_ 1

#include <map>
#include <string>
#include <vector>

#include "eixTk/eixint.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "portage/basicversion.h"
#include "portage/instversion.h"
#include "portage/package.h"

class PrintFormat;
class DBHeader;

/** Holds every installed version of a package. */
class VarDbPkg {
	private:
		/** Mapping of [category][package] to list versions. */
		std::map<std::string, std::map<std::string, std::vector<InstVersion> >* > installed;
		std::string m_directory; /**< This is the db-directory. */
		bool get_slots, care_of_slots;
		bool get_restrictions, care_of_restrictions, use_build_time;

		/** Find installed versions of packet "name" in category "category".
		 * @return NULLPTR if not found .. else pointer to vector of versions. */
		std::vector<InstVersion> *getInstalledVector(const std::string &category, const std::string &name);

		/** Read category from db-directory.
		 * @param category read this category. */
		void readCategory(const char *category) ATTRIBUTE_NONNULL_;

	public:
		/** Default constructor. */
		VarDbPkg(std::string directory, bool read_slots, bool care_about_slots,
			bool calc_restrictions, bool care_about_restrictions, bool build_time) :
			m_directory(directory),
			get_slots(read_slots || care_about_slots),
			care_of_slots(care_about_slots),
			get_restrictions(calc_restrictions),
			care_of_restrictions(care_about_restrictions),
			use_build_time(build_time)
		{ }

		~VarDbPkg() {
			for(std::map<std::string, std::map<std::string, std::vector<InstVersion> >* >::iterator it(installed.begin());
				likely(it != installed.end()); ++it) {
				delete it->second;
			}
		}

		bool care_slots() const
		{ return care_of_slots; }

		bool readSlot(const Package &p, InstVersion *v) const ATTRIBUTE_NONNULL_;
		bool readUse(const Package &p, InstVersion *v) const ATTRIBUTE_NONNULL_;
		bool readRestricted(const Package &p, InstVersion *v, const DBHeader& header) const ATTRIBUTE_NONNULL_;
		void readInstDate(const Package &p, InstVersion *v) const ATTRIBUTE_NONNULL_;

		bool readOverlay(const Package &p, InstVersion *v, const DBHeader &header) const ATTRIBUTE_NONNULL_;
		std::string readOverlayLabel(const Package *p, const BasicVersion *v) const ATTRIBUTE_NONNULL_;
		eix::SignedBool check_installed_overlays;

		/** Find installed versions
		 * @return NULLPTR if not found .. else pointer to vector of versions. */
		std::vector<InstVersion> *getInstalledVector(const Package &p) {
			return getInstalledVector(p.category, p.name);
		}

		/** Returns true if v is in vec. v=NULLPTR is always in vec.
		    If a serious result is found and r is nonzero, r points to that result */
		static bool isInVec(std::vector<InstVersion> *vec, const BasicVersion *v = NULLPTR, InstVersion **r = NULLPTR);

		/** Returns true if a Package installed.
		 * @param p Check for this Package.
		 * @param v If not NULLPTR, check for this BasicVersion.
		   If a particular version is found and r is nonzero, r points to that version */
		bool isInstalled(const Package &p, const BasicVersion *v = NULLPTR, InstVersion **r = NULLPTR)
		{ return isInVec(getInstalledVector(p), v, r); }

		/** Test if a particular version is installed from the correct overlay.
		 * @return 1 (yes) or 0 (no) or -1 (might be - overlay unclear) */
		eix::SignedBool isInstalledVersion(const Package &p, const Version *v, const DBHeader& header) ATTRIBUTE_NONNULL_;

		/** Returns number of installed versions of this package
		 * @param p Check for this Package. */
		std::vector<InstVersion>::size_type numInstalled(const Package &p);
};

#endif  // SRC_PORTAGE_VARDBPKG_H_
