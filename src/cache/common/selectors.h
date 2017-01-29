// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_COMMON_SELECTORS_H_
#define SRC_CACHE_COMMON_SELECTORS_H_ 1

#include <config.h>

#include <string>

#include "eixTk/attribute.h"
#include "eixTk/utils.h"

ATTRIBUTE_PURE int package_selector(SCANDIR_ARG3 dent);
ATTRIBUTE_PURE int ebuild_selector(SCANDIR_ARG3 dent);

std::string::size_type ebuild_pos(const std::string& str);

#endif  // SRC_CACHE_COMMON_SELECTORS_H_
