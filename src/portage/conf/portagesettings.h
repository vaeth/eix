// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PORTAGESETTINGS_H
#define __PORTAGESETTINGS_H

#include <eixTk/exceptions.h>

#include <portage/keywords.h>
#include <portage/mask_list.h>

#include <map>
#include <string>
#include <vector>
#include <cstdlib>

/* Files for categories the user defined and categories from the official tree */
#define MAKE_GLOBALS_FILE       "/etc/make.globals"
#define MAKE_CONF_FILE          "/etc/make.conf"
#define USER_CATEGORIES_FILE    "/etc/portage/categories"
#define USER_KEYWORDS_FILE      "/etc/portage/package.keywords"
#define USER_MASK_FILE          "/etc/portage/package.mask"
#define USER_UNMASK_FILE        "/etc/portage/package.unmask"
#define USER_USE_FILE           "/etc/portage/package.use"
#define USER_CFLAGS_FILE        "/etc/portage/package.cflags"
#define USER_PROFILE_DIR        "/etc/portage/profile"
#define PORTDIR_CATEGORIES_FILE "profiles/categories"
#define PORTDIR_MASK_FILE       "profiles/package.mask"

class Mask;
class Package;
class EixRc;
class CascadingProfile;

/** Grab Masks from file and add to a category->vector<Mask*> mapping or to a vector<Mask*>. */
bool grab_masks(const char *file, Mask::Type type, MaskList<Mask> *cat_map, std::vector<Mask*> *mask_vec, bool recursive = false);

/** Grab Mask from file and add to category->vector<Mask*>. */
inline bool grab_masks(const char *file, Mask::Type type, std::vector<Mask*> *mask_vec, bool recursive = false) {
	return grab_masks(file, type, NULL , mask_vec, recursive);
}

/** Grab Mask from file and add to vector<Mask*>. */
inline bool grab_masks(const char *file, Mask::Type type, MaskList<Mask> *cat_map, bool recursive = false) {
	return grab_masks(file, type, cat_map, NULL, recursive);
}

class PortageSettings;


class PortageUserConfig {
		friend class PortageSettings;
	private:
		PortageSettings      *m_settings;
		MaskList<Mask>        m_localmasks;
		MaskList<KeywordMask> m_keywords;
		MaskList<KeywordMask> m_use, m_cflags;
		bool read_use, read_cflags;

		/** Your cascading profile, including local settings */
		CascadingProfile     *profile;

		bool readKeywords();
		bool readMasks();

		static bool CheckList(Package *p, const MaskList<KeywordMask> *list, Keywords::Redundant flag_double, Keywords::Redundant flag_in);
		bool CheckFile(Package *p, const char *file, MaskList<KeywordMask> *list, bool *readfile, Keywords::Redundant flag_double, Keywords::Redundant flag_in) const;
		static void ReadVersionFile (const char *file, MaskList<KeywordMask> *list);

	public:
		PortageUserConfig(PortageSettings *psettings, CascadingProfile *local_profile);

		~PortageUserConfig();

		void setProfileMasks(Package *p) const;

		/// @return true if something from /etc/portage/package.* applied and check involves masks
		bool setMasks(Package *p, Keywords::Redundant check = Keywords::RED_NOTHING, bool file_mask_is_profile = false) const;
		/// @return true if something from /etc/portage/package.* applied and check involves keywords
		bool setKeyflags(Package *p, Keywords::Redundant check = Keywords::RED_NOTHING) const;

		/// @return true if something from /etc/portage/package.use applied
		bool CheckUse(Package *p, Keywords::Redundant check)
		{
			if(check & Keywords::RED_ALL_USE)
				return CheckFile(p, USER_USE_FILE, &m_use, &read_use, check & Keywords::RED_DOUBLE_USE, check & Keywords::RED_IN_USE);
			return false;
		}
		/// @return true if something from /etc/portage/package.cflags applied
		bool CheckCflags(Package *p, Keywords::Redundant check)
		{
			if(check & Keywords::RED_ALL_CFLAGS)
				return CheckFile(p, USER_CFLAGS_FILE, &m_cflags, &read_cflags, check & Keywords::RED_DOUBLE_CFLAGS, check & Keywords::RED_IN_CFLAGS);
			return false;
		}
};

/** Holds Portage's settings, e.g. masks, categories, overlay paths.
 * Reads needed files if content is requested .. so don't worry about initialization :) */
class PortageSettings : public std::map<std::string,std::string> {

	private:
		friend class CascadingProfile;
		friend class PortageUserConfig;

		std::vector<std::string> m_categories; /**< Vector of all allowed categories. */
		std::vector<std::string> m_accepted_keywords;
		std::set<std::string>    m_accepted_keywords_set, m_arch_set,
		                        *m_local_arch_set;

		MaskList<SetMask>        m_package_sets;

		std::vector<std::string> set_names;
		std::vector<SetsList>    parent_sets;
		std::vector<SetsList>    children_sets;

		bool know_world_sets;
		std::vector<std::string> world_sets;
		bool world_setslist_up_to_date;
		SetsList world_setslist;

		/** Your cascading profile, excluding local settings */
		CascadingProfile  *profile;

		void override_by_env(const char **vars);
		void read_config(const std::string &name, const std::string &prefix);

		void addOverlayProfiles(CascadingProfile *p) const;

		void calc_recursive_sets(Package *p) const;

		void read_world_sets(const char *file);
		void calc_world_sets(Package *p);

		void update_world_setslist();

		const MaskList<SetMask> *getPackageSets() const
		{ return &m_package_sets; }
	public:
		bool m_recurse_sets;
		bool m_obsolete_minusasterisk;
		std::string m_eprefixconf;
		std::string m_eprefixprofile;
		std::string m_eprefixportdir;
		std::string m_eprefixoverlays;
		std::string m_eprefixaccessoverlays;
		std::string m_world;

		PortageUserConfig *user_config;

		std::vector<std::string> overlays; /**< Location of the portage overlays */

		/** Read make.globals and make.conf. */
		PortageSettings(EixRc &eixrc, bool getlocal, bool init_world);

		/** Free memory. */
		~PortageSettings();

		std::string resolve_overlay_name(const std::string &path, bool resolve);
		void add_overlay(std::string &path, bool resolve, bool modify_path = false);
		void add_overlay_vector(std::vector<std::string> &v, bool resolve, bool modify_v = false);

		void store_world_sets(const std::vector<std::string> *s_world_sets, bool override = false);
		void get_setnames(std::vector<std::string> &names, const Package *p, bool also_nonlocal = false) const;
		std::string get_setnames(const Package *p, bool also_nonlocal = false) const;
		void read_local_sets(const std::string &dir_path);
		const std::vector<std::string> *get_world_sets() const
		{ return &world_sets; }

		/** Return vector of all possible all categories.
		 * Reads categories on first call. */
		std::vector<std::string> *getCategories();

		void setMasks(Package *p, bool filemask_is_profile = false) const;

		/// Set stability according to arch or local ACCEPTED_KEYWORDS
		void setKeyflags(Package *pkg, bool use_accepted_keywords) const;

		void add_name(SetsList &l, const std::string &s, bool recurse) const;

		void calc_local_sets(Package *p) const
		{
			m_package_sets.applyMasks(p);
			if(m_recurse_sets)
				calc_recursive_sets(p);
		}

		void finalize(Package *p)
		{ calc_world_sets(p); p->finalize_masks(); }
};

#endif

