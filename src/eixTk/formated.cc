// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <string>

#include "eixTk/formated.h"

using std::string;

using eix::format;

/// Copy current state of formater.
format& format::operator=(const format& e) {
	m_spec = e.m_spec;
	m_stream.str(e.m_stream.str());
	m_format = e.m_format;
	return *this;
}

/// Reset the internal state and use format_string as the new format string.
format& format::reset(const string& format_string) {
	m_spec = 0;
	m_stream.str("");
	m_format = format_string;
	goto_next_spec();
	return *this;
}

void format::goto_next_spec() {
	m_spec = 0;
	string::size_type next(m_format.find('%'));
	if(next == string::npos || m_format.size() < next + 2) {
		// there are no more specifier, so we move the remaining text to
		// our stream.
		m_stream << m_format;
		m_format.clear();
	} else if(m_format[next + 1] == '%') {
		// %% gives a single %
		m_stream << m_format.substr(0, next + 1);
		m_format.erase(0, next + 2);
		goto_next_spec();
	} else {
		// remember the specifier so we can use it in the next call to
		// the %-operator.
		m_spec = m_format[next + 1];
		m_stream << m_format.substr(0, next);
		m_format.erase(0, next + 2);
	}
}
