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
		bool m_nonlocal, etc_profile;

	public:
		bool use_etc_profile() const
		{ return etc_profile; }

		SetStability(const PortageSettings *psettings, bool nonlocal, bool etc_profile_usage = true)
		{
			portagesettings = psettings;
			m_nonlocal = nonlocal;
			etc_profile = etc_profile_usage;
		}

		void set_stability(Package &package) const
		{
			if(m_nonlocal) {
				package.restore_nonlocal();
				return;
			}
			if(package.restore_local())
				return;
			/* Add local keywords */
			//portagesettings->setStability(&package);
			if(etc_profile)
				portagesettings->user_config->setProfileMasks(&package);
			portagesettings->user_config->setMasks(&package);
			portagesettings->user_config->setStability(&package);
			package.save_local();
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
