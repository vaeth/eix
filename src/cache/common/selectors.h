// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__SELECTORS_H__
#define EIX__SELECTORS_H__ 1

#include <eixTk/utils.h>

#include <string>

int package_selector (SCANDIR_ARG3 dent);
int ebuild_selector (SCANDIR_ARG3 dent);

std::string::size_type ebuild_pos(const std::string &str);

#endif /* EIX__SELECTORS_H__ */
