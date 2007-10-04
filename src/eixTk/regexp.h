// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __REGEXCLASS_H__
#define __REGEXCLASS_H__

#include <sys/types.h>
#include <regex.h>
#include <string>

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
		Regex(const char *regex, int eflags = REG_ICASE)
			: m_compiled(false)
		{ compile(regex, eflags); }

		/// Free the regular expression.
		~Regex();

		/// Compile a regular expression.
		void compile(const char *regex, int eflags = REG_ICASE);

		/// Does the regular expression match s?
		bool match(const char *s);

		/// Get regular expression error for a error-code.
		std::string get_error(int code);

	protected:
		/// Gets the internal regular expression structure.
		const regex_t *get()
		{ return &m_re; }

		/// The actual regular expression (GNU C Library).
		regex_t m_re;

		/// Is the regex already compiled and nonempty?
		bool m_compiled;
};

#endif
