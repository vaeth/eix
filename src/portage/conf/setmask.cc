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
		(*it)->maskflags.set(MaskFlags::MASK_NONE);
	}
	getAllowedPackages()->applyMasks(p);
	getSystemPackages()->applyMasks(p);
	getPackageMasks()->applyMasks(p);
}

void
PortageUserConfig::setProfileMasks(Package *p) const
{
	if(p->restore_maskflags(Version::SAVEMASK_USERPROFILE))
		return;
	if(profile)
		profile->applyMasks(p);
	else
		m_settings->setMasks(p);
	p->save_maskflags(Version::SAVEMASK_USERPROFILE);
}

/// @return true if something from /etc/portage/package.* applied and check involves masks
bool
PortageUserConfig::setMasks(Package *p, Keywords::Redundant check, bool file_mask_is_profile) const
{
	Version::SavedMaskIndex ind = file_mask_is_profile ?
		Version::SAVEMASK_USERFILE : Version::SAVEMASK_USER;
	if((check & Keywords::RED_ALL_KEYWORDS) == Keywords::RED_NOTHING)
	{
		if(p->restore_maskflags(ind))
			return false;
	}
	if(file_mask_is_profile) {
		if(!(p->restore_maskflags(Version::SAVEMASK_FILE))) {
			throw ExBasic("internal error: Tried to restore nonlocal mask without saving");
		}
	}
	else
		setProfileMasks(p);
	bool rvalue = m_localmasks.applyMasks(p, check);
	p->save_maskflags(ind);
	return rvalue;
}

void
PortageSettings::setMasks(Package *p, bool filemask_is_profile) const
{
	if(filemask_is_profile) {
		if(!(p->restore_maskflags(Version::SAVEMASK_FILE))) {
			throw ExBasic("internal error: Tried to restore nonlocal mask without saving");
		}
		return;
	}
	if(p->restore_maskflags(Version::SAVEMASK_PROFILE))
		return;
	profile->applyMasks(p);
	p->save_maskflags(Version::SAVEMASK_PROFILE);
}

