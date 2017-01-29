// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_OUTPUT_FORMATSTRING_PRINT_H_
#define SRC_OUTPUT_FORMATSTRING_PRINT_H_ 1

#include <string>

class PrintFormat;
class OutputString;

ATTRIBUTE_NONNULL_ void get_package_property(OutputString *s, const PrintFormat *fmt, void *entity, const std::string& name);
ATTRIBUTE_NONNULL_ void get_diff_package_property(OutputString *s, const PrintFormat *fmt, void *void_entity, const std::string& name);

#endif  // SRC_OUTPUT_FORMATSTRING_PRINT_H_
