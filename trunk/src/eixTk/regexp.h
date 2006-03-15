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

#ifndef __REGEXCLASS_H__
#define __REGEXCLASS_H__

#include <regex.h>
#include <string>

using namespace std;

/** Handles regular expressions.
 * It is normally used within global scope so that a regular expression doesn't
 * have to be compiled with every instance of a class using it.  */
class Regex {

	private:
		regex_t re;       /**< The actual regular expression (GNU C Library) */
		bool    compiled; /**< Is the regex already compiled? */

	public:
		/** Compile a regular expression. */
		Regex() {
			compiled = false;
		}

		Regex(const char *regex, int eflags = REG_ICASE) {
			compiled = false;
			compile(regex, eflags);
		}

		void compile(const char *regex, int eflags = REG_ICASE) {
			if(compiled) {
				regfree(&re);
			}
			compiled = false;
			int errcode = regcomp(&re, regex, eflags|REG_EXTENDED);
			if(errcode != 0) {
				fprintf(stderr, "regcomp(\"%s\"): %s\n", regex, get_error(errcode).c_str());
				exit(1);
			}
			compiled = true;
		}

		/** Free the regular expression. */
		~Regex() {
			if(compiled) {
				regfree(&re);
			}
		}

		/** Gets the internal regular expression structure. */
		const regex_t *get() {
			return (const regex_t *) &re;
		}

		string get_error(int code) {
			char buf[512];
			regerror(code, static_cast<const regex_t*>(&re), buf, 511);
			return string(buf);
		}
};

#endif
