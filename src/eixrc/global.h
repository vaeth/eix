// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXRC_GLOBAL_H_
#define SRC_EIXRC_GLOBAL_H_ 1

#include <config.h>

class EixRc;

/** Must be called exactly once before get_eixrc() can be used */
EixRc &get_eixrc(const char *varprefix);

/** Return a static eixrc. */
EixRc &get_eixrc() ATTRIBUTE_PURE;

void fill_defaults_part_1(EixRc *eixrc);
void fill_defaults_part_2(EixRc *eixrc);
void fill_defaults_part_3(EixRc *eixrc);
void fill_defaults_part_4(EixRc *eixrc);
void fill_defaults_part_5(EixRc *eixrc);

#endif  // SRC_EIXRC_GLOBAL_H_
