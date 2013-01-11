// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <iostream>
#include <string>
#include <vector>

#include "eixTk/diagnostics.h"
#include "eixTk/likely.h"
#include "eixTk/outputstring.h"
#include "eixTk/stringutils.h"

using std::string;
using std::vector;

using std::cout;

void
OutputString::assign(const OutputString &s)
{
	m_string = s.m_string;
	m_size = s.m_size;
	m_insert = s.m_insert;
	absolute = s.absolute;
}

void
OutputString::assign(const string &t)
{
	m_string = t;
	m_size = t.size();
	absolute = false;
	m_insert.clear();
}

void
OutputString::assign(const string &t, string::size_type s)
{
	m_string = t;
	m_size = s;
	absolute = false;
	m_insert.clear();
}

void
OutputString::clear()
{
	m_string.clear();
	m_size = 0;
	absolute = false;
	m_insert.clear();
}

void
OutputString::set_one()
{
	m_string = "1";
	m_size = 1;
	absolute = false;
	m_insert.clear();
}

void
OutputString::append_column(string::size_type s)
{
	if(absolute) {
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

void
OutputString::append_escape(const char **pos)
{
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

void
OutputString::append(char s)
{
	m_string.append(1, s);
	++m_size;
}

void
OutputString::append_smart(char s)
{
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
			if(absolute) {
				m_size += 8 - (m_size % 8);
			} else {
				m_insert.push_back(m_string.size());
				m_insert.push_back(m_size);
				m_insert.push_back(0);
			}
			break;
		default:
			++m_size;;
			break;
	}
}

void
OutputString::append(const string &t)
{
	m_string.append(t);
	m_size += t.size();
}

void
OutputString::append(const string &t, string::size_type s)
{
	m_string.append(t);
	m_size += s;
}

void
OutputString::append(const OutputString &a)
{
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
		for(vector<string::size_type>::const_iterator it(a.m_insert.begin());
			unlikely(it != a.m_insert.end()); ++it) {
			m_insert.push_back((*it) + m_string.size());
			m_insert.push_back((*(++it)) + m_size);
			m_insert.push_back(*(++it));
		}
		m_size += a.m_size;
	}
	m_string.append(a.m_string);
}

void
OutputString::print(std::string *dest, std::string::size_type *s) const
{
	std::string::size_type inserted(0);
	if(likely(m_insert.empty())) {
		dest->append(m_string);
	} else {
		std::string::size_type r(0);
		std::string::size_type curr(*s);
		for(vector<string::size_type>::const_iterator it(m_insert.begin());
			it != m_insert.end(); ++it) {
			if(*it > r) {
				dest->append(m_string, r, (*it) - r);
				r = *it;
			}
			curr += *(++it);
			std::string::size_type aim(*(++it));
			std::string::size_type d;
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

void
OutputString::print(std::string::size_type *s) const
{
	string d;
	print(&d, s);
	cout << d;
}
