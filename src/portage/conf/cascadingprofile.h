// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_PORTAGE_CONF_CASCADINGPROFILE_H_
#define SRC_PORTAGE_CONF_CASCADINGPROFILE_H_ 1

#include <set>
#include <string>
#include <vector>

#include "eixTk/null.h"
#include "portage/mask.h"
#include "portage/mask_list.h"
#include "portage/overlay.h"

class Package;
class PortageSettings;
class ProfileFilenames;

class ProfileFile {
		std::string filename;
	public:
		std::vector<OverlayIdent>::size_type reponum;

		ProfileFile(const std::string &s, std::vector<OverlayIdent>::size_type repo_num)
			: filename(s), reponum(repo_num) {
		}

		const std::string &name() const {
			return filename;
		}

		const char *c_str() const {
			return filename.c_str();
		}
};

/** Access to the cascading profile pointed to by /etc/make.profile. */
class CascadingProfile {
		friend class ProfileFilenames;
	public:
		bool print_profile_paths;
		std::string profile_paths_append;
		bool use_world, finalized;
		MaskList<Mask> m_world;          /**< Packages in world. This must be set externally */

	protected:
		bool m_init_world;
		std::vector<ProfileFile> m_profile_files; /**< List of files in profile. */
		PortageSettings *m_portagesettings; /**< Profilesettings to which this instance "belongs" */

		MaskList<Mask> m_system;         /**< Packages in system profile. */
		MaskList<Mask> m_system_allowed; /**< Packages that are not in system profile but only allowed to have specific versions.*/
		MaskList<Mask> m_package_masks;  /**< Masks from package.mask */
		MaskList<Mask> m_package_unmasks;/**< Masks from package.unmask */
		MaskList<PKeywordMask> m_package_keywords;/**< Masks from package.keywords */
		MaskList<KeywordMask> m_package_accept_keywords;/**< Masks from package.accept_keywords */
		PreList p_system, p_system_allowed, p_package_masks, p_package_unmasks,
			p_package_keywords, p_package_accept_keywords;

	private:
		/** Add all files from profile and its parents to m_profile_files. */
		bool addProfile(const char *profile, std::set<std::string> *sourced_files = NULLPTR) ATTRIBUTE_NONNULL((2));

		/** Handler functions follow for reading a file */
		typedef bool (CascadingProfile::*Handler)(const std::string &filename, const char *repo);

		/** Read all "packages" files found in profile.
		 * Populate p_system and p_system_allowed.
		 * @return true if data was changed */
		bool readPackages(const std::string &filename, const char *repo);

		/** Read all "package.mask" files found in profile.
		 * Populate p_package_masks.
		 * @return true if data was changed */
		bool readPackageMasks(const std::string &filename, const char *repo);

		/** Read all "package.unmask" files found in profile.
		 * Populate p_package_unmasks.
		 * @return true if data was changed */
		bool readPackageUnmasks(const std::string &filename, const char *repo);

		/** Read all "package.keywords" files found in profile.
		 * Populate p_package_keywords.
		 * @return true if data was changed */
		bool readPackageKeywords(const std::string &filename, const char *repo);

		/** Read all "package.accept_keywords" files found in profile.
		 * Populate p_package_accept_keywords.
		 * @return true if data was changed */
		bool readPackageAcceptKeywords(const std::string &filename, const char *repo);

	public:
		CascadingProfile(PortageSettings *portagesettings, bool init_world) ATTRIBUTE_NONNULL_ :
			print_profile_paths(false),
			use_world(false), finalized(false),
			m_init_world(init_world),
			m_portagesettings(portagesettings) {
		}

		/** Populate MaskLists from PreLists.
		    All files must have been read and m_raised_arch
		    must be known when this is called. */
		void finalize();

		/** Read all "make.defaults" files previously added by listadd... */
		void readMakeDefaults();

		/** Read all mask/system files previously added by listadd...
		 * and clear this list of files afterwards.
		 * @return true if at least one file changed data. */
		bool readremoveFiles();

		/** Cycle through profile and put path to files into
		 * m_profile_files. */
		void listaddProfile(const char *profile_dir = NULLPTR);

		/** Put file into m_profile_files */
		void listaddFile(const std::string &file, std::vector<OverlayIdent>::size_type i) {
			m_profile_files.push_back(ProfileFile(file, i));
		}

		/** Clear m_profile_files */
		void listclear() {
			m_profile_files.clear();
		}

		void applyMasks(Package *p) const ATTRIBUTE_NONNULL_;
		void applyKeywords(Package *p) const ATTRIBUTE_NONNULL_;

		static void init_static();
};


#endif  // SRC_PORTAGE_CONF_CASCADINGPROFILE_H_
