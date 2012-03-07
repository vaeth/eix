// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__STRINGUTILS_H__
#define EIX__STRINGUTILS_H__ 1

// include <eixTk/stringutils.h> This comment satisfies check_include script

#include <config.h>
#include <eixTk/likely.h>

#include <locale>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdlib>
#include <cstring>

#ifndef HAVE_STRNDUP
#include <sys/types.h>
/** strndup in case we don't have one. */
char *strndup(const char *s, size_t n);
#endif

#ifdef HAVE_STRTOUL
#define my_atoi(a) strtoul(a, NULL, 10)
#else
#ifdef HAVE_STRTOL
#define my_atoi(a) strtol(a, NULL, 10)
#else
#define my_atoi(a) atoi(a)
#endif
#endif

/** Spaces for split strings */
extern const char *spaces;

/** Necessary escapes for shell-like strings in "..." */
extern const char *doublequotes;

extern const std::string emptystring, one, space;

extern std::locale localeC;

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
inline bool is_numeric(const char *str) ATTRIBUTE_PURE;
inline bool
is_numeric(const char *str)
{
	for(char c(*str); likely(c != '\0'); c = *(++str)) {
		if(!isdigit(c, localeC))
			return false;
	}
	return true;
}

/** Add symbol if it is not already the last one */
inline void
optional_append(std::string &s, char symbol)
{
	if(s.empty() || ((*(s.rbegin()) != symbol)))
		s.append(1, symbol);
}

/** Trim characters on left side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed */
inline void
ltrim(std::string &str, const char *delims = spaces)
{
	// trim leading whitespace
	std::string::size_type notwhite(str.find_first_not_of(delims));
	if(notwhite != std::string::npos)
		str.erase(0, notwhite);
	else
		str.clear();
}

/** Trim characters on right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed */
inline void
rtrim(std::string &str, const char *delims = spaces)
{
	// trim trailing whitespace
	std::string::size_type notwhite(str.find_last_not_of(delims));
	if(notwhite != std::string::npos)
		str.erase(notwhite+1);
	else
		str.clear();
}

/** Trim characters on left and right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed */
void
trim(std::string &str, const char *delims = spaces);

/** Replaces all characters of delims by c (counting several delims as one).
 * delims on the beginning of end of the strig are removed.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @param c character that should be inserted */
void
trimall(std::string &str, const char *delims = spaces, char c = ' ');

/** return the character corresponding to an escaped symbol.
    For instance, n -> \n, \ -> \, \0 -> \ */
char get_escape(char c) ATTRIBUTE_CONST;

/** Resolve all escapes in a string (a safe printf) */
void unescape_string(std::string &str);

/** Escape all "at" and "\" characters in string. */
void escape_string(std::string &str, const char *at = spaces);

/** Split a string into multiple strings.
 * @param vec Will contain the result. Actually the result is pushed_back
 *            or inserted, respectively. So if you have no new vec,
 *            you must usually call vec.clear() in advance.
 * @param str Reference to the string that should be splitted.
 * @param at  Split at the occurrence of any these characters.
 * @param ignore_empty  Remove empty strings from the result.
 * @param handle_escape Do not split at escaped characters from "at" symbols,
 *                      removing escapes for \\ or "at" symbols from result. */
void split_string(std::vector<std::string> &vec, const std::string &str, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true);
void split_string(std::set<std::string> &vec, const std::string &str, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true);

/** Split a string into multiple strings.
 * @param str Reference to the string that should be splitted.
 * @param at  Split at the occurrence of any these characters.
 * @param ignore_empty  Remove empty strings from the result.
 * @param handle_escape Do not split at escaped characters from "at" symbols,
 *                      removing escapes for \\ or "at" symbols from result.
 * @return the resulting vector */
inline std::vector<std::string>
split_string(const std::string &str, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true)
{
	std::vector<std::string> vec;
	split_string(vec, str, handle_escape, at, ignore_empty);
	return vec;
}

template<typename T>
void push_back(std::vector<T> &v, const T &e)
{ v.push_back(e); }

template<typename T>
void push_back(std::set<T> &s, const T &e)
{ s.insert(e); }

/** Join a string-vector.
 * @param glue glue between the elements. */
void join_to_string(std::string &s, const std::vector<std::string> &vec, const std::string &glue = space);

/** Join a string-set
 * @param glue glue between the elements. */
void join_to_string(std::string &s, const std::set<std::string> &vec, const std::string &glue = space);

/** Join a string-vector or string-set
 * @param glue glue between the elements. */
template<typename T>
inline std::string
join_to_string(T &vec, const std::string &glue = space)
{
	std::string ret;
	join_to_string(ret, vec, glue);
	return ret;
}

/** Calls split_string() with a vector and then join_to_string().
 * @param source string to split
 * @param dest   result. May be identical to source. */
inline void
split_and_join(std::string &dest, const std::string &source, const std::string &glue = space, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true)
{
	std::vector<std::string> vec;
	split_string(vec, source, handle_escape, at, ignore_empty);
	join_to_string(dest, vec, glue);
}

/** Calls split_string() with a vector and then join_to_string().
 * @param source string to split
 * @return result. */
inline std::string
split_and_join_string(const std::string &source, const std::string &glue = space, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true)
{
	std::string r;
	split_and_join(r, source, glue, handle_escape, at, ignore_empty);
	return r;
}

/** Calls join_to_string() and then split_string() */
template<typename Td, typename Ts>
void
join_and_split(Td &vec, const Ts &s, const std::string &glue = space, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true)
{
	std::string t;
	join_to_string(t, s, glue);
	split_string(vec, t, handle_escape, at, ignore_empty);
}

/** Resolve a vector of -/+ keywords to a set of actually set keywords.
 * @param s will get influenced by the string; it is not cleared in advance!
 * @param warnignore       List of keywords for which -keywords might apply
 * @return true            if there was -keyword which did not apply for */
bool
resolve_plus_minus(std::set<std::string> &s, const std::vector<std::string> &l, const std::set<std::string> *warnignore = NULL);

/// Add items from s to the end of d.
template<typename T>
void push_backs(std::vector<T> &d, const std::vector<T> &s)
{
	d.insert(d.end(), s.begin(), s.end());
}

/// Insert a whole vector to a set.
template<typename T>
void insert_list(std::set<T> &the_set, const std::vector<T> &the_list)
{
	the_set.insert(the_list.begin(), the_list.end());
}

/// Make a set from a vector.
template<typename T>
void make_set(std::set<T> &the_set, const std::vector<T> &the_list)
{
	the_set.clear();
	insert_list(the_set, the_list);
}


/** Make a vector from a set. */
template<typename T>
void make_vector(std::vector<T> &the_list, const std::set<T> &the_set)
{
	the_list.clear();
	for(typename std::set<T>::const_iterator it(the_set.begin()),
			it_end(the_set.end());
		likely(it != it_end); ++it) {
		the_list.push_back(*it);
	}
}

class StringHash : public std::vector<std::string>
{
	public:
		StringHash(bool will_hash = true) : hashing(will_hash), finalized(false)
		{ }

		void init(bool will_hash)
		{
			hashing = will_hash; finalized = false;
			clear(); str_map.clear();
		}

		void finalize();

		void store_string(const std::string &s);
		void store_words(const std::vector<std::string> &v);
		void store_words(const std::string &s)
		{ hash_words(split_string(s)); }

		void hash_string(const std::string &s);
		void hash_words(const std::vector<std::string> &v);
		void hash_words(const std::string &s)
		{ hash_words(split_string(s)); }

		StringHash::size_type get_index(const std::string &s) const;

		void output(const std::set<std::string> *s = NULL) const;

		const std::string& operator[](StringHash::size_type i) const;
	private:
		bool hashing, finalized;
		std::map<std::string, StringHash::size_type> str_map;
};

#endif /* EIX__STRINGUTILS_H__ */
