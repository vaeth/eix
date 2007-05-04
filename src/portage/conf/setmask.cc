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

#include <portage/conf/cascadingprofile.h>
#include <portage/conf/portagesettings.h>

using namespace std;

void
CascadingProfile::applyMasks(Package *p) const
{
	for(Package::iterator it = p->begin(); it != p->end(); ++it) {
		**it &= Keywords::KEY_ALL;
	}
	getAllowedPackages()->applyMasks(p);
	getSystemPackages()->applyMasks(p);
	getPackageMasks()->applyMasks(p);
}

void PortageUserConfig::setProfileMasks(Package *p) const
{
	profile->applyMasks(p);
	m_settings->getMasks()->applyMasks(p);
}

void PortageSettings::setMasks(Package *p)
{
	profile->applyMasks(p);
	getMasks()->applyMasks(p);
}

