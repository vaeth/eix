// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/stringutils.h"
#include <config.h>

#include <fnmatch.h>

#include <locale>

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/diagnostics.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

using std::locale;

const char *spaces(" \t\r\n");
const char *shellspecial(" \t\r\n\"'`${}()[]<>?*~;|&#");
const char *doublequotes("\"$\\");

StringHash *StringHash::comparison_this;

locale localeC("C");

ATTRIBUTE_NONNULL_ static void erase_escapes(string *s, const char *at);
template<typename S, typename T> ATTRIBUTE_PURE ATTRIBUTE_NONNULL_ inline static S calc_table_pos(const vector<S>& table, S pos, const T *pattern, T c);
template<typename T> ATTRIBUTE_NONNULL_ inline static void split_string_template(T *vec, const string& str, bool handle_escape, const char *at, bool ignore_empty);
template<typename T> ATTRIBUTE_NONNULL_ inline static void join_to_string_template(string *s, const T& vec, const string& glue);

/**
Check whether str contains only digits
**/
bool is_numeric(const char *str) {
	for(char c(*str); likely(c != '\0'); c = *(++str)) {
		if(!my_isdigit(c)) {
			return false;
		}
	}
	return true;
}

/**
Add symbol if it is not already the last one
**/
void optional_append(std::string *s, char symbol) {
	if(s->empty() || ((*(s->rbegin()) != symbol))) {
		s->append(1, symbol);
	}
}

/**
Trim characters on left side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
void ltrim(std::string *str, const char *delims) {
	// trim leading whitespace
	std::string::size_type notwhite(str->find_first_not_of(delims));
	if(notwhite != std::string::npos) {
		str->erase(0, notwhite);
	} else {
		str->clear();
	}
}

/**
Trim characters on right side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
void rtrim(std::string *str, const char *delims) {
	// trim trailing whitespace
	std::string::size_type notwhite(str->find_last_not_of(delims));
	if(notwhite != std::string::npos) {
		str->erase(notwhite+1);
	} else {
		str->clear();
	}
}

/**
Trim characters on left and right side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
void trim(string *str, const char *delims) {
	ltrim(str, delims);
	rtrim(str, delims);
}

/**
Trim characters on left and right side of string.
@param str String that should be trimmed
@param delims characters that should me removed
**/
void trimall(string *str, const char *delims, char c) {
	string::size_type pos(0);
	while(unlikely((pos = str->find_first_of(delims, pos)) != string::npos)) {
		string::size_type end(str->find_first_not_of(spaces, pos + 1));
		if(end == string::npos) {
			str->erase(pos);
			return;
		}
		if(pos != 0) {
			(*str)[pos] = c;
			if(++pos == end) {
				continue;
			}
		}
		str->erase(pos, end - pos);
	}
}

/**
Check if slot contains a subslot and if yes, split it away.
Also turn slot "0" into nothing
**/
bool slot_subslot(string *slot, string *subslot) {
	string::size_type sep(slot->find('/'));
	if(sep == string::npos) {
		subslot->clear();
		if((*slot) == "0") {
			slot->clear();
		}
		return false;
	}
	subslot->assign(*slot, sep + 1, string::npos);
	slot->resize(sep);
	if(*slot == "0") {
		slot->clear();
	}
	return true;
}

/**
Split full to slot and subslot. Also turn slot "0" into nothing
@return true if subslot exists
**/
bool slot_subslot(const string& full, string *slot, string *subslot) {
	string::size_type sep(full.find('/'));
	if(sep == string::npos) {
		if(full != "0") {
			*slot = full;
		} else {
			slot->clear();
		}
		subslot->clear();
		return false;
	}
	subslot->assign(full, sep + 1, string::npos);
	slot->assign(full, 0, sep);
	if((*slot) == "0") {
		slot->clear();
	}
	return true;
}

const char *ExplodeAtom::get_start_of_version(const char *str, bool allow_star) {
	// There must be at least one symbol before the version:
	if(unlikely(*(str++) == '\0')) {
		return NULLPTR;
	}
	const char *x(NULLPTR);
	while(likely((*str != '\0') && (*str != ':') && (*str != '['))) {
		if(unlikely(*str++ == '-')) {
			if(my_isdigit(*str) ||
				unlikely(allow_star && ((*str) == '*'))) {
				x = str;
			}
		}
	}
	return x;
}

bool ExplodeAtom::split_version(string *version, const char *str) {
	const char *x(get_start_of_version(str, false));
	if(unlikely(x == NULLPTR)) {
		return false;
	}
	version->assign(x);
	return true;
}

bool ExplodeAtom::split_name(string *name, const char *str) {
	const char *x(get_start_of_version(str, false));
	if(unlikely(x == NULLPTR)) {
		return false;
	}
GCC_DIAG_OFF(sign-conversion)
	name->assign(str, ((x - 1) - str));
GCC_DIAG_ON(sign-conversion)
	return true;
}

bool ExplodeAtom::split(string *name, string *version, const char *str) {
	const char *x(get_start_of_version(str, false));
	if(unlikely(x == NULLPTR)) {
		return false;
	}
	version->assign(x);
GCC_DIAG_OFF(sign-conversion)
	name->assign(str, ((x - 1) - str));
GCC_DIAG_ON(sign-conversion)
	return true;
}

string to_lower(const string& str) {
	string::size_type s(str.size());
	string res;
	for(string::size_type c(0); c != s; ++c) {
		res.append(1, my_tolower(str[c]));
	}
	return res;
}

char get_escape(char c) {
	switch(c) {
		case 0:
		case '\\': return '\\';
		case 'n':  return '\n';
		case 'r':  return '\r';
		case 't':  return '\t';
		case 'b':  return '\b';
		case 'a':  return '\a';
		default:
			break;
	}
	return c;
}

void unescape_string(string *str) {
	string::size_type pos(0);
	while(unlikely((pos = str->find('\\', pos)) != string::npos)) {
		string::size_type p(pos + 1);
		if(p == str->size())
			return;
		str->replace(pos, 2, 1, get_escape((*str)[p]));
	}
}

void escape_string(string *str, const char *at) {
	string my_at(at);
	my_at.append("\\");
	string::size_type pos(0);
	while(unlikely((pos = str->find_first_of(my_at, pos)) != string::npos)) {
		str->insert(pos, 1, '\\');
		pos += 2;
	}
}

static void erase_escapes(string *s, const char *at) {
	string::size_type pos(0);
	while((pos = s->find('\\', pos)) != string::npos) {
		++pos;
		if(pos == s->size()) {
			s->erase(pos - 1, 1);
			break;
		}
		char c((*s)[pos]);
		if((c == '\\') || (strchr(at, c) != NULLPTR))
			s->erase(pos - 1, 1);
	}
}

template<typename T> inline static void split_string_template(T *vec, const string& str, bool handle_escape, const char *at, bool ignore_empty) {
	string::size_type last_pos(0), pos(0);
	while((pos = str.find_first_of(at, pos)) != string::npos) {
		if(unlikely(handle_escape)) {
			bool escaped(false);
			string::size_type s(pos);
			while(s > 0) {
				if(str[--s] != '\\')
					break;
				escaped = !escaped;
			}
			if(escaped) {
				++pos;
				continue;
			}
			string r(str, last_pos, pos - last_pos);
			erase_escapes(&r, at);
			if(likely((!r.empty()) || !ignore_empty)) {
				push_back(vec, r);
			}
		} else if(likely((pos > last_pos) || !ignore_empty)) {
			push_back(vec, str.substr(last_pos, pos - last_pos));
		}
		last_pos = ++pos;
	}
	if(unlikely(handle_escape)) {
		string r(str, last_pos);
		erase_escapes(&r, at);
		if(likely((!r.empty()) || !ignore_empty)) {
			push_back(vec, r);
		}
	} else if(likely((str.size() > last_pos) || !ignore_empty)) {
		push_back(vec, str.substr(last_pos));
	}
}

void split_string(WordVec *vec, const string& str, bool handle_escape, const char *at, bool ignore_empty) {
	split_string_template<WordVec>(vec, str, handle_escape, at, ignore_empty);
}

void split_string(WordSet *vec, const string& str, bool handle_escape, const char *at, bool ignore_empty) {
	split_string_template<WordSet>(vec, str, handle_escape, at, ignore_empty);
}

WordVec split_string(const string& str, bool handle_escape, const char *at, bool ignore_empty) {
	WordVec vec;
	split_string(&vec, str, handle_escape, at, ignore_empty);
	return vec;
}

/**
Call split_string() with a vector and then join_to_string().
@param source string to split
@param dest   result. May be identical to source.
**/
void split_and_join(string *dest, const string& source, const string& glue, bool handle_escape, const char *at, bool ignore_empty) {
	WordVec vec;
	split_string(&vec, source, handle_escape, at, ignore_empty);
	join_to_string(dest, vec, glue);
}

/**
Call split_string() with a vector and then join_to_string().
@param source string to split
@return result.
**/
string split_and_join_string(const string& source, const string& glue, bool handle_escape, const char *at, bool ignore_empty) {
	string r;
	split_and_join(&r, source, glue, handle_escape, at, ignore_empty);
	return r;
}

/**
Resolve a string of -/+ keywords to a set of actually set keywords
**/
bool resolve_plus_minus(WordSet *s, const string& str, const WordSet *warnignore) {
	WordVec l;
	split_string(&l, str);
	return resolve_plus_minus(s, l, warnignore);
}

template<typename T> inline static void join_to_string_template(string *s, const T& vec, const string& glue) {
	for(typename T::const_iterator it(vec.begin()); likely(it != vec.end()); ++it) {
		if(likely(!s->empty())) {
			s->append(glue);
		}
		s->append(*it);
	}
}

void join_to_string(string *s, const WordVec& vec, const string& glue) {
	join_to_string_template<WordVec>(s, vec, glue);
}

void join_to_string(string *s, const WordSet& vec, const string& glue) {
	join_to_string_template<WordSet>(s, vec, glue);
}

bool resolve_plus_minus(WordSet *s, const WordVec& l, const WordSet *warnignore) {
	bool minuskeyword(false);
	for(WordVec::const_iterator it(l.begin()); likely(it != l.end()); ++it) {
		if(unlikely(it->empty())) {
			continue;
		}
		if(unlikely((*it)[0] == '+')) {
			cerr << eix::format(_("flags should not start with a '+': %s")) % *it
				<< endl;
			s->insert(it->substr(1));
			continue;
		}
		if(unlikely((*it)[0] == '-')) {
			if(*it == "-*") {
				s->clear();
				continue;
			}
			if(*it == "-~*") {
				WordVec v;
				make_vector(&v, *s);
				for(WordVec::const_iterator i(v.begin());
					unlikely(i != v.end()); ++i) {
					if((i->size() >=2) && ((*i)[0] == '~')) {
						s->erase(*i);
					}
				}
			}
			string key(*it, 1);
			if(s->erase(key)) {
				continue;
			}
			if(warnignore != NULLPTR) {
				if(warnignore->find(key) == warnignore->end()) {
					minuskeyword = true;
				}
			} else {
				minuskeyword = true;
			}
		}
		s->insert(*it);
	}
	return minuskeyword;
}

void StringHash::store_string(const string& s) {
	if(finalized) {
		cerr << _("internal error: Storing required after finalizing") << endl;
		exit(EXIT_FAILURE);
	}
	push_back(s);
}

void StringHash::hash_string(const string& s) {
	if(finalized) {
		cerr << _("internal error: Hashing required after finalizing") << endl;
		exit(EXIT_FAILURE);
	}
	if(!hashing) {
		cerr << _("internal error: Hashing required in non-hash mode") << endl;
		exit(EXIT_FAILURE);
	}
	// During hashing, we use str_map as a frequency counter to optimize
	StrSizeMap::iterator i(str_map.find(s));
	if(i != str_map.end()) {
		++(i->second);
	} else {
		str_map[s] = 0;
	}
}

void StringHash::store_words(const WordVec& v) {
	for(WordVec::const_iterator i(v.begin()); likely(i != v.end()); ++i) {
		store_string(*i);
	}
}

void StringHash::hash_words(const WordVec& v) {
	for(WordVec::const_iterator i(v.begin()); likely(i != v.end()); ++i) {
		hash_string(*i);
	}
}

StringHash::size_type StringHash::get_index(const string& s) const {
	if(!finalized) {
		cerr << _("internal error: Index required before sorting.") << endl;
		exit(EXIT_FAILURE);
	}
	StrSizeMap::const_iterator i(str_map.find(s));
	if(i == str_map.end()) {
		cerr << _("internal error: Trying to shortcut non-hashed string.") << endl;
		exit(EXIT_FAILURE);
	}
	return i->second;
}

const string& StringHash::operator[](StringHash::size_type i) const {
	if(i >= size()) {
		cerr << _("database corrupt: nonexistent hash required");
		exit(EXIT_FAILURE);
	}
	return WordVec::operator[](i);
}

void StringHash::output() const {
	for(WordVec::const_iterator i(begin()); likely(i != end()); ++i) {
		cout << *i << "\n";
	}
}

void StringHash::output_depends() const {
	WordSet out;
	for(WordVec::const_iterator i(begin()); likely(i != end()); ++i) {
		string::size_type q(i->find('"'));
		if(q == string::npos) {
			out.insert(*i);
			continue;
		}
		if(q == 0) {
			if(i->length() != 1) {
				out.insert(string(*i, 1));
			}
			continue;
		}
		out.insert(string(*i, 0, q));
		if(++q != i->length()) {
			out.insert(string(*i, q));
		}
	}
	for(WordSet::const_iterator i(out.begin()); likely(i != out.end()); ++i) {
		cout << *i << "\n";
	}
}

bool StringHash::frequency_comparison(const string a, const string b) {
	return ((comparison_this->str_map)[b] < (comparison_this->str_map)[a]);
}

void StringHash::finalize() {
	if(finalized) {
		return;
	}
	finalized = true;
	if(!hashing) {
		return;
	}
	make_vector(this, str_map);
	comparison_this = this;
	sort(begin(), end(), StringHash::frequency_comparison);
	// For get_index(), we use str_map as the index map
	size_type i(0);
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		str_map[*it] = i++;
	}
}

bool match_list(const char *const *str_list, const char *str) {
	if(str_list != NULLPTR) {
		while(likely(*str_list != NULLPTR)) {
			if(fnmatch(*(str_list++), str, 0) == 0) {
				return true;
			}
		}
	}
	return false;
}

bool is_valid_pkgpath(char c) {
	if(likely(my_isalnum(c))) {
		return true;
	}
	switch(c) {
		case '_':
		case '+':
		case '-':
		case '/':
			return true;
		default:
			return false;
	}
}

const char *first_alnum(const char *s) {
	for(char c(*s); likely(c != '\0'); c = *(++s)) {
		if(my_isalnum(c)) {
			return s;
		}
	}
	return s;
}

const char *first_not_alnum_or_ok(const char *s, const char *ok) {
	for(char c(*s); likely(c != '\0'); c = *(++s)) {
		if(unlikely((!my_isalnum(c)) && (strchr(ok, c) == NULLPTR))) {
			return s;
		}
	}
	return s;
}

/**
Match str against a lowercase pattern case-insensitively
**/
bool caseequal(const char *str, const char *pattern) {
	for(char c(*str); c != '\0'; c = *(++str)) {
		if(my_tolower(c) != *pattern) {
			return false;
		}
		++pattern;
	}
	return(*pattern == '\0');
}

/**
Subroutine for Knuth-Morris-Pratt algorithm
**/
template<typename S, typename T> inline static S calc_table_pos(const vector<S>& table, S pos, const T *pattern, T c) {
	while(pattern[pos] != c) {
		if(pos == 0) {
			return 0;
		}
		pos = table[pos];
	}
	return pos + 1;
}

/**
Check whether str contains a nonempty lowercase pattern case-insensitively
**/
bool casecontains(const char *str, const char *pattern) {
	// Knuth-Morris-Pratt algorithm
	typedef string::size_type IndexType;
	IndexType l(strlen(pattern));
	vector<IndexType> table(l, 0);
	IndexType pos(0);
	for(IndexType i(1); likely(i < l); ++i) {
		pos = table[i] = calc_table_pos(table, pos, pattern, pattern[i]);
	}
	pos = 0;
	for(char c(*str); likely(c != '\0'); c = *(++str)) {
		pos = calc_table_pos(table, pos, pattern, my_tolower(c));
		if(pos == l) {
			return true;
		}
	}
	return false;
}

string::size_type utf8size(const string &t, string::size_type begin, string::size_type end) {
	if(end == string::npos) {
		end = t.size();
	}
	string::size_type len(0);
	for(string::size_type i(begin); likely(i < end); ++i) {
		if(likely(isutf8firstbyte(t[i]))) {
			++len;
		}
	}
	return len;
}
