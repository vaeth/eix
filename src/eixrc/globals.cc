// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixrc/globals.h"
#include <config.h>  // NOLINT(build/include_order)

void fill_defaults(EixRc *eixrc) {
#include "eixrc/defaults.cc"  // NOLINT(build/include)
#include "eixrc/def_i18n.cc"  // NOLINT(build/include)
}
