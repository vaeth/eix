// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "package_reader.h"
#include <database/io.h>
#include <database/types.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <portage/conf/portagesettings.h>
#include <portage/package.h>
#include <portage/version.h>

#include <cstdio>
#include <sys/types.h>

using namespace std;

PackageReader::~PackageReader()
{
	delete m_pkg;
}

void
PackageReader::read(Attributes need)
{
	if(likely(m_have >= need)) // Already got this one.
		return;

	switch(m_have)
	{
		case NONE:
			m_pkg->name = io::read_string(m_fp);
			if(unlikely(need == NAME))
				break;
		case NAME:
			m_pkg->desc = io::read_string(m_fp);
			if(unlikely(need == DESCRIPTION))
				break;
		case DESCRIPTION:
			m_pkg->homepage = io::read_string(m_fp);
			if(unlikely(need == HOMEPAGE))
				break;
		case HOMEPAGE:
			m_pkg->licenses = io::read_hash_string(m_fp, header->license_hash);
			if(unlikely(need == LICENSE))
				break;
		case LICENSE:
			for(io::Versize i(io::read<io::Versize>(m_fp)); likely(i); --i)
				m_pkg->addVersion(io::read_version(m_fp, *header));
			if(likely(m_portagesettings != NULLPTR)) {
				m_portagesettings->calc_local_sets(m_pkg);
				m_portagesettings->finalize(m_pkg);
			}
			else {
				m_pkg->finalize_masks();
			}
			m_pkg->save_maskflags(Version::SAVEMASK_FILE);
		default:
		//case ALL:
			break;
	}
	m_have = need;
}

void
PackageReader::skip()
{
	// only seek if needed
	if (m_have != ALL) {
#ifdef HAVE_FSEEKO
		fseeko(m_fp, m_next, SEEK_SET);
#else
		fseek(m_fp, m_next, SEEK_SET);
#endif
	}
}

/// Release the package.
// Complete the current package, and release it.
Package *
PackageReader::release()
{
	read();
	Package *r(m_pkg);
	m_pkg = NULLPTR;
	return r;
}

bool
PackageReader::next()
{
	if(unlikely(m_cat_size-- == 0)) {
		if(unlikely(m_frames-- == 0)) {
			return false;
		}
		m_cat_size = io::read_category_header(m_fp, m_cat_name);
		return next();
	}

	io::OffsetType len(io::read<io::OffsetType>(m_fp));
#ifdef HAVE_FSEEKO
	// We rely on autoconf whose documentation states:
	// All system with fseeko() also supply ftello()
	m_next = ftello(m_fp) + len;
#else
	// We want an off_t-addition, so we cast first to be safe:
	m_next = off_t(ftell(m_fp)) + len;
#endif
	m_have = NONE;
	delete m_pkg;
	m_pkg = new Package;
	m_pkg->category = m_cat_name;

	return true;
}

bool
PackageReader::nextCategory()
{
	if(unlikely(m_frames-- == 0))
		return false;

	m_cat_size = io::read_category_header(m_fp, m_cat_name);
	return true;
}

bool
PackageReader::nextPackage()
{
	if(unlikely(m_cat_size-- == 0))
		return false;

	/* Ignore the offset and read the whole package at once.
	 */

	io::read<io::OffsetType>(m_fp);
	m_have = NONE;
	delete m_pkg;
	m_pkg = new Package;
	m_pkg->category = m_cat_name;
	read(ALL);
	return true;
}
