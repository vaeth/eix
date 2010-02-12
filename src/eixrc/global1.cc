// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "global.h"
#include <config.h>
#include <eixrc/eixrc.h>
#include <eixTk/i18n.h>

#include <string>

#include <cassert>

#include <cstdlib>
#define DO_STRINGIFY(a) #a
#define EXPAND_STRINGIFY(a) DO_STRINGIFY(a)
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif


#define DEFAULT_PART 1

void fill_defaults_part_1(EixRc &eixrc)
{
#include <eixrc/defaults.cc>
}

using namespace std;

/** Create a static EixRc and fill with defaults.
 * This should only be called once! */
static
EixRc &
get_eixrc_once(const char *varprefix)
{
	static EixRc eixrc;
	assert(varprefix);
	eixrc.varprefix = string(varprefix);

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
	static EixRc &rc(get_eixrc_once(varprefix));
	return rc;
}
