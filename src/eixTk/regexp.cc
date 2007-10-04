// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#include "regexp.h"
#include <cstdlib>

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

	if(!regex)
		return;
	if(!(*regex))
		return;

	int errcode = regcomp(&m_re, regex, eflags|REG_EXTENDED);
	if(errcode != 0) {
		fprintf(stderr, "regcomp(\"%s\"): %s\n", regex, get_error(errcode).c_str());
		exit(1);
	}

	m_compiled = true;
}

bool
Regex::match(const char *s)
{
	// empty or uncompiled regex matches always:
	if(!m_compiled)
		return true;
	return !regexec(get(), s, 0, NULL, 0);
}

string
Regex::get_error(int code)
{
	char buf[512];
	regerror(code, static_cast<const regex_t*>(&m_re), buf, 511);
	return string(buf);
}

