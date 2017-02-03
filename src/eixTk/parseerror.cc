// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/parseerror.h"
#include <config.h>

#include <set>
#include <string>

#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"

using std::string;
using std::set;

static set<string> *printed = NULLPTR;

/**
Provide a common look for error-messages for parse-errors in
portage.{mask,keywords,..}
**/
void ParseError::output(const string& file, const LineVec::size_type line_nr, const string& line, const string& errtext) const {
	if(tacit) {
		return;
	}
	if(printed == NULLPTR) {
		printed = new set<string>;
	}
	string cache(eix::format("%s\a%s\v%s") % file % line_nr % errtext);
	if(printed->find(cache) != printed->end()) {
		return;
	}
	printed->insert(cache);
	eix::say_error(_("-- invalid line %s in %s: \"%s\""))
		% line_nr % file % line;

	// Indent the message correctly
	WordVec lines;
	split_string(&lines, errtext, false, "\n", false);
	for(WordVec::const_iterator i(lines.begin()); likely(i != lines.end()); ++i) {
		eix::say_error("    %s") % (*i);
	}
}
