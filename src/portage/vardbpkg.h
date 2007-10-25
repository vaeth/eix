// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PORTAGECONF_H__
#define __PORTAGECONF_H__

#include <vector>
#include <map>
#include <cstdlib>

#include <eixTk/exceptions.h>
#include <portage/package.h>
#include <portage/instversion.h>

class PrintFormat;
class DBHeader;

/** Holds every installed version of a package. */
class VarDbPkg {
	private:
		/** Mapping of [category][package] to list versions. */
		std::map<std::string, std::map<std::string, std::vector<InstVersion> >* > installed;
		std::string _directory; /**< This is the db-directory. */
		bool get_slots, care_of_slots;
		bool get_restrictions, care_of_restrictions;

		/** Find installed versions of packet "name" in category "category".
		 * @return NULL if not found .. else pointer to vector of versions. */
		std::vector<InstVersion> *getInstalledVector(const std::string &category, const std::string &name);

		/** Read category from db-directory.
		 * @param category read this category. */
		void readCategory(const char *category);

	public:
		/** Default constructor. */
		VarDbPkg(std::string directory, bool read_slots, bool care_about_slots,
			bool calc_restrictions, bool care_about_restrictions) :
			_directory(directory),
			get_slots(read_slots || care_about_slots),
			care_of_slots(care_about_slots),
			get_restrictions(calc_restrictions),
			care_of_restrictions(care_about_restrictions)
		{ }

		~VarDbPkg() {
			std::map<std::string, std::map<std::string, std::vector<InstVersion> >* >::iterator it = installed.begin();
			while(it != installed.end()) {
				delete it++->second;
			}
		}

		bool care_slots() const
		{ return care_of_slots; }

		bool readSlot(const Package &p, InstVersion &v) const;
		bool readUse(const Package &p, InstVersion &v) const;
		bool readRestricted(const Package &p, InstVersion &v, const DBHeader& header, const char *portdir) const;

		bool readOverlay(const Package &p, InstVersion &v, const DBHeader &header, const char *portdir) const;
		std::string readOverlayLabel(const Package *p, const BasicVersion *v) const;
		std::string readOverlayPath(const Package *p, const BasicVersion *v) const;
		short check_installed_overlays;

		/** Find installed versions
		 * @return NULL if not found .. else pointer to vector of versions. */
		std::vector<InstVersion> *getInstalledVector(const Package &p) {
			return getInstalledVector(p.category, p.name);
		};

		/** Returns true if v is in vec. v=NULL is always in vec.
		    If a serious result is found and r is nonzero, r points to that result */
		static bool isInVec(std::vector<InstVersion> *vec, const BasicVersion *v = NULL, InstVersion **r = NULL);

		/** Returns true if a Package installed.
		 * @param p Check for this Package.
		 * @param v If not NULL, check for this BasicVersion.
		   If a particular version is foundand r is nonzero, r points to that version */
		bool isInstalled(const Package &p, const BasicVersion *v = NULL, InstVersion **r = NULL)
		{ return isInVec(getInstalledVector(p), v, r); }

		/** Returns number of installed versions of this package
		 * @param p Check for this Package. */
		std::vector<InstVersion>::size_type numInstalled(const Package &p);
};

#endif /* __PORTAGECONF_H__ */
