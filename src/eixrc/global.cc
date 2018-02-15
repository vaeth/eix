// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixrc/global.h"
#include <config.h>  // IWYU pragma: keep

#include "eixTk/assert.h"
#include "eixTk/null.h"
#include "eixrc/eixrc.h"

static EixRc *static_eixrc = NULLPTR;

/**
Must be called exactly once before get_eixrc() can be used
**/
EixRc& get_eixrc(const char *varprefix) {
	eix_assert_static(static_eixrc == NULLPTR);
	static_eixrc = new EixRc(varprefix);

#ifdef JUMBO_BUILD
	fill_defaults(static_eixrc);
#else
	fill_defaults_part_1(static_eixrc);
	fill_defaults_part_2(static_eixrc);
	fill_defaults_part_3(static_eixrc);
	fill_defaults_part_4(static_eixrc);
	fill_defaults_part_5(static_eixrc);
	fill_defaults_part_6(static_eixrc);
#endif

	static_eixrc->read();
	return *static_eixrc;
}

/**
@return reference to internal static EixRc.
This can be called everywhere!
**/
EixRc& get_eixrc() {
	eix_assert_static(static_eixrc != NULLPTR);
	return *static_eixrc;
}
