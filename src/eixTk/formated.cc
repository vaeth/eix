// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/formated.h"
#include <config.h>

#include <cstdio>
#include <cstdlib>

#include <string>
#include <vector>

#include "eixTk/dialect.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"

using std::string;

// check_includes.sh: eix::format()

namespace eix {

const FormatManip::ArgType
	FormatManip::NONE,
	FormatManip::STRING,
	FormatManip::DIGIT,
	FormatManip::BOTH;

void format::bad_format() const {
	eix::say_error(_("internal error: bad format specification \"%s\""))
		% m_text;
	std::exit(EXIT_FAILURE);
}

#ifdef EIX_DEBUG_FORMAT
void format::too_few_arguments() const {
	eix::say_error(_("internal error: too few arguments passed for \"%s\""))
		% m_text;
	std::exit(EXIT_FAILURE);
}

void format::too_many_arguments() const {
	eix::say_error(_("internal error: too many arguments passed for \"%s\""))
		% m_text;
	std::exit(EXIT_FAILURE);
}
#endif

void format::init() {
	simple = false;
	current = 0;
	FormatManip::ArgCount imp(0);
	string::size_type i(0);
	while(i = m_text.find('%', i), likely(i != string::npos)) {
		string::size_type start(i), len(2);
		++i;
		if(unlikely(i == m_text.size())) {
			bad_format();
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
		ArgCount argnum(imp++);
		if(my_isdigit(c)) {
			string::size_type e(m_text.find('$', i));
			if(unlikely(e == string::npos)) {
				bad_format();
			}
			len += e - start;
			string number(m_text, i, e - i);
			if(unlikely(!is_numeric(number.c_str()))) {
				bad_format();
			}
			argnum = my_atou(number.c_str());
			if(unlikely(argnum <= 0)) {
				bad_format();
			}
			--argnum;
			++e;
			if(unlikely(e == m_text.size())) {
				bad_format();
			}
			c = m_text[e];
		}
		ArgType typ;
#ifdef EIX_DEBUG_FORMAT
		switch(c) {
			case 's':
				typ = FormatManip::STRING;
				break;
			case 'd':
				typ = FormatManip::DIGIT;
				break;
			default:
				bad_format();
				break;
		}
#else
		typ = ((c == 's') ? FormatManip::STRING : FormatManip::DIGIT);
#endif
		if(likely(argnum == wanted.size())) {
			wanted.PUSH_BACK(typ);
		} else if(argnum > wanted.size()) {
			wanted.insert(wanted.end(), argnum - wanted.size() + 1, FormatManip::NONE);
			wanted[argnum] = typ;
		} else {
			wanted[argnum] |= typ;
		}
		manip.EMPLACE_BACK(FormatManip, (start, argnum, typ));
		m_text.erase(start, len);
		i = start;
		if(i == m_text.size()) {
			break;
		}
	}
	if(unlikely(wanted.empty())) {
		finalize();
	} else {
		args.insert(args.end(), wanted.size(), FormatReplace());
	}
}

void format::finalize() {
	for(std::vector<FormatManip>::const_reverse_iterator it(manip.rbegin());
		it != manip.rend(); ++it) {
		m_text.insert(it->m_index, (it->m_type ?
			args[it->argnum].s : args[it->argnum].d));
	}
	manip.clear();
	wanted.clear();
	args.clear();
	newline_output();
}

void format::newline_output() {
	if(add_newline) {
		m_text.append(1, '\n');
	}
	if(output != NULLPTR) {
		if(likely(!m_text.empty())) {
			std::fwrite(m_text.c_str(), sizeof(char),  m_text.size(), output);
		}
		if(unlikely(do_flush)) {
			std::fflush(output);
		}
	}
}

}/* namespace eix */
