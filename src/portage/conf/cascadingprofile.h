// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_CONF_CASCADINGPROFILE_H_
#define SRC_PORTAGE_CONF_CASCADINGPROFILE_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "portage/mask.h"
#include "portage/mask_list.h"
#include "portage/overlay.h"

class Package;
class PortageSettings;
class ProfileFilenames;

class ProfileFile {
	private:
		std::string filename;
		OverlayVec::size_type reponum;
		bool honour_repo;

	public:
		ProfileFile(const std::string& s, OverlayVec::size_type repo_num, bool only_repo)
			: filename(s), reponum(repo_num), honour_repo(only_repo) {
		}

		const std::string& name() const {
			return filename;
		}

		const char *c_str() const {
			return filename.c_str();
		}

		OverlayVec::size_type repo_num() const {
			return reponum;
		}

		bool only_repo() const {
			return honour_repo;
		}
};

/**
Access to the cascading profile pointed to by /etc/make.profile
**/
class CascadingProfile {
		friend class ProfileFilenames;
	public:
		bool print_profile_paths;
		std::string profile_paths_append;
		bool use_world, finalized;
		MaskList<Mask> m_world;            ///< Packages in world. This must be set externally

	protected:
		bool m_init_world;
		typedef std::vector<ProfileFile> ProfileFiles;
		ProfileFiles m_profile_files;      ///< List of files in profile.
		PortageSettings *m_portagesettings;  ///< Profilesettings to which this instance "belongs"

		MaskList<Mask> m_system;           ///< Packages in @system
		MaskList<Mask> m_profile;          ///< Packages in @profile
		MaskList<Mask> m_package_masks;    ///< Masks from package.mask
		MaskList<Mask> m_package_unmasks;  ///< Masks from package.unmask
		MaskList<PKeywordMask> m_package_keywords;  ///< Masks from package.keywords
		MaskList<KeywordMask> m_package_accept_keywords;  ///< Masks from package.accept_keywords
		PreList p_system, p_profile, p_package_masks, p_package_unmasks,
			p_package_keywords, p_package_accept_keywords;

	private:
		/**
		Add all files from profile and its parents to m_profile_files
		**/
		ATTRIBUTE_NONNULL((2)) bool addProfile(const char *profile, WordUnorderedSet *sourced_files);
		ATTRIBUTE_NONNULL_ bool addProfile(const char *profile) {
			return addProfile(profile, NULLPTR);
		}

		/**
		Handler functions follow for reading a file
		**/
		typedef bool (CascadingProfile::*Handler)(const std::string& filename, const char *repo, bool only_repo);

		/**
		Read all "packages" files found in profile.
		Populate p_system and p_profile.
		@return true if data was changed
		**/
		bool readPackages(const std::string& filename, const char *repo, bool only_repo);

		/**
		Read all "package.mask" files found in profile.
		Populate p_package_masks.
		@return true if data was changed
		**/
		bool readPackageMasks(const std::string& filename, const char *repo, bool only_repo);

		/**
		Read all "package.unmask" files found in profile.
		Populate p_package_unmasks.
		@return true if data was changed
		**/
		bool readPackageUnmasks(const std::string& filename, const char *repo, bool only_repo);

		/**
		Read all "package.keywords" files found in profile.
		Populate p_package_keywords.
		@return true if data was changed
		**/
		bool readPackageKeywords(const std::string& filename, const char *repo, bool only_repo);

		/**
		Read all "package.accept_keywords" files found in profile.
		Populate p_package_accept_keywords.
		@return true if data was changed
		**/
		bool readPackageAcceptKeywords(const std::string& filename, const char *repo, bool only_repo);

	public:
		ATTRIBUTE_NONNULL_ CascadingProfile(PortageSettings *portagesettings, bool init_world) :
			print_profile_paths(false),
			use_world(false), finalized(false),
			m_init_world(init_world),
			m_portagesettings(portagesettings) {
		}

		/**
		Populate MaskLists from PreLists.
		All files must have been read and m_raised_arch
		must be known when this is called.
		**/
		void finalize();

		/**
		Read all "make.defaults" files previously added by listadd...
		**/
		void readMakeDefaults();

		/**
		Read all mask/system files previously added by listadd...
		and clear this list of files afterwards.
		@return true if at least one file changed data.
		**/
		bool readremoveFiles();

		/**
		Cycle through profile and put path to files into m_profile_files
		**/
		void listaddProfile(const char *profile_dir = NULLPTR);

		/**
		Put file into m_profile_files
		**/
		void listaddFile(const std::string& file, OverlayVec::size_type i, bool only_repo) {
			m_profile_files.EMPLACE_BACK(ProfileFile, (file, i, only_repo));
		}

		/**
		Clear m_profile_files
		**/
		void listclear() {
			m_profile_files.clear();
		}

		ATTRIBUTE_NONNULL_ void applyMasks(Package *p) const;
		ATTRIBUTE_NONNULL_ void applyKeywords(Package *p) const;

		static void init_static();
};


#endif  // SRC_PORTAGE_CONF_CASCADINGPROFILE_H_
