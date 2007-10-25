// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "global.h"
#include <cstdlib>
#include <cassert>
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
EixRc &
get_eixrc_once(const char *varprefix)
{
	static EixRc eixrc;
	assert(varprefix);
	eixrc.varprefix = std::string(varprefix);

	fill_defaults_part_1(eixrc);
	fill_defaults_part_2(eixrc);
	fill_defaults_part_3(eixrc);
	fill_defaults_part_4(eixrc);

	eixrc.read();
	return eixrc;
}


/** Return reference to internal static EixRc.
 * This can be called everywhere! */
EixRc &
get_eixrc(const char *varprefix)
{
	static EixRc &rc = get_eixrc_once(varprefix);
	return rc;
}
