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

#ifndef __VERSION_H__
#define __VERSION_H__

#include <iostream>

#include <portage/basicversion.h>
#include <portage/keywords.h>

/** Version expands the BasicVersion class by portagedb stability keys */
class Version : public BasicVersion, public Keywords {

	public:
		/** Constructor, calls BasicVersion::parseVersion( str ) */
		Version( const char* str );
		/** Constructor, calls BasicVersion::parseVersion( str ) */
		Version( string str );
		/** Constructor, calls BasicVersion::parseVersion( str ) */
		Version(FILE *stream);

		/** Read a Version instance from the eix db */
		void read( FILE *is );
		/** Write a Version instance to the eix db */
		void write( FILE *os );

		/** Key for Portagedb.overlays/overlaylist from header. */
		unsigned short overlay_key;

		bool operator == (const Version &v) {
			return ((BasicVersion)*this) == ((BasicVersion)*&v) && overlay_key == v.overlay_key;
		}

		bool operator == (const BasicVersion &v) {
			return ((BasicVersion)*this) == v;
		}
};

#endif /* __VERSION_H__ */
