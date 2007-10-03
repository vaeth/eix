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

#ifndef __PACKAGE_READER_H__
#define __PACKAGE_READER_H__

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <memory>

// No forward decl of Package because gcc-3.3.6 will scream bloody
// murder.
#include <portage/package.h>

/// Forward-iterate for packages stored in the cachefile.
class PackageReader {

	public:
		enum Attributes {
			NONE = 0,
			NAME, DESCRIPTION, PROVIDE, HOMEPAGE, LICENSE, COLL_IUSE, VERSIONS,
			ALL = 7
		};

		/** In general, Offsetsize will not be sizeof(Offset),
		    because it should be system independent. It must only be
		    large enough to store the offsets occurring in the database.
		*/
		typedef off_t Offset;
		static const unsigned short Offsetsize = io::Longsize;

		/// Initialize with file-stream and number of packages.
		PackageReader(FILE *fp, io::Treesize size)
			: m_fp(fp), m_frames(size), m_cat_size(0) { }

		/// Read attributes from the database into the current package.
		void read(Attributes need = ALL);

		/// Get pointer to the package.
		// It's possible that some attributes of the package are not yet read
		// from the database.
		Package *get() const;

		/// Skip the current package.
		// The current package is deleted and the file pointer is moved to the
		// next package.
		void skip();

		/// Release the package.
		// Complete the current package, and release it.
		Package *release();

		/// Return true if there is a next package.
		// Read the package-header.
		bool next();

	protected:
		FILE             *m_fp;

		io::Treesize      m_frames;
		io::Treesize      m_cat_size;
		std::string       m_cat_name;

		off_t             m_next;
		Attributes        m_have;
		std::auto_ptr<Package> m_pkg;

	private:
};

inline Package *
PackageReader::get() const
{
	return m_pkg.get();
}

inline Package *
PackageReader::release()
{
	read();
	return m_pkg.release();
}

#endif /* __PACKAGE_READER_H__ */
