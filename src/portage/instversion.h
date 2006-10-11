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

#ifndef __INSTVERSION_H__
#define __INSTVERSION_H__

#include <portage/basicversion.h>
#include <set>

/** InstVersion expands the BasicVersion class by data relevant for vardbpkg */
class InstVersion : public BasicVersion, public Keywords {

	public:
		/** For versions in vardbpkg we might not yet know the slot.
		    For caching, we mark this here: */
		bool know_slot, read_failed;
		/** Similarly for iuse and usedUse: */
		bool know_use;

		time_t instDate;
		std::set<std::string> usedUse; /* Those useflags in iuse actually used */

		InstVersion() : instDate(0), know_slot(false), read_failed(false), know_use(false)
		{ }

		/** Constructor, calls BasicVersion::parseVersion( str ) */
		InstVersion(const char* str) : BasicVersion(str), instDate(0), know_slot(false), read_failed(false), know_use(false)
		{ }

		/** The equality operator does not test the additional data */
		bool operator == (const InstVersion &v) const
		{ return ((BasicVersion)*this) == ((BasicVersion)v); }

		bool operator == (const BasicVersion &v) const
		{ return ((BasicVersion)*this) == v; }

		bool operator != (const InstVersion &v) const
		{ return !((*this) == v); }

		bool operator != (const BasicVersion &v) const
		{ return !((*this) == v); }

};

#endif /* __INSTVERSION_H__ */
