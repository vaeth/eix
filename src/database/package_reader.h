// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_DATABASE_PACKAGE_READER_H_
#define SRC_DATABASE_PACKAGE_READER_H_ 1

#include <sys/types.h>

#include <memory>
#include <string>

#include "database/header.h"
#include "eixTk/eixint.h"
#include "eixTk/null.h"

class Database;
class DBHeader;
class Package;
class PortageSettings;

/**
Forward-iterate for packages stored in the cachefile
**/
class PackageReader {
	public:
		enum Attributes {
			NONE = 0,
			NAME, DESCRIPTION, HOMEPAGE, LICENSE, VERSIONS,
			ALL = 7
		};

		/**
		Initialize with file-stream and number of packages.
		@arg ps is used to define the local package sets while version reading
		**/
		PackageReader(Database *db, const DBHeader& hdr, PortageSettings *ps)
			: m_db(db), m_frames(hdr.size), m_cat_size(0), m_pkg(NULLPTR), header(&hdr), m_portagesettings(ps), m_error(false) {
		}

		PackageReader(Database *db, const DBHeader& hdr)
			: m_db(db), m_frames(hdr.size), m_cat_size(0), m_pkg(NULLPTR), header(&hdr), m_portagesettings(NULLPTR), m_error(false) {
		}

		~PackageReader();

		/**
		Read attributes from the database into the current package
		**/
		bool read(Attributes need);
		bool read() {
			return read(ALL);
		}

		/**
		Get pointer to the package.
		It's possible that some attributes of the package are not yet read
		from the database.
		**/
		Package *get() const {
			return m_pkg;
		}

		/**
		Skip the current package.
		The file pointer is moved to the next package.
		**/
		bool skip();

		/**
		Release the package.
		Complete the current package, and release it.
		**/
		Package *release();

		/**
		@return true if there is a next package.
		Read the package-header
		**/
		bool next();

#if 0
		/**
		Go into the next (or first) category part.
		@return false if there are none more.
		**/
		bool nextCategory();

		/**
		Read the whole next package in the current category.
		return false if there are none more.
		**/
		bool nextPackage();
#endif

		/**
		@return name of current category
		**/
		const std::string& category() const {
			return m_cat_name;
		}

		const char *get_errtext() const {
			return (m_error ? m_errtext.c_str() : NULLPTR);
		}

	protected:
		Database         *m_db;

		eix::Treesize     m_frames;
		eix::Treesize     m_cat_size;
		std::string       m_cat_name;

		off_t             m_next;
		Attributes        m_have;
		Package          *m_pkg;

		const DBHeader   *header;
		PortageSettings  *m_portagesettings;

		std::string m_errtext;
		bool m_error;
};

#endif  // SRC_DATABASE_PACKAGE_READER_H_
