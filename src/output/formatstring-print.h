// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__FORMATSTRING_PRINT_H__
#define EIX__FORMATSTRING_PRINT_H__ 1

#include <string>

class PrintFormat;

std::string get_package_property(const PrintFormat *fmt, const void *entity, const std::string &name);
std::string get_diff_package_property(const PrintFormat *fmt, const void *void_entity, const std::string &name);

#endif /* EIX__FORMATSTRING-PRINT_H__ */
