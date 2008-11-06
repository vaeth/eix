// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

/* we use strndup */
#if !defined _GNU_SOURCE
#define _GNU_SOURCE
#endif /* !defined _GNU_SOURCE */

#include "stringutils.h"

#include <iostream>
#include <cstdio>

using namespace std;

const char *spaces = " \t\r\n";

#if !defined(HAVE_STRNDUP)
/* If we don't have strndup, we use our own ..
 * darwin (macos) doesn't have strndup, it's a GNU extension
 * See http://bugs.gentoo.org/show_bug.cgi?id=111912 */

char *
strndup(const char *s, size_t n)
{
	char       *r = NULL;
	const char *p = s;
	while(*p++ && n--);
	n = p - s - 1;
	r = static_cast<char *>(malloc(n + 1));
	if(r) {
		memcpy(r, s, n);
		r[n] = 0;
	}
	return r;
}
#endif /* !defined(HAVE_STRNDUP) */

const char *
ExplodeAtom::get_start_of_version(const char* str)
{
	const char *x = NULL;
	// There must be at least one symbol before the version:
	if(!*(str++))
		return NULL;
	while((*str) && (*str != ':'))
	{
		if(*str++ == '-'
		   && (*str >= 48
			   && *str <= 57))
		{
			x = str;
		}
	}

	return x;
}

char *
ExplodeAtom::split_version(const char* str)
{
	const char *x = get_start_of_version(str);

	if(x)
		return strdup(x);
	return NULL;
}

char *
ExplodeAtom::split_name(const char* str)
{
	const char *x = get_start_of_version(str);

	if(x)
		return strndup(str, ((x - 1) - str));
	return NULL;
}

char **
ExplodeAtom::split(const char* str)
{
	static char* out[2] = { NULL, NULL };
	const char *x = get_start_of_version(str);

	if(!x)
		return NULL;
	out[0] = strndup(str, ((x - 1) - str));
	out[1] = strdup(x);
	return out;
}

vector<string>
split_string(const string &str, const char *at, bool ignore_empty, bool ignore_escaped, bool remove_escape)
{
	vector<string> vec;
	string::size_type last_pos = 0,
		pos = 0;
	while((pos = str.find_first_of(at, pos)) != string::npos) {
		if(ignore_escaped) {
			bool escaped = false;
			string::size_type s = pos;
			while(s > 0)
			{
				if(str[--s] != '\\')
					break;
				escaped = !escaped;
			}
			if(escaped)
			{
				++pos;
				continue;
			}
		}
		if((pos - last_pos) > 0 || !ignore_empty)
			vec.push_back(str.substr(last_pos, pos - last_pos));
		last_pos = ++pos;
	}
	if((str.size() - last_pos) > 0 || !ignore_empty)
		vec.push_back(str.substr(last_pos));
	if(remove_escape)
	{
		for(vector<string>::iterator it = vec.begin();
			it != vec.end(); ++it) {
			pos = 0;
			while((pos = it->find_first_of(at, pos)) != string::npos) {
				it->erase(pos - 1, 1);
			}
		}
	}
	return vec;
}

string
join_set(const set<string> &vec, const string &glue)
{
	set<string>::const_iterator it = vec.begin();
	if(it == vec.end()) {
		return "";
	}
	string ret;
	for(;;) {
		ret.append(*it);
		if(++it == vec.end()) {
			break;
		}
		ret.append(glue);
	}
	return ret;
}

string
join_vector(const vector<string> &vec, const string &glue)
{
	vector<string>::const_iterator it = vec.begin();
	if(it == vec.end()) {
		return "";
	}
	string ret;
	for(;;) {
		ret.append(*it);
		if(++it == vec.end()) {
			break;
		}
		ret.append(glue);
	}
	return ret;
}

bool
resolve_plus_minus(set<string> &s, const vector<string> &l, bool obsolete_minus, bool *warnminus, const set<string> *warnignore)
{
	bool minusasterisk = false;
	bool minuskeyword  = false;
	s.clear();
	for(vector<string>::const_iterator it = l.begin(); it != l.end(); ++it) {
		if(it->empty())
			continue;
		if((*it)[0] == '+') {
			cerr << "flags should not start with a '+':" << *it << endl << endl;
			s.insert(it->substr(1));
			continue;
		}
		if((*it)[0] == '-') {
			if(*it == "-*") {
				minusasterisk = true;
				if(!obsolete_minus) {
					s.clear();
					continue;
				}
			}
			string key = it->substr(1);
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
		fprintf(stderr, "Internal error: Storing required after finalizing");
		exit(-1);
	}
	push_back(s);
}

void
StringHash::hash_string(const string &s)
{
	if(finalized) {
		fprintf(stderr, "Internal error: Hashing required after finalizing");
		exit(-1);
	}
	if(!hashing) {
		fprintf(stderr, "Internal error: Hashing required in non-hash mode");
		exit(-1);
	}
	map<string, StringHash::size_type>::const_iterator i = str_map.find(s);
	if(i != str_map.end())
		return;
	// For the moment, use str_map only as a set: Wait for finalize()
	str_map[s] = 0;//size();
	//store_string(s);
}

void
StringHash::store_words(const std::string &s)
{
	vector<string> v = split_string(s);
	for(vector<string>::const_iterator i = v.begin(); i != v.end(); ++i)
		store_string(*i);
}

void
StringHash::hash_words(const std::string &s)
{
	vector<string> v = split_string(s);
	for(vector<string>::const_iterator i = v.begin(); i != v.end(); ++i)
		hash_string(*i);
}

StringHash::size_type
StringHash::get_index(const string &s) const
{
	if(!finalized) {
		fprintf(stderr, "Internal error: Index required before sorting.");
		exit(-1);
	}
	map<string, StringHash::size_type>::const_iterator i = str_map.find(s);
	if(i == str_map.end()) {
		fprintf(stderr, "Internal error: Trying to shortcut non-hashed string.");
		exit(-1);
	}
	return i->second;
}

const string&
StringHash::operator[](StringHash::size_type i) const
{
	if(i >= size()) {
		fprintf(stderr, "Database corrupt: Nonexistent hash required");
		exit(-1);
	}
	return vector<string>::operator[](i);
}

void
StringHash::output(const char *s) const
{
	if(s)
		cout << s << ":\n";
	for(vector<string>::const_iterator i = begin(); i != end(); ++i)
		cout << *i << "\n";
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
	for(map<string, size_type>::iterator it = str_map.begin();
		it != str_map.end(); ++it) {
		it->second = size();
		push_back(it->first);
	}
}
