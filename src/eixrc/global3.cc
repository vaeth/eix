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

#define DEFAULT_PART 3

void fill_defaults_part_3(EixRc &eixrc)
{
#include <eixrc/defaults.cc>
}
