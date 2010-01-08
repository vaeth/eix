// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "stringutils.h"

#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>

#include <iostream>
#include <locale>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifndef HAVE_STRNDUP
#include <sys/types.h>
#endif

using namespace std;

const char *spaces(" \t\r\n");

const string emptystring(""), one("1"), space(" ");

locale localeC("C");

/* strndup() is a GNU extension
 * However, we do not #define _GNU_SOURCE but instead make sure to
 * #include <config.h> (at least implicitly) */

#ifndef HAVE_STRNDUP
/* If we don't have strndup, we use our own ..
 * darwin (macos) doesn't have strndup, it's a GNU extension
 * See http://bugs.gentoo.org/show_bug.cgi?id=111912 */

char *
strndup(const char *s, size_t n)
{
	char *r(NULL);
	const char *p(s);
	while(likely(*p++ && n--));
	n = p - s - 1;
	r = static_cast<char *>(malloc(n + 1));
	if(r) {
		memcpy(r, s, n);
		r[n] = 0;
	}
	return r;
}
#endif /* HAVE_STRNDUP */

const char *
ExplodeAtom::get_start_of_version(const char* str)
{
	const char *x(NULL);
	// There must be at least one symbol before the version:
	if(unlikely(*(str++) == '\0'))
		return NULL;
	while(likely((*str != '\0') && (*str != ':'))) {
		if(unlikely((*str++ == '-') && (isdigit(*str, localeC))))
			x = str;
	}
	return x;
}

char *
ExplodeAtom::split_version(const char* str)
{
	const char *x(get_start_of_version(str));
	if(likely(x != NULL))
		return strdup(x);
	return NULL;
}

char *
ExplodeAtom::split_name(const char* str)
{
	const char *x(get_start_of_version(str));
	if(likely(x != NULL))
		return strndup(str, ((x - 1) - str));
	return NULL;
}

char **
ExplodeAtom::split(const char* str)
{
	static char* out[2] = { NULL, NULL };
	const char *x(get_start_of_version(str));

	if(unlikely(x == NULL))
		return NULL;
	out[0] = strndup(str, ((x - 1) - str));
	out[1] = strdup(x);
	return out;
}

char
get_escape(char c)
{
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

void
unescape_string(string &str)
{
	string::size_type pos(0);
	while(unlikely((pos = str.find_first_of('\\', pos)) != string::npos)) {
		string::size_type p(pos + 1);
		if(p == str.size())
			return;
		str.replace(pos, 2, 1, get_escape(str[p]));
	}
}

void
escape_string(string &str, const char *at)
{
	string my_at(at);
	my_at.append("\\");
	string::size_type pos(0);
	while(unlikely((pos = str.find_first_of(my_at, pos)) != string::npos)) {
		str.insert(pos, 1, '\\');
		pos += 2;
	}
}

inline static void
erase_escapes(string &s, const char *at)
{
	string::size_type pos(0);
	while((pos = s.find('\\', pos)) != string::npos) {
		++pos;
		if(pos == s.size()) {
			s.erase(pos - 1, 1);
			break;
		}
		char c(s[pos]);
		if((c == '\\') || (strchr(at, c) != NULL))
			s.erase(pos - 1, 1);
	}
}

template <typename T>
void
split_string_template(T &vec, const string &str, const bool handle_escape, const char *at, const bool ignore_empty)
{
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
			erase_escapes(r, at);
			if(likely((!r.empty()) || !ignore_empty))
				push_back(vec, r);
		}
		else if(likely((pos > last_pos) || !ignore_empty))
			push_back(vec, str.substr(last_pos, pos - last_pos));
		last_pos = ++pos;
	}
	if(unlikely(handle_escape)) {
		string r(str, last_pos);
		erase_escapes(r, at);
		if(likely((!r.empty()) || !ignore_empty))
			push_back(vec, r);
	}
	else if(likely((str.size() > last_pos) || !ignore_empty))
		push_back(vec, str.substr(last_pos));
}

void
split_string(vector<string> &vec, const string &str, const bool handle_escape, const char *at, const bool ignore_empty)
{ split_string_template< vector<string> >(vec, str, handle_escape, at, ignore_empty); }

void
split_string(set<string> &vec, const string &str, const bool handle_escape, const char *at, const bool ignore_empty)
{ split_string_template< set<string> >(vec, str, handle_escape, at, ignore_empty); }

template <typename T>
void
join_to_string_template(string &s, const T &vec, const string &glue)
{
	for(typename T::const_iterator it(vec.begin()); likely(it != vec.end()); ++it) {
		if(likely(!s.empty())) {
			s.append(glue);
		}
		s.append(*it);
	}
}

void
join_to_string(string &s, const vector<string> &vec, const string &glue)
{ join_to_string_template< vector<string> >(s, vec, glue); }

void
join_to_string(string &s, const set<string> &vec, const string &glue)
{ join_to_string_template< set<string> >(s, vec, glue); }

bool
resolve_plus_minus(set<string> &s, const vector<string> &l, bool obsolete_minus, bool *warnminus, const set<string> *warnignore)
{
	bool minusasterisk(false);
	bool minuskeyword(false);
	for(vector<string>::const_iterator it(l.begin()); likely(it != l.end()); ++it) {
		if(unlikely(it->empty()))
			continue;
		if(unlikely((*it)[0] == '+')) {
			cerr << eix::format(_("flags should not start with a '+': %s")) % *it
				<< endl;
			s.insert(it->substr(1));
			continue;
		}
		if(unlikely((*it)[0] == '-')) {
			if(*it == "-*") {
				minusasterisk = true;
				if(!obsolete_minus) {
					s.clear();
					continue;
				}
			}
			string key(*it, 1);
			if(s.erase(key))
				continue;
			if(warnignore) {
				if(warnignore->find(key) == warnignore->end())
					minuskeyword = true;
			}
			else
				minuskeyword = true;
		}
		s.insert(*it);
	}
	if(warnminus)
		*warnminus = minuskeyword;
	return minusasterisk;
}

void
StringHash::store_string(const string &s)
{
	if(finalized) {
		fprintf(stderr, _("Internal error: Storing required after finalizing"));
		exit(-1);
	}
	push_back(s);
}

void
StringHash::hash_string(const string &s)
{
	if(finalized) {
		fprintf(stderr, _("Internal error: Hashing required after finalizing"));
		exit(-1);
	}
	if(!hashing) {
		fprintf(stderr, _("Internal error: Hashing required in non-hash mode"));
		exit(-1);
	}
	map<string, StringHash::size_type>::const_iterator i(str_map.find(s));
	if(i != str_map.end())
		return;
	// For the moment, use str_map only as a set: Wait for finalize()
	str_map[s] = 0;//size();
	//store_string(s);
}

void
StringHash::store_words(const vector<string> &v)
{
	for(vector<string>::const_iterator i(v.begin()); likely(i != v.end()); ++i) {
		store_string(*i);
	}
}

void
StringHash::hash_words(const vector<string> &v)
{
	for(vector<string>::const_iterator i(v.begin()); likely(i != v.end()); ++i)
		hash_string(*i);
}

StringHash::size_type
StringHash::get_index(const string &s) const
{
	if(!finalized) {
		cerr << _("Internal error: Index required before sorting.") << endl;
		exit(2);
	}
	map<string, StringHash::size_type>::const_iterator i = str_map.find(s);
	if(i == str_map.end()) {
		cerr << _("Internal error: Trying to shortcut non-hashed string.") << endl;
		exit(2);
	}
	return i->second;
}

const string&
StringHash::operator[](StringHash::size_type i) const
{
	if(i >= size()) {
		cerr << _("Database corrupt: Nonexistent hash required");
		exit(2);
	}
	return vector<string>::operator[](i);
}

void
StringHash::output(const char *s) const
{
	if(unlikely(s != NULL))
		cout << s << ":\n";
	for(vector<string>::const_iterator i(begin()); likely(i != end()); ++i) {
		cout << *i << "\n";
	}
}

void
StringHash::finalize()
{
	if(finalized)
		return;
	finalized = true;
	if(!hashing)
		return;
	clear();
	for(map<string, size_type>::iterator it(str_map.begin());
		likely(it != str_map.end()); ++it) {
		it->second = size();
		push_back(it->first);
	}
}
