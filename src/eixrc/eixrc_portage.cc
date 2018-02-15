// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "eixrc/eixrc.h"
#include <config.h>  // IWYU pragma: keep

#include <string>

#include "eixTk/dialect.h"
#include "eixTk/formated.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/parseerror.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/conf/portagesettings.h"

using std::string;

void EixRc::known_vars() {
	WordSet vars;
	for(WordUnorderedMap::const_iterator it(main_map.begin());
		it != main_map.end(); ++it) {
		vars.INSERT(it->first);
	}
	ParseError parse_error(true);
	PortageSettings ps(this, &parse_error, false, true);
	for(WordIterateMap::const_iterator it(ps.begin());
		it != ps.end(); ++it) {
		vars.INSERT(it->first);
	}
	for(WordSet::const_iterator it(vars.begin());
		it != vars.end(); ++it) {
		eix::say() % *it;
	}
}

bool EixRc::print_var(const string& key) {
	string print_append((*this)["PRINT_APPEND"]);
	unescape_string(&print_append);
	const char *s;
	if(likely(key != "PORTDIR")) {
		s = cstr(key);
		if(likely(s != NULLPTR)) {
			eix::print("%s%s") % s % print_append;
			return true;
		}
	}
	ParseError parse_error(true);
	PortageSettings ps(this, &parse_error, false, true);
	s = ps.cstr(key);
	if(likely(s != NULLPTR)) {
		eix::print("%s%s") % s % print_append;
		return true;
	}
	return false;
}
