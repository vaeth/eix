// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__REGEXCLASS_H__
#define EIX__REGEXCLASS_H__ 1

#include <eixTk/likely.h>

#include <iostream>
#include <string>

#include <cstddef>
#include <cstdlib>
#include <regex.h>

/// Handles regular expressions.
// It is normally used within global scope so that a regular expression doesn't
// have to be compiled with every instance of a class using it.

class Regex
{
	public:
		/// Initalize class.
		Regex()
			: m_compiled(false)
		{ }

		/// Initalize and compile regular expression.
		Regex(const char *regex, int eflags = REG_EXTENDED)
			: m_compiled(false)
		{ compile(regex, eflags); }

		/// Free the regular expression.
		~Regex() {
			if(m_compiled)
				regfree(&m_re);
		}

		/// Compile a regular expression.
		void compile(const char *regex, int eflags = REG_EXTENDED)
		{
			if(unlikely(m_compiled)) {
				regfree(&m_re);
				m_compiled = false;
			}
			if(! regex ||! regex[0])
				return;

			int retval(regcomp(&m_re, regex, eflags|REG_EXTENDED));
			if(unlikely(retval != 0)) {
				char buf[512];
				regerror(retval, &m_re, buf, 511);
				std::cerr << "regcomp(" << regex << "): " << buf << std::endl;
				exit(EXIT_FAILURE);
			}
			m_compiled = true;
		}

		/// Does the regular expression match s?
		bool match(const char *s) const
		{ return ! m_compiled || ! regexec(get(), s, 0, NULL, 0); }

		/// Does the regular expression match s? Get beginning/end
		bool match(const char *s, std::string::size_type *b, std::string::size_type *e) const
		{
			regmatch_t pmatch[1];
			if(!m_compiled) {
				if(b)
					*b = 0;
				if(e)
					*e = std::string::npos;
				return true;
			}
			if(regexec(get(), s, 1, pmatch, 0)) {
				if(b)
					*b = std::string::npos;
				if(e)
					*e = std::string::npos;
				return false;
			}
			if(b)
				*b = pmatch[0].rm_so;
			if(e)
				*e = pmatch[0].rm_eo;
			return true;
		}

		bool compiled() const
		{ return m_compiled; }

	protected:
		/// Gets the internal regular expression structure.
		const regex_t *get() const
		{ return &m_re; }

		/// The actual regular expression (GNU C Library).
		regex_t m_re;

		/// Is the regex already compiled and nonempty?
		bool m_compiled;
};

#endif
