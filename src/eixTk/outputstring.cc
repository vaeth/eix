// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cstring>

#include <iostream>
#include <string>

#include "eixTk/diagnostics.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/outputstring.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"

using std::string;

using std::cout;

bool OutputString::is_equal(const OutputString& t) const {
	return ((m_string == t.m_string) &&
		// gcc-5 has problems for == with empty vectors
			likely(m_insert.empty() ? t.m_insert.empty() :
				(t.m_insert.empty() ? false :
		likely(m_insert == t.m_insert)
			))
		);
}

void OutputString::assign(const OutputString& s) {
	m_string.assign(s.m_string);
	m_size = s.m_size;
	m_insert = s.m_insert;
	absolute = s.absolute;
}

void OutputString::assign_fast(const string& t) {
	m_string.assign(t);
	m_size = t.size();
	absolute = false;
	m_insert.clear();
}

void OutputString::assign_fast(const char *s) {
	m_string.assign(s);
	m_size = m_string.size();
	absolute = false;
	m_insert.clear();
}

void OutputString::assign_fast(char s) {
	m_string.assign(1, s);
	m_size = 1;
	absolute = false;
	m_insert.clear();
}

void OutputString::assign(const string& t, string::size_type s) {
	m_string.assign(t);
	m_size = s;
	absolute = false;
	m_insert.clear();
}

void OutputString::assign_smart(const string& t) {
	m_string.assign(t);
	m_insert.clear();
	append_internal(t);
}

void OutputString::assign_smart(const char *s) {
	m_string = s;
	m_insert.clear();
	append_internal(m_string);
}

void OutputString::clear() {
	m_string.clear();
	m_size = 0;
	absolute = false;
	m_insert.clear();
}

void OutputString::set_one() {
	m_string.assign(1, '1');
	m_size = 1;
	absolute = false;
	m_insert.clear();
}

void OutputString::append_column(string::size_type s) {
	if(unlikely(absolute)) {
		if(likely(m_size < s)) {
			m_string.append(s - m_size, ' ');
			m_size = s;
		}
	} else {
		m_insert.push_back(m_string.size());
		m_insert.push_back(m_size);
		m_insert.push_back(s);
	}
}

void OutputString::append_escape(const char **pos) {
	const char *band_position = ++*pos;
	char ch(*band_position);
	if(ch == 'C') {
		if(likely(*(++band_position) == '<')) {
			const char *start(++band_position);
			const char *end(strchr(start, '>'));
			if(likely(end != NULLPTR)) {
GCC_DIAG_OFF(sign-conversion)
				string num(start, end - start);
GCC_DIAG_ON(sign-conversion)
				if(likely(is_numeric(num.c_str()))) {
					string::size_type s(my_atoi(num.c_str()));
					if(s > 0) {
						append_column(s);
					}
					*pos = end;
					return;
				}
			}
		}
	}
	append_smart(get_escape(ch));
}

void OutputString::append_fast(char s) {
	m_string.append(1, s);
	++m_size;
}

void OutputString::append_smart(char s) {
	m_string.append(1, s);
	switch(s) {
		case '\a':
		case '\b':
			break;
		case '\n':
		case '\r':
			m_size = 0;
			absolute = true;
			break;
		case '\t':
			if(unlikely(absolute)) {
				m_size += 8 - (m_size % 8);
			} else {
				m_insert.push_back(m_string.size());
				m_insert.push_back(m_size);
				m_insert.push_back(0);
			}
			break;
		default:
			if(likely(isutf8firstbyte(s))) {
				++m_size;
			}
			break;
	}
}

void OutputString::append_internal(const string& t, string::size_type ts, string::size_type s, bool a) {
	string::size_type lastpos(0);
	for(string::size_type currpos;
		unlikely((currpos = t.find_first_of("\a\b\n\r\t", lastpos)) != string::npos);
		lastpos = currpos + 1) {
		s += utf8size(t, lastpos, currpos);
		ts += currpos - lastpos + 1;
		switch(t[currpos]) {
			case '\n':
			case '\r':
				s = 0;
				a = true;
				break;
			case '\t':
				if(unlikely(a)) {
					s += 8 - (s % 8);
				} else {
					m_insert.push_back(ts - 1);
					m_insert.push_back(s);
					m_insert.push_back(0);
				}
			// case '\a':
			// case '\b':
			default:
				break;
		}
	}
	m_size = s + utf8size(t, lastpos);
	absolute = a;
}

void OutputString::append_fast(const string& t) {
	m_string.append(t);
	m_size += t.size();
}

void OutputString::append_fast(const char *s) {
	string::size_type old(m_string.size());
	m_string.append(s);
	m_size += (m_string.size() - old);
}

void OutputString::append(const string& t, string::size_type s) {
	m_string.append(t);
	m_size += s;
}

void OutputString::append_smart(const string& t) {
	append_internal(t, t.size(), m_size, absolute);
	m_string.append(t);
}

void OutputString::append(const OutputString& a) {
	if(a.empty()) {
		return;
	}
	if(a.absolute) {
		absolute = true;
		m_size = a.m_size;
	} else if(absolute) {
		a.print(&m_string, &m_size);
		return;
	} else {
		for(InsertType::const_iterator it(a.m_insert.begin());
			unlikely(it != a.m_insert.end()); ++it) {
			m_insert.push_back((*it) + m_string.size());
			m_insert.push_back((*(++it)) + m_size);
			m_insert.push_back(*(++it));
		}
		m_size += a.m_size;
	}
	m_string.append(a.m_string);
}

void OutputString::print(std::string *dest, WordSize *s) const {
	WordSize inserted(0);
	if(likely(m_insert.empty())) {
		dest->append(m_string);
	} else {
		WordSize r(0);
		WordSize curr(*s);
		for(InsertType::const_iterator it(m_insert.begin());
			unlikely(it != m_insert.end()); ++it) {
			if(*it > r) {
				dest->append(m_string, r, (*it) - r);
				r = *it;
			}
			curr += *(++it);
			WordSize aim(*(++it));
			WordSize d;
			if(aim  == 0) {  // tab
				d = 8 - (curr % 8);
				curr += d;
				inserted += d;
			} else if(curr < aim) {
				d = aim - curr;
				curr = aim;
				inserted += d;
				dest->append(d, ' ');
			}
		}
		dest->append(m_string, r, string::npos);
	}
	if(unlikely(absolute)) {
		*s = m_size;
	} else {
		*s += inserted + m_size;
	}
}

void OutputString::print(WordSize *s) const {
	string d;
	print(&d, s);
	cout << d;
}
