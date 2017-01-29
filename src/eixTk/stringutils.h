// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_STRINGUTILS_H_
#define SRC_EIXTK_STRINGUTILS_H_

#include <config.h>

#include <locale>

#include <cstdlib>
#include <cstring>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"

// check_includes: include "eixTk/stringutils.h" strlen

#ifdef HAVE_STRTOUL
#define my_atoi(a) strtoul((a), NULLPTR, 10)
#else
#ifdef HAVE_STRTOL
#define my_atoi(a) strtol((a), NULLPTR, 10)
#else
#define my_atoi(a) atoi(a)
#endif
#endif
#ifdef HAVE_STRTOL
#define my_atois(a) strtol((a), NULLPTR, 10)
#else
#define my_atois(a) atoi(a)
#endif

#define my_isalnum(a) isalnum((a), localeC)
#define my_isalpha(a) isalpha((a), localeC)
#define my_isdigit(a) isdigit((a), localeC)
#define my_isspace(a) isspace((a), localeC)
#define my_tolower(a) tolower((a), localeC)
#define my_toupper(a) toupper((a), localeC)

/**
Spaces for split strings
**/
extern const char *spaces;

extern const char *shellspecial;

/**
Necessary escapes for shell-like strings in "..."
**/
extern const char *doublequotes;

extern std::locale localeC;

/**
Split names of Atoms in different ways.
**/
class ExplodeAtom {
	public:
		ATTRIBUTE_NONNULL_ static const char *get_start_of_version(const char* str, bool allow_star);

		/**
		Get the version-string of a Atom (e.g. get 1.2.3 from foobar-1.2.3).
		**/
		ATTRIBUTE_NONNULL_ static bool split_version(std::string *version, const char* str);

		/**
		Get the name-string of a Atom (e.g. get foobar from foobar-1.2.3).
		**/
		ATTRIBUTE_NONNULL_ static bool split_name(std::string *name, const char* str);

		/**
		Get name and version from a Atom (e.g. foobar and 1.2.3 from foobar-1.2.3).
		@warn If successfull, the result is stored in ExplodeAtom::split_name and
		must be freed with ExplodeAtom::free_split() before later calls.
		**/
		ATTRIBUTE_NONNULL_ static bool split(std::string *name, std::string *version, const char* str);
};

/**
Check whether str contains only digits
**/
ATTRIBUTE_NONNULL_ ATTRIBUTE_PURE bool is_numeric(const char *str);

/**
Check if char is character valid in "category/name" (including /)
**/
bool is_valid_pkgpath(char c);

/**
@return pointer to first alphanumeric or to 0 symbol
**/
const char *first_alnum(const char *s);

/**
@return pointer to the first non-alphanumeric not in ok, maybe to 0 symbol
**/
const char *first_not_alnum_or_ok(const char *s, const char *ok);

/**
Add symbol if it is not already the last one
**/
ATTRIBUTE_NONNULL_ void optional_append(std::string *s, char symbol);

/**
Trim characters on left side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
ATTRIBUTE_NONNULL_ void ltrim(std::string *str, const char *delims);

/**
Trim characters on right side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
ATTRIBUTE_NONNULL_ void rtrim(std::string *str, const char *delims);

/**
Trim characters on left and right side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
ATTRIBUTE_NONNULL_ void trim(std::string *str, const char *delims);
ATTRIBUTE_NONNULL_ inline static void trim(std::string *str);
inline static void trim(std::string *str) {
	trim(str, spaces);
}

/**
Replace all characters of delims by c (counting several delims as one).
delims on the beginning of end of the string are removed.
@param str String that should be trimmed
@param delims characters that should me removed
@param c character that should be inserted
**/
ATTRIBUTE_NONNULL_ void trimall(std::string *str, const char *delims, char c);
ATTRIBUTE_NONNULL_ inline static void trimall(std::string *str);
inline static void trimall(std::string *str) {
	trimall(str, spaces, ' ');
}

/**
@return the lowercase version of str
**/
std::string to_lower(const std::string& str);

/**
@return the character corresponding to an escaped symbol.
For instance, n -> \n, \ -> \, \0 -> \
**/
ATTRIBUTE_CONST char get_escape(char c);

/**
Resolve all escapes in a string (a safe printf)
**/
ATTRIBUTE_NONNULL_ void unescape_string(std::string *str);

/**
Escape all "at" and "\" characters in string
**/
ATTRIBUTE_NONNULL_ void escape_string(std::string *str, const char *at);
ATTRIBUTE_NONNULL_ inline static void escape_string(std::string *str);
inline static void escape_string(std::string *str) {
	escape_string(str, spaces);
}

/**
Split a string into multiple strings.
@param vec Will contain the result. Actually the result is pushed_back
           or inserted, respectively. So if you have no new vec,
           you must usually call vec.clear() in advance.
@param str Reference to the string that should be splitted.
@param at  Split at the occurrence of any these characters.
@param ignore_empty  Remove empty strings from the result.
@param handle_escape Do not split at escaped characters from "at" symbols,
                     removing escapes for \\ or "at" symbols from result.
**/
ATTRIBUTE_NONNULL_ void split_string(WordVec *vec, const std::string& str, bool handle_escape, const char *at, bool ignore_empty);
ATTRIBUTE_NONNULL_ void split_string(WordSet *vec, const std::string& str, bool handle_escape, const char *at, bool ignore_empty);
ATTRIBUTE_NONNULL_ inline static void split_string(WordVec *vec, const std::string& str, bool handle_escape, const char *at);
inline static void split_string(WordVec *vec, const std::string& str, bool handle_escape, const char *at) {
	split_string(vec, str, handle_escape, at, true);
}
ATTRIBUTE_NONNULL_ inline static void split_string(WordSet *vec, const std::string& str, bool handle_escape, const char *at);
inline static void split_string(WordSet *vec, const std::string& str, bool handle_escape, const char *at) {
	split_string(vec, str, handle_escape, at, true);
}
ATTRIBUTE_NONNULL_ inline static void split_string(WordVec *vec, const std::string& str, bool handle_escape);
inline static void split_string(WordVec *vec, const std::string& str, bool handle_escape) {
	split_string(vec, str, handle_escape, spaces);
}
ATTRIBUTE_NONNULL_ inline static void split_string(WordSet *vec, const std::string& str, bool handle_escape);
inline static void split_string(WordSet *vec, const std::string& str, bool handle_escape) {
	split_string(vec, str, handle_escape, spaces);
}
ATTRIBUTE_NONNULL_ inline static void split_string(WordVec *vec, const std::string& str);
inline static void split_string(WordVec *vec, const std::string& str) {
	split_string(vec, str, false);
}
ATTRIBUTE_NONNULL_ inline static void split_string(WordSet *vec, const std::string& str);
inline static void split_string(WordSet *vec, const std::string& str) {
	split_string(vec, str, false);
}

/**
Check if slot contains a subslot and if yes, split it away.
Also turn slot "0" into nothing
**/
ATTRIBUTE_NONNULL_ bool slot_subslot(std::string *slot, std::string *subslot);

/**
Split full to slot and subslot. Also turn slot "0" into nothing
@return true if subslot exists
**/
ATTRIBUTE_NONNULL_ bool slot_subslot(const std::string& full, std::string *slot, std::string *subslot);

/**
Split a string into multiple strings.
@param str Reference to the string that should be splitted.
@param at  Split at the occurrence of any these characters.
@param ignore_empty  Remove empty strings from the result.
@param handle_escape Do not split at escaped characters from "at" symbols,
                     removing escapes for \\ or "at" symbols from result.
@return the resulting vector
**/
ATTRIBUTE_NONNULL_ WordVec split_string(const std::string& str, bool handle_escape, const char *at, bool ignore_empty);
ATTRIBUTE_NONNULL_ inline static WordVec split_string(const std::string& str, bool handle_escape, const char *at);
inline static WordVec split_string(const std::string& str, bool handle_escape, const char *at) {
	return split_string(str, handle_escape, at, true);
}
inline static WordVec split_string(const std::string& str, bool handle_escape);
inline static WordVec split_string(const std::string& str, bool handle_escape) {
	return split_string(str, handle_escape, spaces);
}
inline static WordVec split_string(const std::string& str);
inline static WordVec split_string(const std::string& str) {
	return split_string(str, false);
}

/**
Push back to a vector
**/
template<typename T> ATTRIBUTE_NONNULL_ inline static void push_back(std::vector<T> *v, const T& e);

/**
Push back (=insert) to a set
**/
template<typename T> ATTRIBUTE_NONNULL_ inline static void push_back(std::set<T> *s, const T& e);

/**
Join a string-vector or string-set
@param glue glue between the elements
**/
template<typename T> inline static std::string join_to_string(T vec, const std::string& glue);
template<typename T> inline static std::string join_to_string(T vec) {
	return join_to_string(vec, " ");
}

/**
Add items from s to the end of d
**/
template<typename T> ATTRIBUTE_NONNULL_ inline static void push_backs(std::vector<T> *d, const std::vector<T>& s);

/**
Join a string-vector
@param glue glue between the elements.
**/
ATTRIBUTE_NONNULL_ void join_to_string(std::string *s, const WordVec& vec, const std::string& glue);
ATTRIBUTE_NONNULL_ inline static void join_to_string(std::string *s, const WordVec& vec);
inline static void join_to_string(std::string *s, const WordVec& vec) {
	join_to_string(s, vec, " ");
}

/**
Join a string-set
@param glue glue between the elements
**/
ATTRIBUTE_NONNULL_ void join_to_string(std::string *s, const WordSet& vec, const std::string& glue);
ATTRIBUTE_NONNULL_ inline static void join_to_string(std::string *s, const WordSet& vec);
inline static void join_to_string(std::string *s, const WordSet& vec) {
	join_to_string(s, vec, " ");
}

/**
Calls split_string() with a vector and then join_to_string().
@param source string to split
@param dest   result. May be identical to source
**/
ATTRIBUTE_NONNULL_ void split_and_join(std::string *dest, const std::string& source, const std::string& glue, bool handle_escape, const char *at, bool ignore_empty);
ATTRIBUTE_NONNULL_ inline static void split_and_join(std::string *dest, const std::string& source);
inline static void split_and_join(std::string *dest, const std::string& source) {
	split_and_join(dest, source, " ", false, spaces, true);
}

/**
Call split_string() with a vector and then join_to_string().
@param source string to split
@return result
**/
ATTRIBUTE_NONNULL_ std::string split_and_join_string(const std::string& source, const std::string& glue, bool handle_escape, const char *at, bool ignore_empty);
inline static std::string split_and_join_string(const std::string& source);
inline static std::string split_and_join_string(const std::string& source) {
	return split_and_join_string(source, " ", false, spaces, true);
}

/**
Call join_to_string() and then split_string()
**/
template<typename Td, typename Ts> inline static void join_and_split(Td vec, const Ts& s, const std::string& glue, bool handle_escape, const char *at, bool ignore_empty);
template<typename Td, typename Ts> inline static void join_and_split(Td vec, const Ts& s) {
	join_and_split(vec, s, " ", false, spaces, true);
}

/**
Resolve a vector of -/+ keywords to a set of actually set keywords.
@param s will get influenced by the string; it is not cleared in advance!
@param warnignore List of keywords for which -keywords might apply
@return true      if there was -keyword which did not apply for
**/
ATTRIBUTE_NONNULL((1)) bool resolve_plus_minus(WordSet *s, const WordVec& l, const WordSet *warnignore);
ATTRIBUTE_NONNULL_ inline static bool resolve_plus_minus(WordSet *s, const WordVec& l);
inline static bool resolve_plus_minus(WordSet *s, const WordVec& l) {
	return resolve_plus_minus(s, l, NULLPTR);
}

/**
Resolve a string of -/+ keywords to a set of actually set keywords
**/
ATTRIBUTE_NONNULL((1)) bool resolve_plus_minus(WordSet *s, const std::string& str, const WordSet *warnignore);
ATTRIBUTE_NONNULL_ inline static bool resolve_plus_minus(WordSet *s, const std::string& str);
inline static bool resolve_plus_minus(WordSet *s, const std::string& str) {
	return resolve_plus_minus(s, str, NULLPTR);
}

/**
Insert a whole vector to a set
**/
template<typename T> ATTRIBUTE_NONNULL_ inline static void insert_list(std::set<T> *the_set, const std::vector<T>& the_list);

/**
Make a set from a vector
**/
template<typename T> ATTRIBUTE_NONNULL_ inline static void make_set(std::set<T> *the_set, const std::vector<T>& the_list);

/**
Make a vector from a set
**/
template<typename T> ATTRIBUTE_NONNULL_ inline static void make_vector(std::vector<T> *the_list, const std::set<T>& the_set);

/**
Match str against a null-terminated list of patterns
**/
bool match_list(const char *const *str_list, const char *str);

/**
Match str against a lowercase pattern case-insensitively
**/
ATTRIBUTE_NONNULL_ bool caseequal(const char *str, const char *pattern);

/**
Match str against a lowercase pattern case-insensitively
**/
ATTRIBUTE_NONNULL_ inline static bool caseequal(const std::string& str, const char *pattern);
inline static bool caseequal(const std::string& str, const char *pattern) {
	return caseequal(str.c_str(), pattern);
}

/**
Check whether str contains a nonempty lowercase pattern case-insensitively
**/
ATTRIBUTE_NONNULL_ bool casecontains(const char *str, const char *pattern);

/**
Check whether str contains a nonempty lowercase pattern case-insensitively
**/
ATTRIBUTE_NONNULL_ inline static bool casecontains(const std::string& str, const char *pattern);
inline static bool casecontains(const std::string& str, const char *pattern) {
	return casecontains(str.c_str(), pattern);
}

/**
Check whether char is utf8 first-byte
**/
ATTRIBUTE_PURE inline bool isutf8firstbyte(char c);
inline bool isutf8firstbyte(char c) {
	return ((c & 0xC0) != 0x80);
}

/**
Calculate size of utf8 string
**/
ATTRIBUTE_PURE std::string::size_type utf8size(const std::string& t, std::string::size_type begin, std::string::size_type end);
ATTRIBUTE_PURE inline static std::string::size_type utf8size(const std::string &t, std::string::size_type begin);
inline static std::string::size_type utf8size(const std::string &t, std::string::size_type begin) {
	return utf8size(t, begin, std::string::npos);
}
ATTRIBUTE_PURE inline static std::string::size_type utf8size(const std::string &t);
inline static std::string::size_type utf8size(const std::string &t) {
	return utf8size(t, 0, std::string::npos);
}

class StringHash : public WordVec {
	public:
		StringHash() : hashing(true), finalized(false) {
		}

		explicit StringHash(bool will_hash) : hashing(will_hash), finalized(false) {
		}

		void init(bool will_hash) {
			hashing = will_hash;
			finalized = false;
			clear();
			str_map.clear();
		}

		void finalize();

		void store_string(const std::string& s);
		void store_words(const WordVec& v);
		void store_words(const std::string& s) {
			store_words(split_string(s));
		}

		void hash_string(const std::string& s);
		void hash_words(const WordVec& v);
		void hash_words(const std::string& s) {
			hash_words(split_string(s));
		}

		StringHash::size_type get_index(const std::string& s) const;

		void output() const;
		void output_depends() const;

		const std::string& operator[](StringHash::size_type i) const;

	private:
		bool hashing, finalized;
		typedef std::map<std::string, StringHash::size_type> StrSizeMap;
		StrSizeMap str_map;
		static StringHash *comparison_this;
		static bool frequency_comparison(const std::string a, const std::string b);
};

// Implementation of the templates:

/**
Push back to a vector
**/
template<typename T> inline static void push_back(std::vector<T> *v, const T& e) {
	v->push_back(e);
}

/**
Push back (=insert) to a set
**/
template<typename T> inline static void push_back(std::set<T> *s, const T& e) {
	s->insert(e);
}

/**
Join a string-vector or string-set
@param glue glue between the elements
**/
template<typename T> inline static std::string join_to_string(T vec, const std::string& glue) {
	std::string ret;
	join_to_string(&ret, vec, glue);
	return ret;
}

/**
Calls join_to_string() and then split_string()
**/
template<typename Td, typename Ts> inline static void join_and_split(Td vec, const Ts& s, const std::string& glue, bool handle_escape, const char *at, bool ignore_empty) {
	std::string t;
	join_to_string(&t, s, glue);
	split_string(vec, t, handle_escape, at, ignore_empty);
}

/**
Add items from s to the end of d
**/
template<typename T> inline static void push_backs(std::vector<T> *d, const std::vector<T>& s) {
	d->insert(d->end(), s.begin(), s.end());
}

/**
Insert a whole vector to a set
**/
template<typename T> inline static void insert_list(std::set<T> *the_set, const std::vector<T>& the_list) {
	the_set->insert(the_list.begin(), the_list.end());
}

/**
Make a set from a vector
**/
template<typename T> inline static void make_set(std::set<T> *the_set, const std::vector<T>& the_list) {
	the_set->clear();
	insert_list(the_set, the_list);
}

/**
Make a vector from a set
**/
template<typename T> inline static void make_vector(std::vector<T> *the_list, const std::set<T>& the_set) {
	the_list->clear();
	the_list->reserve(the_set.size());
	the_list->insert(the_list->end(), the_set.begin(), the_set.end());
}

/**
Make a vector from map->first
**/
template<typename T, typename Y> inline static void make_vector(std::vector<T> *the_list, const std::map<T, Y>& the_map) {
	the_list->clear();
	the_list->reserve(the_map.size());
	for(typename std::map<T, Y>::const_iterator it(the_map.begin()),
			it_end(the_map.end());
		likely(it != it_end); ++it) {
		the_list->push_back(it->first);
	}
}

#endif  // SRC_EIXTK_STRINGUTILS_H_
