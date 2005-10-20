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

#include <eixTk/regexp.h> 

#include <string>
#include <vector>

using namespace std;

/** Split names of Atoms in different ways. */
class ExplodeAtom {
	public:

		/** Get the version-string of a Atom (e.g. get 1.2.3 from foobar-1.2.3).  */
		static char *getVersion(const char* filename);

		/** Get the name-string of a Atom (e.g. get foobar from foobar-1.2.3).  */
		static char *getName(const char* filename);
		
		/** Get name and version from a Atom (e.g. foobar and 1.2.3 from foobar-1.2.3).
		 * @warn You'll get a pointer to a static array of 2 pointer to char. */
		static char **getNameVersion(const char* filename);
};

/** Check string if it only contains digits. */
inline bool
is_numeric(char *str)
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
inline string
ltrim(string str, const char *delims = " \t\r\n")
{
	// trim leading whitespace
	string::size_type  notwhite = str.find_first_not_of(delims);
	if(notwhite != string::npos)
		str.erase(0,notwhite);
	return str;
}

/** Trim characters on right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @return trimmed string */
inline string 
rtrim(string str, const char *delims = " \t\r\n")
{
	// trim trailing whitespace
	string::size_type  notwhite = str.find_last_not_of(delims);
	if(notwhite != string::npos)
		str.erase(notwhite+1);
	return str;
}

/** Trim characters on left and right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @return trimmed string */
inline string
trim(string str, const char *delims = " \t\r\n")
{
	return rtrim(ltrim(str, delims), delims);
}

/** Split a string into multiple strings. 
 * @param str Reference to the string that should be splitted.
 * @param at  Split at the occurrence of any these characters.
 * @return    vector of strings. */
vector<string> split_string(const string &str, const char *at = " \t\r\n", bool ignore_empty = true);

/** Join a string-vector.
 * @param glue glue between the elements. */
string join_vector(vector<string> &vec, string glue = " ");

/** Resolve a vector of -/+ keywords.
 * If we find a -keyword we look for a (+)keyword. If one ore more (+)keywords
 * are found, they (and the -keyword) are removed.
 * @param warn_plus  If true, warn if keywords begin with a '+'.
 * @param order      If true, only remove keywords that come before the according -keyword. 
 * @return           Reference to cleaned vector (it anyway the same vector you gave us). */
vector<string>& resolve_plus_minus(vector<string> &v, bool warn_plus = true, bool order = true);

#endif /* __STRINGUTILS_H__ */
