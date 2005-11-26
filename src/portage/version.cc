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

#include "version.h"

#include <database/basicio.h>

/** Constructor, calls BasicVersion::parseVersion( str ) */
Version::Version( const char* str ) : BasicVersion( str )
{
	overlay_key = 0;
}

/** Constructor, calls BasicVersion::parseVersion( str ) */
Version::Version( string str ) : BasicVersion( str.c_str() )
{
	overlay_key = 0;
}

Version::Version(FILE *stream)
{
	overlay_key = 0;
	read(stream);
}
/***********************************************************************************/

/** Read a Version instance from the eix db */
void Version::read( FILE *is )
{
	// read full string
	full = io::read_string(is);

	// read stability & masking
	_mask = io::read<Keywords::Type>(is);

#if 0
	// read primary string
	primary = io::read_string(is);
#endif

	// read primsplit
	unsigned char i = io::read<unsigned char>(is);
	while(i--)
	{
		primsplit.push_back(io::read<unsigned short>(is));
	}
	primarychar = io::read<unsigned char>(is);

	// read suffixlevel,suffixnum,gentoorelease
	suffixlevel   = io::read<unsigned char>(is);
	suffixnum     = io::read<unsigned int>(is);
	gentoorelease = io::read<unsigned char>(is);

	overlay_key = io::read<short>(is);
}

/** Write a Version instance to the eix db */
void Version::write( FILE *os )
{
	// write full string
	io::write_string( os, full );

	// write stability & masking
	io::write<Keywords::Type>(os, _mask);

#if 0
	// write primary string
	io::write_string( os, primary );
#endif

	// write primsplit
	io::write<unsigned char>(os, (unsigned char)primsplit.size());
	for(vector<unsigned short>::iterator it = primsplit.begin();
			it != primsplit.end();
			++it)
	{
		io::write<unsigned short>(os, *it);
	}

	io::write<unsigned char>(os, primarychar);

	// write suffixlevel,suffixnum,gentoorelease
	io::write<unsigned char>(os, suffixlevel);
	io::write<unsigned int>(os, suffixnum);
	io::write<unsigned char>(os, gentoorelease);

	io::write<short>(os, overlay_key);
}
