// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__CASCADINGPROFILE_H__
#define EIX__CASCADINGPROFILE_H__ 1

#include <eixTk/exceptions.h>
#include <portage/mask_list.h>
#include <portage/package.h>

class PortageSettings;
class Package;

/** Access to the cascading profile pointed to by /etc/make.profile. */
class CascadingProfile {

	public:
		bool use_world;
		MaskList<Mask> m_world;          /**< Packages in world. This must be set externally */

	protected:
		bool m_init_world;
		std::vector<std::string>      m_profile_files; /**< List of files in profile. */
		PortageSettings *m_portagesettings; /**< Profilesettings to which this instance "belongs" */

		MaskList<Mask> m_system;         /**< Packages in system profile. */
		MaskList<Mask> m_system_allowed; /**< Packages that are not in system profile but only allowed to have specific versions.*/
		MaskList<Mask> m_package_masks;  /**< Masks from package.mask */
		MaskList<Mask> m_package_unmasks;/**< Masks from package.unmask */
		MaskList<PKeywordMask> m_package_keywords;/**< Masks from package.keywords */

	private:

		/** Add all files from profile ans its parents to m_profile_files. */
		void addProfile(const char *profile, unsigned int depth = 0);

		/** Read all "packages" files found in profile.
		 * Populate m_system and m_system_allowed.
		 * @return true if data was changed */
		bool readPackages(const std::string &line);

		/** Read all "package.mask" files found in profile.
		 * Populate m_package_masks.
		 * @return true if data was changed */
		bool readPackageMasks(const std::string &line);

		/** Read all "package.unmask" files found in profile.
		 * Populate m_package_unmasks.
		 * @return true if data was changed */
		bool readPackageUnmasks(const std::string &line);

		/** Read all "package.keywords" files found in profile.
		 * Populate m_package_keywords.
		 * @return true if data was changed */
		bool readPackageKeywords(const std::string &line);
	public:
		CascadingProfile(PortageSettings *portagesettings, bool init_world)
		{
			m_portagesettings = portagesettings;
			use_world = false;
			m_init_world = init_world;
		}

		/** Read all "make.defaults" files previously added by listadd... */
		void readMakeDefaults();

		/** Read all mask/system files previously added by listadd...
		 * and clear this list of files afterwards.
		 * @return true if at least one file changed data. */
		bool readremoveFiles();

		/** Cycle through profile and put path to files into
		 * m_profile_files. */
		void listaddProfile(const char *profile_dir = NULL) throw(ExBasic);

		/** Put file into m_profile_files */
		void listaddFile(const char *file)
		{ m_profile_files.push_back(file); }

		/** Clear m_profile_files */
		void listclear()
		{ m_profile_files.clear(); }

		void applyMasks(Package *p) const;
		void applyKeywords(Package *p) const;

		/** Get all m_system packages. */
		const MaskList<Mask> *getSystemPackages() const {
			return &m_system;
		}

		/** Get all m_system packages. */
		const MaskList<Mask> *getWorldPackages() const {
			return &m_world;
		}

		/** Get packages that are not in m_system profile but only allowed to have specific versions .*/
		const MaskList<Mask> *getAllowedPackages() const {
			return &m_system_allowed;
		}

		const MaskList<Mask> *getPackageMasks() const {
			return &m_package_masks;
		}

		const MaskList<Mask> *getPackageUnmasks() const {
			return &m_package_unmasks;
		}

		const MaskList<PKeywordMask> *getPackageKeywords() const {
			return &m_package_keywords;
		}
};


#endif /* EIX__CASCADINGPROFILE_H__ */
