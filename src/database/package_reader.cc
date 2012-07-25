// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <sys/types.h>

#include <cstdio>

#include "database/io.h"
#include "database/package_reader.h"
#include "database/types.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "portage/conf/portagesettings.h"
#include "portage/package.h"
#include "portage/version.h"

PackageReader::~PackageReader()
{
	delete m_pkg;
}

bool
PackageReader::read(Attributes need)
{
	if(likely(m_have >= need))  // Already got this one
		return true;

	switch(m_have)
	{
		case NONE:
			if(unlikely(!io::read_string(&(m_pkg->name), m_fp, &m_errtext))) {
				m_error = true;
				return false;
			}
			if(unlikely(need == NAME))
				break;
		case NAME:
			if(unlikely(!io::read_string(&(m_pkg->desc), m_fp, &m_errtext))) {
				m_error = true;
				return false;
			}
			if(unlikely(need == DESCRIPTION))
				break;
		case DESCRIPTION:
			if(unlikely(!io::read_string(&(m_pkg->homepage), m_fp, &m_errtext))) {
				m_error = true;
				return false;
			}
			if(unlikely(need == HOMEPAGE))
				break;
		case HOMEPAGE:
			if(unlikely(!io::read_hash_string(header->license_hash, &(m_pkg->licenses), m_fp, &m_errtext))) {
				m_error = true;
				return false;
			}
			if(unlikely(need == LICENSE))
				break;
		case LICENSE:
			{
				io::Versize i;
				if(unlikely(!io::read_num(&i, m_fp, &m_errtext))) {
					m_error = true;
					return false;
				}
				for(; likely(i != 0); --i) {
					Version *v;
					if(unlikely(!io::read_version(v, *header, m_fp, &m_errtext))) {
						m_error = true;
						return false;
					}
					m_pkg->addVersion(v);
				}
			}
			if(likely(m_portagesettings != NULLPTR)) {
				m_portagesettings->calc_local_sets(m_pkg);
				m_portagesettings->finalize(m_pkg);
			} else {
				m_pkg->finalize_masks();
			}
			m_pkg->save_maskflags(Version::SAVEMASK_FILE);
		default:
		// case ALL:
			break;
	}
	m_have = need;
	return true;
}

bool
PackageReader::skip()
{
	// only seek if needed
	if (m_have != ALL) {
#ifdef HAVE_FSEEKO
		if(unlikely(fseeko(m_fp, m_next, SEEK_SET) != 0))
#else
		if(unlikely(fseek(m_fp, m_next, SEEK_SET) != 0))
#endif
		{
			m_errtext = _("fseek failed");
			m_error = true;
			return false;
		}
	}
	return true;
}

/// Release the package.
// Complete the current package, and release it.
Package *
PackageReader::release()
{
	if(unlikely(!read())) {
		return NULLPTR;
	}
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
		if(unlikely(!io::read_category_header(&m_cat_name, &m_cat_size, m_fp, &m_errtext))) {
			m_error = true;
			return false;
		}
		return next();
	}

	io::OffsetType len;
	if(unlikely(!io::read_num(&len, m_fp, &m_errtext))) {
		m_error = true;
		return false;
	}
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
	if(unlikely(m_frames-- == 0)) {
		return false;
	}

	if(likely(io::read_category_header(&m_cat_name, &m_cat_size, m_fp, &m_errtext))) {
		return true;
	}
	m_error = true;
	return false;
}

bool
PackageReader::nextPackage()
{
	if(unlikely(m_cat_size-- == 0))
		return false;

	/* Ignore the offset and read the whole package at once.
	 */

	io::OffsetType dummy;
	if(unlikely(!io::read_num(&dummy, m_fp, &m_errtext))) {
		m_error = true;
		return false;
	}
	m_have = NONE;
	delete m_pkg;
	m_pkg = new Package;
	m_pkg->category = m_cat_name;
	return read(ALL);
}
