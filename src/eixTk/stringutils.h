// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_STRINGUTILS_H_
#define SRC_EIXTK_STRINGUTILS_H_

// include "eixTk/stringutils.h" This comment satisfies check_include script

#ifndef HAVE_STRNDUP
#include <sys/types.h>
/** strndup in case we don't have one. */
char *strndup(const char *s, size_t n) ATTRIBUTE_NONNULL_;
#endif

#include <locale>

#include <cstdlib>
#include <cstring>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/null.h"

#ifdef HAVE_STRTOUL
#define my_atoi(a) strtoul(a, NULLPTR, 10)
#else
#ifdef HAVE_STRTOL
#define my_atoi(a) strtol(a, NULLPTR, 10)
#else
#define my_atoi(a) atoi(a)
#endif
#endif

/** Spaces for split strings */
extern const char *spaces;

/** Necessary escapes for shell-like strings in "..." */
extern const char *doublequotes;

extern std::locale localeC;

/** Split names of Atoms in different ways. */
class ExplodeAtom {
	public:
		static const char *get_start_of_version(const char* str, bool allow_star) ATTRIBUTE_NONNULL_;

		/** Get the version-string of a Atom (e.g. get 1.2.3 from foobar-1.2.3).  */
		static char *split_version(const char* str) ATTRIBUTE_NONNULL_;

		/** Get the name-string of a Atom (e.g. get foobar from foobar-1.2.3).  */
		static char *split_name(const char* str) ATTRIBUTE_NONNULL_;

		/** Get name and version from a Atom (e.g. foobar and 1.2.3 from foobar-1.2.3).
		 * @warn You'll get a pointer to a static array of 2 pointer to char. */
		static char **split(const char* str) ATTRIBUTE_NONNULL_;
};

/** Check string if it only contains digits. */
bool is_numeric(const char *str) ATTRIBUTE_NONNULL_ ATTRIBUTE_PURE;

/** Add symbol if it is not already the last one */
void optional_append(std::string *s, char symbol) ATTRIBUTE_NONNULL_;

/** Trim characters on left side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed */
void ltrim(std::string *str, const char *delims = spaces) ATTRIBUTE_NONNULL_;

/** Trim characters on right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed */
void rtrim(std::string *str, const char *delims = spaces) ATTRIBUTE_NONNULL_;

/** Trim characters on left and right side of string.
 * @param str String that should be trimmed
 * @param delims characters that should me removed */
void trim(std::string *str, const char *delims = spaces) ATTRIBUTE_NONNULL_;

/** Replaces all characters of delims by c (counting several delims as one).
 * delims on the beginning of end of the strig are removed.
 * @param str String that should be trimmed
 * @param delims characters that should me removed
 * @param c character that should be inserted */
void trimall(std::string *str, const char *delims = spaces, char c = ' ') ATTRIBUTE_NONNULL_;

/** return the lowercase version of str */
std::string to_lower(const std::string &str);

/** return the character corresponding to an escaped symbol.
    For instance, n -> \n, \ -> \, \0 -> \ */
char get_escape(char c) ATTRIBUTE_CONST;

/** Resolve all escapes in a string (a safe printf) */
void unescape_string(std::string *str) ATTRIBUTE_NONNULL_;

/** Escape all "at" and "\" characters in string. */
void escape_string(std::string *str, const char *at = spaces) ATTRIBUTE_NONNULL_;

/** Split a string into multiple strings.
 * @param vec Will contain the result. Actually the result is pushed_back
 *            or inserted, respectively. So if you have no new vec,
 *            you must usually call vec.clear() in advance.
 * @param str Reference to the string that should be splitted.
 * @param at  Split at the occurrence of any these characters.
 * @param ignore_empty  Remove empty strings from the result.
 * @param handle_escape Do not split at escaped characters from "at" symbols,
 *                      removing escapes for \\ or "at" symbols from result. */
void split_string(std::vector<std::string> *vec, const std::string &str, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true) ATTRIBUTE_NONNULL_;
void split_string(std::set<std::string> *vec, const std::string &str, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true) ATTRIBUTE_NONNULL_;

/** Check if slot contains a subslot and if yes, split it away.
    Also turn slot "0" into nothing */
bool slot_subslot(std::string *slot, std::string *subslot) ATTRIBUTE_NONNULL_;

/** Split full to slot and subslot. Also turn slot "0" into nothing
 * @return true if subslot exists */
bool slot_subslot(const std::string &full, std::string *slot, std::string *subslot) ATTRIBUTE_NONNULL_;

/** Split a string into multiple strings.
 * @param str Reference to the string that should be splitted.
 * @param at  Split at the occurrence of any these characters.
 * @param ignore_empty  Remove empty strings from the result.
 * @param handle_escape Do not split at escaped characters from "at" symbols,
 *                      removing escapes for \\ or "at" symbols from result.
 * @return the resulting vector */
std::vector<std::string> split_string(const std::string &str, const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true) ATTRIBUTE_NONNULL_;

/** Push back to a vector */
template<typename T> inline static void push_back(std::vector<T> *v, const T &e) ATTRIBUTE_NONNULL_;

/** Push back (=insert) to a set */
template<typename T> inline static void push_back(std::set<T> *s, const T &e) ATTRIBUTE_NONNULL_;

/** Join a string-vector or string-set
 * @param glue glue between the elements. */
template<typename T> inline static std::string join_to_string(T vec, const std::string &glue = " ");

/** Add items from s to the end of d. */
template<typename T> inline static void push_backs(std::vector<T> *d, const std::vector<T> &s) ATTRIBUTE_NONNULL_;

/** Join a string-vector.
 * @param glue glue between the elements. */
void join_to_string(std::string *s, const std::vector<std::string> &vec, const std::string &glue = " ") ATTRIBUTE_NONNULL_;

/** Join a string-set
 * @param glue glue between the elements. */
void join_to_string(std::string *s, const std::set<std::string> &vec, const std::string &glue = " ") ATTRIBUTE_NONNULL_;

/** Calls split_string() with a vector and then join_to_string().
 * @param source string to split
 * @param dest   result. May be identical to source. */
void split_and_join(std::string *dest, const std::string &source, const std::string &glue = " ", const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true) ATTRIBUTE_NONNULL_;

/** Calls split_string() with a vector and then join_to_string().
 * @param source string to split
 * @return result. */
std::string split_and_join_string(const std::string &source, const std::string &glue = " ", const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true) ATTRIBUTE_NONNULL_;

/** Calls join_to_string() and then split_string() */
template<typename Td, typename Ts> inline static void join_and_split(Td vec, const Ts &s, const std::string &glue = " ", const bool handle_escape = false, const char *at = spaces, const bool ignore_empty = true);

/** Resolve a vector of -/+ keywords to a set of actually set keywords.
 * @param s will get influenced by the string; it is not cleared in advance!
 * @param warnignore       List of keywords for which -keywords might apply
 * @return true            if there was -keyword which did not apply for */
bool resolve_plus_minus(std::set<std::string> *s, const std::vector<std::string> &l, const std::set<std::string> *warnignore = NULLPTR) ATTRIBUTE_NONNULL((1));

/** Resolve a string of -/+ keywords to a set of actually set keywords */
bool resolve_plus_minus(std::set<std::string> *s, const std::string &str, const std::set<std::string> *warnignore = NULLPTR) ATTRIBUTE_NONNULL((1));

/** Insert a whole vector to a set. */
template<typename T> inline static void insert_list(std::set<T> *the_set, const std::vector<T> &the_list) ATTRIBUTE_NONNULL_;

/** Make a set from a vector. */
template<typename T> inline static void make_set(std::set<T> *the_set, const std::vector<T> &the_list) ATTRIBUTE_NONNULL_;

/** Make a vector from a set. */
template<typename T> inline static void make_vector(std::vector<T> *the_list, const std::set<T> &the_set) ATTRIBUTE_NONNULL_;

class StringHash : public std::vector<std::string>
{
	public:
		explicit StringHash(bool will_hash = true) : hashing(will_hash), finalized(false)
		{ }

		void init(bool will_hash)
		{
			hashing = will_hash;
			finalized = false;
			clear();
			str_map.clear();
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

		void output(const std::set<std::string> *s = NULLPTR) const;

		const std::string& operator[](StringHash::size_type i) const;
	private:
		bool hashing, finalized;
		std::map<std::string, StringHash::size_type> str_map;
};

// Implementation of the templates:

/** Push back to a vector */
template<typename T>
inline static void
push_back(std::vector<T> *v, const T &e)
{ v->push_back(e); }

/** Push back (=insert) to a set */
template<typename T>
inline static void
push_back(std::set<T> *s, const T &e)
{ s->insert(e); }

/** Join a string-vector or string-set
 * @param glue glue between the elements. */
template<typename T>
inline static std::string
join_to_string(T vec, const std::string &glue)
{
	std::string ret;
	join_to_string(&ret, vec, glue);
	return ret;
}

/** Calls join_to_string() and then split_string() */
template<typename Td, typename Ts>
inline static void
join_and_split(Td vec, const Ts &s, const std::string &glue, const bool handle_escape, const char *at, const bool ignore_empty)
{
	std::string t;
	join_to_string(&t, s, glue);
	split_string(vec, t, handle_escape, at, ignore_empty);
}

/** Add items from s to the end of d. */
template<typename T>
inline static void
push_backs(std::vector<T> *d, const std::vector<T> &s)
{
	d->insert(d->end(), s.begin(), s.end());
}

/** Insert a whole vector to a set. */
template<typename T>
inline static void
insert_list(std::set<T> *the_set, const std::vector<T> &the_list)
{
	the_set->insert(the_list.begin(), the_list.end());
}

/** Make a set from a vector. */
template<typename T>
inline static void
make_set(std::set<T> *the_set, const std::vector<T> &the_list)
{
	the_set->clear();
	insert_list(the_set, the_list);
}

/** Make a vector from a set. */
template<typename T>
inline static void
make_vector(std::vector<T> *the_list, const std::set<T> &the_set)
{
	the_list->clear();
	for(typename std::set<T>::const_iterator it(the_set.begin()),
			it_end(the_set.end());
		likely(it != it_end); ++it) {
		the_list->push_back(*it);
	}
}

#endif  // SRC_EIXTK_STRINGUTILS_H_
