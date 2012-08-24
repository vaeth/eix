// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixrc/eixrc.h"
#include "portage/conf/portagesettings.h"

using std::map;
using std::set;
using std::string;

using std::cout;

void
EixRc::known_vars()
{
	set<string> vars;
	for(map<string, string>::const_iterator it(main_map.begin());
		it != main_map.end(); ++it) {
		vars.insert(it->first);
	}
	PortageSettings ps(this, false, true);
	for(map<string, string>::const_iterator it(ps.begin());
		it != ps.end(); ++it) {
		vars.insert(it->first);
	}
	for(set<string>::const_iterator it(vars.begin());
		it != vars.end(); ++it) {
		cout << *it << "\n";
	}
}

bool
EixRc::print_var(const string &key)
{
	string print_append((*this)["PRINT_APPEND"]);
	unescape_string(&print_append);
	const char *s;
	if(likely(key != "PORTDIR")) {
		s = cstr(key);
		if(likely(s != NULLPTR)) {
			cout << s << print_append;
			return true;
		}
	}
	PortageSettings ps(this, false, true);
	s = ps.cstr(key);
	if(likely(s != NULLPTR)) {
		cout << s << print_append;
		return true;
	}
	return false;
}
