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

#ifndef __STRINGUTILS_H__
#define __STRINGUTILS_H__

#include "../../config.h"

#include <string>
#include <vector>
#include <set>

#if !defined HAVE_STRNDUP
/** strndup in case we don't have one. */
char *strndup(const char *s, size_t n);
#endif /* !defined HAVE_STRNDUP */

/** Split names of Atoms in different ways. */
class ExplodeAtom {

	public:

		static const char *get_start_of_version(const char* str);

		/** Get the version-string of a Atom (e.g. get 1.2.3 from foobar-1.2.3).  */
		static char *split_version(const char* str);

		/** Get the name-string of a Atom (e.g. get foobar from foobar-1.2.3).  */
		static char *split_name(const char* str);

		/** Get name and version from a Atom (e.g. foobar and 1.2.3 from foobar-1.2.3).
		 * @warn You'll get a pointer to a static array of 2 pointer to char. */
		static char **split(const char* str);
};

/** Check string if it only contains digits. */
inline bool
is_numeric(const char *str)
{
	while( (*str) )
		if( !isdigit(*str++) )
			return false;
	return true;
}

/** Trim characters on left side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @return trimmed string */
inline void
ltrim(std::string *str, const char *delims = " \t\r\n")
{
	// trim leading whitespace
	std::string::size_type notwhite = str->find_first_not_of(delims);
	if(notwhite != std::string::npos)
		str->erase(0, notwhite);
	else
		str->clear();
}

/** Trim characters on right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @return trimmed string */
inline void
rtrim(std::string *str, const char *delims = " \t\r\n")
{
	// trim trailing whitespace
	std::string::size_type  notwhite = str->find_last_not_of(delims);
	if(notwhite != std::string::npos)
		str->erase(notwhite+1);
	else
		str->clear();
}

/** Trim characters on left and right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @return trimmed string */
inline void
trim(std::string *str, const char *delims = " \t\r\n")
{
	ltrim(str, delims);
	rtrim(str, delims);
}

/** Split a string into multiple strings.
 * @param str Reference to the string that should be splitted.
 * @param at  Split at the occurrence of any these characters.
 * @return    vector of strings. */
std::vector<std::string> split_string(const std::string &str, const char *at = " \t\r\n", bool ignore_empty = true, bool ignore_escaped = false, bool remove_escape = false);

/** Join a string-vector.
 * @param glue glue between the elements. */
std::string join_vector(const std::vector<std::string> &vec, std::string glue = " ");

/** Make a set from a string-vector. */
void make_set(std::set<std::string> *the_set, const std::vector<std::string> &the_list);

/** Resolve a vector of -/+ keywords.
 * If we find a -keyword we look for a (+)keyword. If one ore more (+)keywords
 * are found, they (and the -keyword) are removed.
 * @param warn_plus  If true, warn if keywords begin with a '+'.
 * @param order      If true, only remove keywords that come before the according -keyword.
 * @return           Reference to cleaned vector (it anyway the same vector you gave us). */
std::vector<std::string>& resolve_plus_minus(std::vector<std::string> &v, bool warn_plus = true, bool order = true);

#if defined(HAVE_ASPRINTF)
#define asprintf_stringarg(tmp, format, arg) asprintf(tmp, format, arg)
#else
int asprintf_stringarg(char **tmp, const char *format, const char *arg);
#endif

#endif /* __STRINGUTILS_H__ */
