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
		Keywords default_accepted_keywords, local_accepted_keywords;
		bool ignore_etc_portage, etc_profile;

	public:
		SetStability(const PortageSettings *psettings, bool no_etc_portage, bool use_etc_profile = true)
		{
			portagesettings = psettings;
			default_accepted_keywords = psettings->getAcceptKeywordsDefault();
			ignore_etc_portage = no_etc_portage;
			etc_profile = use_etc_profile;
			if(no_etc_portage)
				return;
			local_accepted_keywords = psettings->getAcceptKeywordsLocal();
		}

		void set_stability(Package &package) const
		{
			portagesettings->setStability(&package, default_accepted_keywords, true);
			if(ignore_etc_portage)
				return;
			/* Add individual maskings from this machines /etc/portage/ */
			if(etc_profile)
				portagesettings->user_config->setProfileMasks(&package);
			portagesettings->user_config->setMasks(&package);
			portagesettings->user_config->setStability(&package, local_accepted_keywords);
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
