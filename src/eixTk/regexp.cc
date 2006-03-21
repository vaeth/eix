/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "regexp.h"

using namespace std;

Regex::~Regex()
{
	if(m_compiled)
		regfree(&m_re);
}

void
Regex::compile(const char *regex, int eflags)
{
	if(m_compiled) {
		regfree(&m_re);
		m_compiled = false;
	}

	int errcode = regcomp(&m_re, regex, eflags|REG_EXTENDED);
	if(errcode != 0) {
		fprintf(stderr, "regcomp(\"%s\"): %s\n", regex, get_error(errcode).c_str());
		exit(1);
	}

	m_compiled = true;
}

string
Regex::get_error(int code)
{
	char buf[512];
	regerror(code, static_cast<const regex_t*>(&m_re), buf, 511);
	return string(buf);
}

