// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/regexp.h"
#include <config.h>

#include <cstdlib>

#include <string>
#include <vector>

#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/formated.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"

using std::string;
using std::vector;

/**
Free the regular expression
**/
void Regex::clear() {
	if(m_compiled) {
		regfree(&m_re);
		m_compiled = false;
	}
}

/**
Compile a regular expression
**/
void Regex::compile(const char *regex, int eflags) {
	if(unlikely(m_compiled)) {
		regfree(&m_re);
		m_compiled = false;
	}
	if((regex == NULLPTR) || (regex[0] == '\0')) {
		return;
	}

	int retval(regcomp(&m_re, regex, eflags|REG_EXTENDED));
	if(unlikely(retval != 0)) {
		char buf[512];
		regerror(retval, &m_re, buf, 511);
		eix::say_error("regcomp(%s)") % buf;
		std::exit(EXIT_FAILURE);
	}
	m_compiled = true;
}

/**
@arg s string to match.
@return true if the regular expression matches
**/
bool Regex::match(const char *s) const {
	return (!m_compiled) || (!regexec(get(), s, 0, NULLPTR, 0));
}

/**
@arg s string to match
@arg b beginning of match
@arg e end of match
@return true if the regular expression matches
**/
bool Regex::match(const char *s, string::size_type *b, string::size_type *e) const {
	regmatch_t pmatch[1];
	if(!m_compiled) {
		if(likely(b != NULLPTR)) {
			*b = 0;
		}
		if(likely(e != NULLPTR)) {
			*e = string::npos;
		}
		return true;
	}
	if(regexec(get(), s, 1, pmatch, 0)) {
		if(likely(b != NULLPTR)) {
			*b = string::npos;
		}
		if(likely(e != NULLPTR)) {
			*e = string::npos;
		}
		return false;
	}
	if(likely(b != NULLPTR)) {
GCC_DIAG_OFF(sign-conversion)
		*b = pmatch[0].rm_so;
GCC_DIAG_ON(sign-conversion)
	}
	if(likely(e != NULLPTR)) {
GCC_DIAG_OFF(sign-conversion)
		*e = pmatch[0].rm_eo;
GCC_DIAG_ON(sign-conversion)
	}
	return true;
}

RegexList::RegexList(const string& stringlist) {
	WordVec l;
	split_string(&l, stringlist, true);
	for(WordVec::const_iterator it(l.begin());
		likely(it != l.end()); ++it) {
		reglist.PUSH_BACK(new Regex(it->c_str()));
	}
}

RegexList::~RegexList() {
	for(vector<Regex*>::iterator it(reglist.begin());
		likely(it != reglist.end()); ++it) {
		delete *it;
	}
}

bool RegexList::match(const char *str) {
	for(vector<Regex*>::const_iterator it(reglist.begin());
		likely(it != reglist.end()); ++it) {
		if((*it)->match(str)) {
			return true;
		}
	}
	return false;
}
