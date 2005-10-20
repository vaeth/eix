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

#include <eixTk/exceptions.h>

#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
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

		Regex(const char *regex, int eflags = 0) throw(ExBasic) {
			compiled = false;
			compile(regex, eflags);
		}

		void compile(const char *regex, int eflags = 0) throw(ExBasic) {
			if(compiled) {
				regfree(&re);
			}
			compiled = false;
			if(regcomp(&re, regex, eflags) != 0) {
				throw(ExBasic("regcomp(): %s", strerror(errno)));
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
};

#endif
