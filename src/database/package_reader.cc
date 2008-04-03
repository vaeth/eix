// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "package_reader.h"

#include <portage/package.h>
#include <portage/version.h>

#include <database/io.h>

void
PackageReader::read(Attributes need)
{
	if(m_have >= need) // Already got this one.
		return;

	switch(m_have)
	{
		case NONE:
			m_pkg->name = io::read_string(m_fp);
			if(need == NAME)
				break;
		case NAME:
			m_pkg->desc = io::read_string(m_fp);
			if(need == DESCRIPTION)
				break;
		case DESCRIPTION:
			m_pkg->provide = io::read_hash_string(m_fp, header->provide_hash);
			if(need == PROVIDE)
				break;
		case PROVIDE:
			m_pkg->homepage = io::read_string(m_fp);
			if(need == HOMEPAGE)
				break;
		case HOMEPAGE:
			m_pkg->licenses = io::read_hash_string(m_fp, header->license_hash);
			if(need == LICENSE)
				break;
		case LICENSE:
			io::read_hash_container(m_fp, header->iuse_hash,
					std::inserter(m_pkg->m_collected_iuse, m_pkg->m_collected_iuse.begin()));
#if defined(NOT_FULL_USE)
			if(need == COLL_IUSE)
				break;
		case COLL_IUSE:
#endif
			{
				for(io::Versize i = io::read<io::Versize>(m_fp); i; i-- ) {
					m_pkg->addVersion(io::read_version(m_fp, *header));
				}
			}
			m_pkg->save_maskflags(Version::SAVEMASK_FILE);
		//case COLL_IUSE: // If NOT_FULL_USE
		//case ALL:
		default:
			break;
	}
	m_have = need;
}

void
PackageReader::skip()
{
	// only seek if needed
	if (m_have != ALL)
		fseeko(m_fp, m_next , SEEK_SET);
	m_pkg.reset();
}

bool
PackageReader::next()
{
	if(m_cat_size-- == 0)
	{
		if(m_frames-- == 0)
		{
			return false;
		}
		m_cat_size = io::read_category_header(m_fp, m_cat_name);
		return next();
	}

	io::OffsetType len = io::read<io::OffsetType>(m_fp);
	m_next =  ftello(m_fp) + len;
	m_have = NONE;
	m_pkg.reset(new Package());
	m_pkg->category = m_cat_name;

	return true;
}

bool
PackageReader::nextCategory()
{
	if(m_frames-- == 0)
		return false;

	m_cat_size = io::read_category_header(m_fp, m_cat_name);
	return true;
}

bool
PackageReader::nextPackage()
{
	if (m_cat_size-- == 0)
		return false;

	/* Ignore the offset and read the whole package at once.
	 */

	io::read<io::OffsetType>(m_fp);
	m_have = NONE;
	m_pkg.reset(new Package());
	m_pkg->category = m_cat_name;
	read(ALL);
	return true;
}
