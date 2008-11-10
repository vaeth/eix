// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__SELECTORS_H__)
#define EIX__SELECTORS_H__

#include <eixTk/utils.h>

int package_selector (SCANDIR_ARG3 dent);
int ebuild_selector (SCANDIR_ARG3 dent);

#endif /* EIX__SELECTORS_H__ */
