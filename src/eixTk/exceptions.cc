// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <iostream>
#include <string>

#include "eixTk/exceptions.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"

using std::string;

using std::cerr;
using std::endl;

/**
Provide a common look for error-messages for parse-errors in
portage.{mask,keywords,..}
**/
void portage_parse_error(const string& file, const LineVec::size_type line_nr, const string& line, const string& errtext) {
	cerr << eix::format(_("-- invalid line %s in %s: \"%s\""))
		% line_nr % file % line << endl;

	// Indent the message correctly
	WordVec lines;
	split_string(&lines, errtext, false, "\n", false);
	for(WordVec::const_iterator i(lines.begin()); likely(i != lines.end()); ++i) {
		cerr << "    " << *i << endl;
	}
	cerr << endl;
}
