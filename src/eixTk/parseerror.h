// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_PARSEERROR_H_
#define SRC_EIXTK_PARSEERROR_H_ 1

#include <config.h>

#include <string>

#include "eixTk/diagnostics.h"
#include "eixTk/stringtypes.h"

/**
Provide a common look for error-messages for parse-errors in
portage.{mask,keywords,..}
**/
class ParseError {
	private:
		bool tacit;

	public:
		ParseError() : tacit(false) {
		}

		explicit ParseError(bool no_warn) : tacit(no_warn) {
		}

		void init(bool no_warn) {
			tacit = no_warn;
		}

		void output(const std::string& file, LineVec::size_type line_nr, const std::string& line, const std::string& errtext) const;

		template<class Iterator> void output(const std::string& file, const Iterator& begin, const Iterator& line, const std::string& errtext) const {
GCC_DIAG_OFF(sign-conversion)
			output(file, std::distance(begin, line) + 1, *line, errtext);
GCC_DIAG_ON(sign-conversion)
		}
};

#endif  // SRC_EIXTK_PARSEERROR_H_
