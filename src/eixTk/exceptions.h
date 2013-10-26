// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_EXCEPTIONS_H_
#define SRC_EIXTK_EXCEPTIONS_H_ 1

// include "eixTk/exceptions.h" This comment satisfies check_include script
#include <string>
#include <vector>

#include "eixTk/diagnostics.h"

/// Provide a common look for error-messages for parse-errors in
/// portage.{mask,keywords,..}.
void portage_parse_error(const std::string &file, std::vector<std::string>::size_type line_nr, const std::string& line, const std::string &errtext);

template<class Iterator> inline static void portage_parse_error(const std::string &file, const Iterator &begin, const Iterator &line, const std::string &errtext);

template<class Iterator> inline static void portage_parse_error(const std::string &file, const Iterator &begin, const Iterator &line, const std::string &errtext) {
GCC_DIAG_OFF(sign-conversion)
	portage_parse_error(file, std::distance(begin, line) + 1, *line, errtext);
GCC_DIAG_ON(sign-conversion)
}

#endif  // SRC_EIXTK_EXCEPTIONS_H_
