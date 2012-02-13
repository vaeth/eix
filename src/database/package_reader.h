// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__PACKAGE_READER_H__
#define EIX__PACKAGE_READER_H__ 1

#include <config.h>
#include <database/types.h>
#include <database/header.h>
#include <portage/package.h>

#include <memory>
#include <string>

#include <cstddef>
#include <cstdio>
#include <sys/types.h>

class DBHeader;
class PortageSettings;

/// Forward-iterate for packages stored in the cachefile.
class PackageReader {

	public:
		enum Attributes {
			NONE = 0,
			NAME, DESCRIPTION, HOMEPAGE, LICENSE, COLL_IUSE, VERSIONS,
			ALL = 7
		};

		/** Initialize with file-stream and number of packages.
		    @arg ps is used to define the local package sets while version reading */
		PackageReader(FILE *fp, const DBHeader &hdr, PortageSettings *ps = NULL)
			: m_fp(fp), m_frames(hdr.size), m_cat_size(0), header(&hdr), m_portagesettings(ps)
		{ }

		/// Read attributes from the database into the current package.
		void read(Attributes need = ALL);

		/// Get pointer to the package.
		// It's possible that some attributes of the package are not yet read
		// from the database.
		Package *get() const
		{ return m_pkg.get(); }

		/// Skip the current package.
		// The current package is deleted and the file pointer is moved to the
		// next package.
		void skip();

		/// Release the package.
		// Complete the current package, and release it.
		Package *release()
		{
			read();
			return m_pkg.release();
		}

		/// Return true if there is a next package.
		// Read the package-header.
		bool next();

		/// Go into the next (or first) category part.
		// @return false if there are none more.
		bool nextCategory();

		/// Read the whole next package in the current category.
		// @return false if there are none more.
		bool nextPackage();

		/// Return name of current category.
		const std::string& category() const
		{ return m_cat_name; }

	protected:
		FILE             *m_fp;

		io::Treesize      m_frames;
		io::Treesize      m_cat_size;
		std::string       m_cat_name;

		off_t             m_next;
		Attributes        m_have;
		std::auto_ptr<Package> m_pkg;

		const DBHeader   *header;
		PortageSettings  *m_portagesettings;
	private:
};

#endif /* EIX__PACKAGE_READER_H__ */
