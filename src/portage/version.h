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

#ifndef __VERSION_H__
#define __VERSION_H__

#include <database/io.h>
#include <portage/basicversion.h>
#include <portage/keywords.h>

#include <iostream>

/** Version expands the BasicVersion class by Keywords, overlay-keys, slots */
class Version : public BasicVersion, public Keywords {

	public:
		friend void     io::write_version(FILE *fp, const Version *v, bool small);
		friend Version *io::read_version(FILE *fp);

		typedef unsigned short Overlay;

		/** Key for Portagedb.overlays/overlaylist from header. */
		Overlay overlay_key;
		/** The slot, the version represents.
		    For saving space, the default "0" is always stored as "" */
		std::string slot;

		Version() : overlay_key(0)
		{ }

		/** Constructor, calls BasicVersion::parseVersion( str ) */
		Version(const char* str) : BasicVersion(str), overlay_key(0), slot("")
		{ }

		/** The equality operator does *not* test the slots */
		bool operator == (const Version &v) const
		{
			return ((((BasicVersion)*this) == ((BasicVersion)v))
			        && (overlay_key == v.overlay_key));
		}

		bool operator == (const BasicVersion &v) const
		{ return ((BasicVersion)*this) == v; }

		bool operator != (const Version &v) const
		{ return !((*this) == v); }

		bool operator != (const BasicVersion &v) const
		{ return !((*this) == v); }

		std::string getSlotAppendix (bool colon) const
		{
			if(slot.length())
			{
				if(colon)
					return std::string(":") + slot;
				return std::string("(") + slot + ")";
			}
			return "";
		}

		std::string getFullSlotted (bool colon) const
		{ return std::string(getFull()) + getSlotAppendix(colon); }
};

#endif /* __VERSION_H__ */
