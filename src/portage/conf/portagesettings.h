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
#include <portage/mask.h>

#include <map>
#include <string>
#include <vector>

/* Files for categories the user defined and categories from the official tree */
#define USER_CATEGORIES_FILE    "/etc/portage/categories"
#define PORTDIR_CATEGORIES_FILE "profiles/categories"

/** Grab Masks from file and add to a category->vector<Mask*> mapping or to a vector<Mask*>. */
bool grab_masks(const char *file, Mask::Type type, MaskList *cat_map, vector<Mask*> *mask_vec);

/** Grab Mask from file and add to category->vector<Mask*>. */
inline bool grab_masks(const char *file, Mask::Type type, vector<Mask*> *mask_vec) {
	return grab_masks(file, type, NULL , mask_vec);
}

/** Grab Mask from file and add to vector<Mask*>. */
inline bool grab_masks(const char *file, Mask::Type type, MaskList *cat_map) {
	return grab_masks(file, type, cat_map, NULL);
}

class PortageSettings;

class PortageUserConfig {
	private:
		PortageSettings *_psettings;
		MaskList         _mask;
		MaskList         _keywords;

		bool readKeywords();
		bool readMasks() {
			bool ok = grab_masks("/etc/portage/package.unmask", Mask::maskUnmask, &_mask);
			return    grab_masks("/etc/portage/package.mask", Mask::maskMask, &_mask) && ok;
		}

	public:
		PortageUserConfig(PortageSettings *psettings) {
			_psettings = psettings;
			readKeywords();
			readMasks();
		}

		void setMasks(Package *p);
		void setStability(Package *p, Keywords kw);
};


class PortageUserConfig;

/** Holds Portage's settings, e.g. masks, categories, overlay paths.
 * Reads needed files if content is requested .. so don't worry about initialization :) */
class PortageSettings : public map<string,string> {
	private:
		vector<string> _categories; /**< Vector of all allowed categories. */
		vector<string> _accepted_keyword;

		/** Mapping of category->masks (first all masks, then all unmasks) */
		MaskList _masks;  
		Keywords _accepted_keywords;

	public:
		/** Your cascading profile. */
		CascadingProfile  *profile;
		PortageUserConfig *user_config;

		vector<string> overlays; /**< Location of the portage overlays */

		/** Read make.globals and make.conf. */
		PortageSettings();

		/** Free memory. */
		~PortageSettings();

		vector<string> &getAcceptKeyword() {
			return _accepted_keyword;
		}
		
		Keywords getAcceptKeywords() {
			return _accepted_keywords;
		}
		
		/** Read maskings & unmaskings from the profile as well as user-defined ones */
		MaskList *getMasks();

		/** Return vector of all possible all categories.
		 * Reads categories on first call. */
		vector<string> *getCategories();

		void setStability(Package *pkg, Keywords &kw) {
			Package::iterator t = pkg->begin();
			for(; t != pkg->end(); ++t) {
				if((*t)->get() & kw.get()) {
					**t |= Keywords::KEY_STABLE;
				}
				else {
					**t &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
				}
			}
		}
};

#endif
