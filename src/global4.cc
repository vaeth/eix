// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "global.h"
#include <cstdlib>
#include <eixrc/eixrc.h>
#include <eixTk/exceptions.h>
#include <config.h>

#define DEFAULT_PART 4

void fill_defaults_part_4(EixRc &eixrc)
{
#include <eixrc/defaults.cc>
}
