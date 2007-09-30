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

#include "global.h"
#include <cstdlib>
#include <eixrc/eixrc.h>
#include <eixTk/exceptions.h>
#include <config.h>

#define DEFAULT_PART 1

void fill_defaults_part_1(EixRc &eixrc)
{
#include <eixrc/defaults.cc>
}

/** Create a static EixRc and fill with defaults.
 * This should only be called once! */
static
EixRc *
get_eixrc_once(const char *varprefix)
{
	static EixRc eixrc;
	eixrc.varprefix = std::string(varprefix);

	fill_defaults_part_1(eixrc);
	fill_defaults_part_2(eixrc);
	fill_defaults_part_3(eixrc);
	fill_defaults_part_4(eixrc);

	eixrc.read();
	return &eixrc;
}


/** Return reference to internal static EixRc.
 * This can be called everywhere! */
EixRc &
get_eixrc(const char *varprefix)
{
	static EixRc *rc = NULL;
	if(rc)
		return *rc;
	ASSERT(varprefix, "internal error: get_eixrc was not initialized with proper argument");
	rc = get_eixrc_once(varprefix);
	return *rc;
}
