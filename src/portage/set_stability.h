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

#ifndef __SETSTABILITY_H__
#define __SETSTABILITY_H__

#include <portage/conf/portagesettings.h>
#include <portage/conf/cascadingprofile.h>
#include <portage/packagetree.h>

class SetStability {
	private:
		const PortageSettings *portagesettings;
		bool m_local, m_filemask_is_profile;

	public:

		SetStability(const PortageSettings *psettings, bool localsettings, bool filemask_is_profile = false)
		{
			portagesettings = psettings;
			m_local = localsettings;
			m_filemask_is_profile = filemask_is_profile;
		}

		SetStability(const SetStability &ori, bool localsettings)
		{
			portagesettings = ori.portagesettings;
			m_local = localsettings;
			m_filemask_is_profile = ori.m_filemask_is_profile;
		}

		Version::SavedKeyIndex orikey_index() const
		{
			if(m_local)
				return Version::SAVEKEY_ACCEPT;
			// This is currently the same, because always use_accapted_keywords in this class
			return Version::SAVEKEY_ACCEPT;
		}

		Version::SavedMaskIndex orimask_index() const
		{
			if(m_filemask_is_profile)
				return Version::SAVEMASK_FILE;
			if(m_local)
				return Version::SAVEMASK_USERPROFILE;
			return Version::SAVEMASK_PROFILE;
		}

		void set_stability(Package &package) const
		{
			if(m_local) {
				portagesettings->user_config->setMasks(&package, m_filemask_is_profile);
				portagesettings->user_config->setKeyflags(&package);
			}
			else {
				portagesettings->setMasks(&package, m_filemask_is_profile);
				portagesettings->setKeyflags(&package, true);
			}
		}

		void set_stability(Category &category) const
		{
			for(Category::iterator it = category.begin();
				it != category.end(); ++it)
				set_stability(**it);
		}

		void set_stability(PackageTree &tree) const
		{
			for(PackageTree::iterator it = tree.begin();
				it != tree.end(); ++it)
				set_stability(**it);
		}
};

#endif /* __SETSTABILITY_H_ */
