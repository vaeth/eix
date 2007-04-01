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
#include <eixTk/stringutils.h>
#include <eixTk/utils.h>

#include <iostream>

/* If NOT_FULL_USE is defined, then the iuse data will be handled per package
   and not per version to save memory and disk space.
   More precisely, if NOT_FULL_USE is defined then the version::iuse entry
   will be empty most of the time:
   The entry is cleared in Package::collect_iuse() which is called by
   Package::addVersionFinalize() / Package::addVersion()
   whenever a version is added to the package: Before clearing,
   collect_iuse() adds the corresponding data to the package-wide data.
   If on the other hand, NOT_FULL_USE is undefined, collect_iuse() will not
   delete this data, and the database-output function will write an empty
   string for the package-wide IUSE data, and the database-reading function
   will get forced to read all package versions (using Package::addVersion()
   and thus calculating the package-wide IUSE) before the package-wide
   IUSE data is assumed to be known. */

#define NOT_FULL_USE

/** Version expands the BasicVersion class by data relevant for versions in tree/overlays */
class Version : public BasicVersion, public Keywords {

	public:
		friend void     io::write_version(FILE *fp, const Version *v, bool small);
		friend Version *io::read_version(FILE *fp);

		/** If NOT_FULL_USE is defined, this might "falsely" be empty
		    to save memory. See the comments above NOT_FULL_USE. */
		std::vector<std::string> iuse;

		typedef io::Short Overlay;
		static const unsigned short Overlaysize = io::Shortsize;

		/** Key for Portagedb.overlays/overlaylist from header. */
		Overlay overlay_key;

		Version() : overlay_key(0)
		{ }

		/** Constructor, calls BasicVersion::parseVersion( str ) */
		Version(const char* str) : BasicVersion(str), overlay_key(0)
		{ }

		void set_iuse(const std::string &i)
		{
			iuse = split_string(i);
			sort_uniquify(iuse);
		}

		std::string get_iuse() const
		{ return join_vector(iuse); }

		/** The equality operator does *not* test the slots */
		bool operator == (const Version &v) const
		{
			return ((*(dynamic_cast<const BasicVersion*>(this)) == dynamic_cast<const BasicVersion&>(v))
				&& (overlay_key == v.overlay_key));
		}

		bool operator == (const BasicVersion &v) const
		{ return (*(dynamic_cast<const BasicVersion *>(this)) == v); }

		bool operator != (const Version &v) const
		{ return !((*this) == v); }

		bool operator != (const BasicVersion &v) const
		{ return !((*this) == v); }

};

#endif /* __VERSION_H__ */
