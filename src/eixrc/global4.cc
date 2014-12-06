// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include "eixTk/i18n.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"

#define DEFAULT_PART 4

void fill_defaults_part_4(EixRc *eixrc) {
#include "eixrc/defaults.cc"  // NOLINT(build/include)
// _( SYSCONFDIR This comment  satisfies check_includes script
}
