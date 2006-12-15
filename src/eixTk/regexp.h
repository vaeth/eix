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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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
