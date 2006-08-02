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

#include "package.h"

#include <portage/version.h>

Package::~Package()
{
	delete_and_clear();
}


/** Check if a package has duplicated versions. */
bool
Package::checkDuplicates(Version *version) const
{
	for(const_iterator i = begin(); i != end(); ++i)
	{
		if(dynamic_cast<BasicVersion&>(**i) == dynamic_cast<BasicVersion&>(*version))
		{
			return true;
		}
	}
	return false;
}

/** Adds a version to "the versions" list. */
void Package::addVersion(Version *version)
{
	/* if the same version is in various places it should be shown.
	   possible thanks to the new [overlay] marker. */
	if(!have_duplicate_versions) {
		have_duplicate_versions = checkDuplicates(version);
	}

	Version::Overlay key = version->overlay_key;

	/* This should remain with two if .. so we can guarante that
	 * versions.size() == 0 in the else. */
	if(empty() == false) {
		if(smallest_overlay != key)
		{
			have_same_overlay_key = false;
			if(smallest_overlay > key)
				smallest_overlay = key;
		}
		if(is_system_package) {
			is_system_package = version->isSystem();
		}
	}
	else {
		smallest_overlay  = key;
		is_system_package = version->isSystem();
	}

	sortedPushBack(version);
}

void
Package::sortedPushBack(Version *v)
{
	for(iterator i = begin(); i != end(); ++i)
	{
		if(*v < **i)
		{
			insert(i, v);
			return;
		}
	}
	push_back(v);
}

Version *
Package::best() const
{
	Version *ret = NULL;
	for(const_reverse_iterator ri = rbegin();
		ri != rend();
		++ri)
	{
		if(ri->isStable() && !ri->isHardMasked())
		{
			ret = *ri;
			break;
		}
	}
	return ret;
}

void Package::deepcopy(const Package &p)
{
	*this=p;
	clear();
	for(Package::const_iterator it=p.begin(); it != p.end(); it++)
	{
		Version *v=new Version;
		*v=(**it);
		this->push_back(v);
	}
}
