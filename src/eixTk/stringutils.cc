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

#include "stringutils.h"
		
Regex re_fn2ver(".*-\\([^a-zA-Z].*\\)", 0);
Regex re_fn2name("\\(.*\\)-[^a-zA-Z].*", 0);
Regex re_fn2name_ver("\\(.*\\)-\\([^a-zA-Z].*\\)", 0);

char *
ExplodeAtom::getVersion(const char* filename)
{
	regmatch_t matchptr[2];
	char *ret = NULL;
	if(regexec(re_fn2ver.get(), filename, 2, matchptr, 0) == 0) {
		OOM_ASSERT(ret =
				strndup( &(filename[matchptr[1].rm_so]),
					matchptr[1].rm_eo-matchptr[1].rm_so)
				);
	}
	return ret;
}

char *
ExplodeAtom::getName(const char* filename)
{
	regmatch_t matchptr[2];
	char *ret = NULL;
	if(regexec(re_fn2name.get(), filename, 2, matchptr, 0) == 0) {
		OOM_ASSERT(ret =
				strndup( &(filename[matchptr[1].rm_so]),
					matchptr[1].rm_eo-matchptr[1].rm_so	 )
				);
	}
	return ret;
}

char **
ExplodeAtom::getNameVersion(const char* filename)
{
	regmatch_t matchptr[3];
	static char* out[2] = { NULL, NULL };
	if(regexec(re_fn2name_ver.get(), filename, 3, matchptr, 0) == 0) {
		OOM_ASSERT(out[0] =
				strndup( &(filename[matchptr[1].rm_so]),
					matchptr[1].rm_eo-matchptr[1].rm_so )
				);
		OOM_ASSERT(out[1] =
				strndup( &(filename[matchptr[2].rm_so]),
					matchptr[2].rm_eo-matchptr[2].rm_so )
				);
		return out;
	}
	return NULL;
}

vector<string>
split_string(const string &str, const char *at, bool ignore_empty)
{
	vector<string> vec;
	string::size_type last_pos = 0,
		pos = 0;
	while((pos = str.find_first_of(at, pos)) != string::npos) {
		if((pos - last_pos) > 0 || !ignore_empty)
			vec.push_back(str.substr(last_pos, pos - last_pos));
		last_pos = ++pos;
	}
	if((str.size() - last_pos) > 0 || !ignore_empty)
		vec.push_back(str.substr(last_pos));
	return vec;
}

string
join_vector(vector<string> &vec, string glue)
{
	string ret;
	vector<string>::iterator it = vec.begin();
	if(it == vec.end()) {
		return "";
	}
	for(;;) {
		ret.append(*it);
		if(++it == vec.end()) {
			break;
		}
		ret.append(glue);
	}
	return ret;
}

inline 
bool 
vec_clean(vector<string>::iterator begin, vector<string>::iterator end, string s)
{
	bool found = false;
	vector<string>::iterator it = find(begin, end, s);
	while(it != end) {
		*it = "";
		it = find(begin, end, s);
		found = true;
	}
	return found;
}


string
resolve_plus_minus(string &v, bool warn_plus, bool order)
{
	vector<string> splitted = split_string(v); 
	return join_vector(resolve_plus_minus(splitted, warn_plus, order));
}

vector<string>& 
resolve_plus_minus(vector<string> &v, bool warn_plus, bool order)
{
	for(vector<string>::iterator it = v.begin(); it != v.end(); ++it) {
		if(it->size() == 0) {
			continue;
		}
		if((*it)[0] == '+') {
			warn_plus && cerr << "keyword begins with '+': " << *it << endl;
			it->erase(0, 1);
		}
		if((*it)[0] == '-') {
			if(*it == "-*") {
				v.erase(v.begin(), ++it);
				return resolve_plus_minus(v, warn_plus, order);
			}
			else if(vec_clean(v.begin(), order ? it : v.end(), it->substr(1))) {
				*it = "";
			}
		}
	}
	vector<string>::iterator it = v.begin();
	while(it != v.end()) {
		if(it->size() == 0) {
			v.erase(it);
		}
		else {
			++it;
		}
	}
	return v;
}
