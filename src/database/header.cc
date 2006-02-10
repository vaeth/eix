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

#include "header.h"

#include <database/io.h>

#include <eixTk/exceptions.h>
#include <eixTk/stringutils.h>

/** Get string for key from directory-table. */
string
DBHeader::getOverlay(short key)
{
	if(key > (short) overlays.size())
		return string("");
	return overlays[key];
}

/** Add overlay to directory-table and return key. */
short
DBHeader::addOverlay(string overlay)
{
	overlays.push_back(overlay);
	return (short) overlays.size() - 1;
}


bool
DBHeader::write(FILE *stream)
{
	/* We can't reference static const. */
	int local_version = DBVERSION;
	io::write(stream, local_version);

	io::write_string(stream, arch);
	io::write<int>(stream, numcategories);
	io::write<int>(stream, numpackages);

	unsigned short overlay_sz = overlays.size();
	io::write<short>(stream, overlay_sz);
	for(int i = 0; i<overlay_sz; i++)
		io::write_string(stream, overlays[i]);
	return true;
}

bool
DBHeader::read(FILE *stream)
{
	version = io::read<int>(stream);
#if 0
	if(version != DBHeader::DBVERSION) {
		throw(ExBasic("Obsolete database file version %i (current version %i)\n"
					"Run 'update-eix' to update the db.",
					version, DBHeader::DBVERSION));
	}
#endif

	arch = io::read_string(stream);
	numcategories = io::read<int>(stream);
	numpackages = io::read<int>(stream);

	unsigned short overlay_sz = io::read<short>(stream);
	overlays.resize(overlay_sz);
	for(int i = 0; i<overlay_sz; i++)
		overlays[i] = io::read_string(stream);
	return true;
}
