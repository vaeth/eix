/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __PORTAGESETTINGS_H
#define __PORTAGESETTINGS_H

#include <eixTk/exceptions.h>

#include <portage/keywords.h>
#include <portage/mask_list.h>

#include <map>
#include <string>
#include <vector>

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
	private:
		PortageSettings      *m_settings;
		MaskList<Mask>        m_mask;
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
		PortageUserConfig(PortageSettings *psettings);

		~PortageUserConfig();

		void setProfileMasks(Package *p) const;

		/// @return true if something from /etc/portage/package.* applied
		bool setMasks(Package *p, Keywords::Redundant check = Keywords::RED_NOTHING) const;
		/// @return true if something from /etc/portage/package.* applied
		bool setStability(Package *p, const Keywords &kw, Keywords::Redundant check = Keywords::RED_NOTHING) const;

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
		std::vector<std::string> m_accepted_keyword;

		/** Mapping of category->masks (first all masks, then all unmasks) */
		MaskList<Mask> m_masks;
		Keywords m_accepted_keywords;

		/** Your cascading profile, excluding local settings */
		CascadingProfile  *profile;

		void override_by_env(const char **vars);
		void read_config(const std::string &name, const std::string &prefix);

	public:
		bool m_obsolete_minusasterisk;
		std::string m_eprefixconf;
		std::string m_eprefixprofile;
		std::string m_eprefixportdir;
		std::string m_eprefixoverlays;
		std::string m_eprefixaccessoverlays;

		PortageUserConfig *user_config;

		std::vector<std::string> overlays; /**< Location of the portage overlays */

		/** Read make.globals and make.conf. */
		PortageSettings(EixRc &eixrc, bool getlocal);

		/** Free memory. */
		~PortageSettings();

		std::vector<std::string> &getAcceptKeyword() {
			return m_accepted_keyword;
		}

		std::string resolve_overlay_name(const std::string &path, bool resolve);
		void add_overlay(std::string &path, bool resolve, bool modify_path = false);
		void add_overlay_vector(std::vector<std::string> &v, bool resolve, bool modify_v = false);

		static Keywords getAcceptKeywordsDefault() {
			return Keywords::KEY_STABLE;
		}

		Keywords getAcceptKeywordsLocal() const {
			return m_accepted_keywords;
		}

		/** Read maskings & unmaskings from the profile as well as user-defined ones */
		MaskList<Mask> *getMasks();

		/** Return vector of all possible all categories.
		 * Reads categories on first call. */
		std::vector<std::string> *getCategories();

		void setMasks(Package *p);

		void setStability(Package *pkg, const Keywords &kw, bool save_after_setting) const;
};

#endif
