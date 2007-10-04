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
#include <cstring>
#include <cstdlib>
#include <algorithm>

#if !defined HAVE_STRNDUP
/** strndup in case we don't have one. */
#include <unistd.h>
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

/** Resolve a vector of -/+ keywords and store the result as a set.
 * If we find a -keyword we look for a (+)keyword. If one ore more (+)keywords
 * are found, they (and the -keyword) are removed.
 * @param obsolete_minus   If true do not treat -* special and keep -keyword.
 * @param warnminus        Set if there was -keyword which did not apply for
 * @param warnignore
 * @param warn_plus        Warn if keywords begin with a '+'.
 * @return true            if -* is contained */
bool resolve_plus_minus(std::set<std::string> &s, const std::vector<std::string> &l, bool obsolete_minus, bool *warnminus = NULL, const std::set<std::string> *warnignore = NULL, bool warn_plus = true);

/** Sort and unique. Return true if there were double entries */
#ifdef UNIQUE_WORKS
template<typename T>
bool sort_uniquify(T &v, bool vector_is_ignored = false)
{
	std::sort(v.begin(), v.end());
	typename T::iterator i = std::unique(v.begin(), v.end());
	if(i == v.end())
		return false;
	if(! vector_is_ignored)
		v.erase(i, v.end());
	return true;
}

#else

template<typename T>
bool sort_uniquify(std::vector<T> &v, bool vector_is_ignored = false)
{
	std::set<T> s(v.begin(), v.end());
	if(! vector_is_ignored) {
		v.clear();
		v.insert(v.end(), s.begin(), s.end());
	}
	return (s.size() != v.size());
}

#endif

/// Add items from s to the end of d.
template<typename T>
void push_backs(std::vector<T> &d, const std::vector<T> &s)
{
	d.insert(d.end(), s.begin(), s.end());
}

/** Make a set from a vector. */
template<typename T>
void make_set(std::set<T> &the_set, const std::vector<T> &the_list)
{
	the_set.clear();
	the_set.insert(the_list.begin(), the_list.end());
}


/** Make a vector from a set. */
template<typename T>
void make_vector(std::vector<T> &the_list, const std::set<T> &the_set)
{
	the_list.clear();
	for(typename std::set<T>::const_iterator it(the_set.begin()),
			it_end(the_set.end());
		it != it_end; ++it) 
	{
		the_list.push_back(*it);
	}
}


#endif /* __STRINGUTILS_H__ */
