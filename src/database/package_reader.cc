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
			m_pkg->provide = io::read_string(m_fp);
			if(need == PROVIDE)
				break;
		case PROVIDE:
			m_pkg->homepage = io::read_string(m_fp);
			if(need == HOMEPAGE)
				break;
		case HOMEPAGE:
			m_pkg->licenses = io::read_string(m_fp);
			if(need == LICENSE)
				break;
		case LICENSE:
			m_pkg->coll_iuse = io::read_string(m_fp);
#if defined(NOT_FULL_USE)
			if(need == COLL_IUSE)
				break;
		case COLL_IUSE:
#endif
			{
				io::Versize n = io::read<io::Versize>(io::Versizesize, m_fp);
				for(io::Versize i = 0; i != n; i++ ) {
					m_pkg->addVersion(io::read_version(m_fp));
				}
			}
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

	m_next =  ftello(m_fp) + io::read<PackageReader::Offset>(PackageReader::Offsetsize, m_fp);
	m_have = NONE;
	m_pkg.reset(new Package());
	m_pkg->category = m_cat_name;

	return true;
}
