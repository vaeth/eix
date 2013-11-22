// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cstdlib>

#include "eixTk/assert.h"
#include "eixTk/i18n.h"
#include "eixTk/null.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"

#define DO_STRINGIFY(a) #a
#define EXPAND_STRINGIFY(a) DO_STRINGIFY(a)
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define DEFAULT_PART 1

static EixRc *static_eixrc = NULLPTR;

void fill_defaults_part_1(EixRc *eixrc) {
#include "eixrc/defaults.cc"
// _( SYSCONFDIR This comment  satisfies check_includes script
}

/** Must be called exactly once before get_eixrc() can be used */
EixRc& get_eixrc(const char *varprefix) {
	eix_assert_static(static_eixrc == NULLPTR);
	static_eixrc = new EixRc(varprefix);

	fill_defaults_part_1(static_eixrc);
	fill_defaults_part_2(static_eixrc);
	fill_defaults_part_3(static_eixrc);
	fill_defaults_part_4(static_eixrc);
	fill_defaults_part_5(static_eixrc);

	static_eixrc->read();
	return *static_eixrc;
}

/** Return reference to internal static EixRc.
 * This can be called everywhere! */
EixRc& get_eixrc() {
	eix_assert_static(static_eixrc != NULLPTR);
	return *static_eixrc;
}
