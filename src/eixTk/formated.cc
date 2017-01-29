// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/formated.h"
#include <config.h>

#include <cstdlib>

#include <iostream>
#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/eixint.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/stringutils.h"

using std::string;

using std::cerr;
using std::endl;

ATTRIBUTE_NORETURN static void badformat();
static void badformat() {
	cerr << _("bad % in eix::format; perhaps %% is meant") << endl;
	exit(EXIT_FAILURE);
}

eix::format::format(const string& format_string) {
	m_text.assign(format_string);
	simple = false;
	current = 0;
	FormatManip::ArgCount imp(0);
	string::size_type i(0);
	while(i = m_text.find('%', i), likely(i != string::npos)) {
		string::size_type start(i), len(2);
		if(unlikely(++i == m_text.size())) {
			badformat();
		}
		char c(m_text[i]);
		if(c == '%') {
			m_text.erase(start, len);
			if(likely(start != m_text.size())) {
				i = start;
				continue;
			}
			break;
		}
		FormatManip::ArgCount argnum(imp++);
		if(my_isdigit(c)) {
			string::size_type e(m_text.find('$', i));
			if(unlikely(e == string::npos)) {
				badformat();
			}
			len += e - start;
			string number(m_text, i, e - i);
#ifndef NDEBUG
			if(unlikely(!is_numeric(number.c_str()))) {
				badformat();
			}
#endif
			argnum = my_atoi(number.c_str());
			if(unlikely(argnum <= 0)) {
				badformat();
			}
			--argnum;
			if(unlikely(++e == m_text.size())) {
				badformat();
			}
			c = m_text[e];
		}
		eix::SignedBool typ;
		switch(c) {
			case 's':
				typ = 1;
				break;
			case 'd':
				typ = -1;
				break;
			default:
				badformat();
				break;
		}
		if(argnum == wanted.size()) {
			wanted.push_back(typ);
		} else if(argnum > wanted.size()) {
			wanted.insert(wanted.end(), argnum - wanted.size() + 1, 0);
			wanted[argnum] = typ;
		} else if(wanted[argnum] != typ) {
			wanted[argnum] = 0;
		}
		manip.push_back(FormatManip(start, argnum, typ));
		m_text.erase(start, len);
		i = start;
		if(i == m_text.size()) {
			break;
		}
	}
	if(unlikely(wanted.empty())) {
		manipulate();
	} else {
		args.insert(args.end(), wanted.size(), FormatReplace());
	}
}

void eix::format::manipulate() {
	for(std::vector<FormatManip>::const_reverse_iterator it(manip.rbegin());
		it != manip.rend(); ++it) {
		m_text.insert(it->m_index, (it->m_type ?
			args[it->argnum].s : args[it->argnum].d));
	}
	manip.clear();
	wanted.clear();
	args.clear();
}
