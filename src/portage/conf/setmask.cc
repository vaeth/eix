// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#include <portage/conf/cascadingprofile.h>
#include <portage/conf/portagesettings.h>

using namespace std;

void
CascadingProfile::applyMasks(Package *p) const
{
	for(Package::iterator it = p->begin(); it != p->end(); ++it) {
		it->maskflags.set(MaskFlags::MASK_NONE);
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

