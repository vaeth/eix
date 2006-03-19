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

#ifndef __CASCADINGPROFILE_H__
#define __CASCADINGPROFILE_H__

#include <eixTk/exceptions.h>
#include <portage/mask.h>

#include <map>
#include <string>
#include <vector>

/** Access to the cascading profile pointed to by /etc/make.profile. */
class CascadingProfile {

	public:

	protected:
		std::vector<std::string>      m_profile_files; /**< List of files in profile. */
		std::map<std::string,std::string> *m_make_defaults; /**< Map of variables found in make.defaults-files. */

		MaskList m_system;         /**< Packages in m_system profile. */
		MaskList m_system_allowed; /**< Packages that are not in m_system profile but only allowed to have specific versions.*/
		MaskList m_package_masks;  /**< Masks from package.masks */

	private:

		/** Cycle through profile.
		 * Look for parent profile of the profile pointed to by path_buffer Write the path for the new
		 * profile into path_buffer and return true Return false if there is no parent profile.
		 * @param path_buffer Path to the profile (new profile is written into this thing)
		 * @return false if there is no parent profile */
		bool getParentProfile(std::string &path_buffer);

		/** Cycle through profile and put path to files into
		 * profile_files. */
		void listProfile(void) throw(ExBasic);

		void readFiles();

		/** Read all "packages" files found in profile.
		 * Populate m_system and m_system_allowed. */
		void readPackages(const std::string &line);
		void readPackageMasks(const std::string &line);

		/** Read all "make.defaults" files found in profile.
		 * Use make_defaults as map for parser. */
		void readMakeDefaults();

	public:
		CascadingProfile(std::map<std::string,std::string> *mapping) {
			m_make_defaults = mapping;

			listProfile();
			readMakeDefaults();
			readFiles();
		}

		/** Get all m_system packages. */
		MaskList *getSystemPackages() {
			return &m_system;
		}

		/** Get packages that are not in m_system profile but only allowed to have specific versions .*/
		MaskList *getAllowedPackages() {
			return &m_system_allowed;
		}

		MaskList *getPackageMasks() {
			return &(m_package_masks);
		}
};


#endif /* __CASCADINGPROFILE_H__ */
