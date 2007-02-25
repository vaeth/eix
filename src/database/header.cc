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

#include "header.h"

#include <database/io.h>

#include <eixTk/exceptions.h>
#include <eixTk/stringutils.h>
#include <eixTk/filenames.h>

using namespace std;

/** Get string for key from directory-table. */
string
DBHeader::getOverlay(Version::Overlay key) const
{
	if(key > countOverlays())
		return string("");
	return overlays[key];
}

/** Add overlay to directory-table and return key. */
Version::Overlay
DBHeader::addOverlay(string overlay)
{
	overlays.push_back(overlay);
	return countOverlays() - 1;
}

bool DBHeader::find_overlay(Version::Overlay *num, const char *name, const char *portdir, Version::Overlay minimal, bool test_saved_portdir) const
{
	if(minimal > countOverlays())
		return false;
	if(*name == '\0') {
		if(countOverlays() == 1)
			return false;
		*num = (minimal != 0) ? minimal : 1;
		return true;
	}
	if(minimal == 0) {
		if(portdir) {
			if(same_filenames(name, portdir, true)) {
				*num = 0;
				return true;
			}
		}
	}
	for(Version::Overlay i = minimal; i != countOverlays(); i++)
	{
		if(same_filenames(name, getOverlay(i).c_str(), true)) {
			if((!test_saved_portdir) && (i == 0))
				continue;
			*num = i;
			return true;
		}
	}
	// Is name a number?
	Version::Overlay number;
	const char *s = name;
	for( ; ((*s) >= '0') && ((*s) <= '9') ; s++);
	if(*s)
		return false;
	try {
		number = atoi(name);
		if(number >= countOverlays())
			return false;
		if(number < minimal)
			return false;
	}
	catch(ExBasic e) {
		return false;
	}
	*num = number;
	return true;
}

void
DBHeader::get_overlay_vector(set<Version::Overlay> *overlayset, const char *name, const char *portdir, Version::Overlay minimal, bool test_saved_portdir) const
{
	Version::Overlay curr;
	for(curr = minimal; find_overlay(&curr, name, portdir, curr, test_saved_portdir); curr++)
		overlayset->insert(curr);
}
