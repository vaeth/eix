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
#include <portage/conf/cascadingprofile.h>

#include <portage/keywords.h>

#include <map>
#include <string>
#include <vector>

/* Files for categories the user defined and categories from the official tree */
#define USER_CATEGORIES_FILE    "/etc/portage/categories"
#define PORTDIR_CATEGORIES_FILE "profiles/categories"

class Mask;
class Package;

/** Grab Masks from file and add to a category->vector<Mask*> mapping or to a vector<Mask*>. */
bool grab_masks(const char *file, Mask::Type type, MaskList<Mask> *cat_map, std::vector<Mask*> *mask_vec, bool recursive=true);

/** Grab Mask from file and add to category->vector<Mask*>. */
inline bool grab_masks(const char *file, Mask::Type type, std::vector<Mask*> *mask_vec, bool recursive=true) {
	return grab_masks(file, type, NULL , mask_vec, recursive);
}

/** Grab Mask from file and add to vector<Mask*>. */
inline bool grab_masks(const char *file, Mask::Type type, MaskList<Mask> *cat_map, bool recursive=true) {
	return grab_masks(file, type, cat_map, NULL, recursive);
}

class PortageSettings;

class PortageUserConfig {
	private:
		PortageSettings      *m_settings;
		MaskList<Mask>        m_mask;
		MaskList<KeywordMask> m_keywords;

		bool readKeywords();
		bool readMasks() {
			bool mask_ok = grab_masks("/etc/portage/package.mask", Mask::maskMask, &m_mask);
			bool unmask_ok = grab_masks("/etc/portage/package.unmask", Mask::maskUnmask, &m_mask);
			return mask_ok && unmask_ok;
		}

	public:
		PortageUserConfig(PortageSettings *psettings) {
			m_settings = psettings;
			readKeywords();
			readMasks();
		}

		void setMasks(Package *p);
		void setStability(Package *p, Keywords kw);
};

class PortageUserConfig;

/** Holds Portage's settings, e.g. masks, categories, overlay paths.
 * Reads needed files if content is requested .. so don't worry about initialization :) */
class PortageSettings : public std::map<std::string,std::string> {

	private:
		std::vector<std::string> m_categories; /**< Vector of all allowed categories. */
		std::vector<std::string> m_accepted_keyword;

		/** Mapping of category->masks (first all masks, then all unmasks) */
		MaskList<Mask> m_masks;
		Keywords m_accepted_keywords;

	public:
		/** Your cascading profile. */
		CascadingProfile  *profile;
		PortageUserConfig *user_config;

		std::vector<std::string> overlays; /**< Location of the portage overlays */

		/** Read make.globals and make.conf. */
		PortageSettings();

		/** Free memory. */
		~PortageSettings();

		std::vector<std::string> &getAcceptKeyword() {
			return m_accepted_keyword;
		}

		Keywords getAcceptKeywords() {
			return m_accepted_keywords;
		}

		/** Read maskings & unmaskings from the profile as well as user-defined ones */
		MaskList<Mask> *getMasks();

		/** Return vector of all possible all categories.
		 * Reads categories on first call. */
		std::vector<std::string> *getCategories();

		void setStability(Package *pkg, Keywords &kw);
};

#endif
