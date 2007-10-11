// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PACKAGE_READER_H__
#define __PACKAGE_READER_H__

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <memory>

#include <database/io.h>

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
		typedef io::UInt Offset;

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
