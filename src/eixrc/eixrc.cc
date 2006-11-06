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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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

#include "eixrc.h"
#include <eixTk/exceptions.h>
#include <varsreader.h>

#define EIX_USERRC   "/.eixrc"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif /* SYSCONFDIR */

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

using namespace std;

EixRcOption::EixRcOption(char t, std::string name, std::string val, std::string desc) {
	type = t;
	key = name;
	if(type == LOCAL) {
		local_value = val;
	}
	else {
		value = val;
		description = desc;
	}
}

bool
EixRc::getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom &r)
{
	r.only &= ~type;
	if(s == NULL)
	{
		r.red &= ~type;
		return true;
	}
	if(*s == '+')
	{
		s++;
		r.only |= type;
		r.oins |= type;
	}
	else if(*s == '-')
	{
		s++;
		r.only |= type;
		r.oins &= ~type;
	}
	if((strcasecmp(s, "no") == 0) ||
	   (strcasecmp(s, "false") == 0))
	{
		r.red &= ~type;
	}
	else if(strcasecmp(s, "some") == 0)
	{
		r.red |= type;
		r.all &= ~type;
		r.spc &= ~type;
	}
	else if(strcasecmp(s, "some-installed") == 0)
	{
		r.red |= type;
		r.all &= ~type;
		r.spc |= type;
		r.ins |= type;
	}
	else if(strcasecmp(s, "some-uninstalled") == 0)
	{
		r.red |= type;
		r.all &= ~type;
		r.spc |= type;
		r.ins &= ~type;
	}
	else if(strcasecmp(s, "all") == 0)
	{
		r.red |= type;
		r.all |= type;
		r.spc &= ~type;
	}
	else if(strcasecmp(s, "all-installed") == 0)
	{
		r.red |= type;
		r.all |= type;
		r.spc |= type;
		r.ins |= type;
	}
	else if(strcasecmp(s, "all-uninstalled") == 0)
	{
		r.red |= type;
		r.all |= type;
		r.spc |= type;
		r.ins &= ~type;
	}
	else
		return false;
	return true;
}

class EixRcReference {
	public:
		bool in_defaults;
		EixRc::default_index ind;

		EixRcReference()
		{ in_defaults = false; }

		void set_reference(EixRc::default_index i)
		{ in_defaults = true; ind = i; }
};

void EixRc::read()
{
	map<string,string> *tempmap = new map<string,string>;
	map<string,EixRcReference> props;

	// Initialize with the default variables
	for(default_index i = 0; i < defaults.size(); ++i) {
		props[defaults[i].key].set_reference(i);
		(*tempmap)[defaults[i].key] = defaults[i].value;
	}

	// override with EIX_SYSTEMRC
	VarsReader rc(//VarsReader::NONE
			VarsReader::SUBST_VARS
			|VarsReader::ALLOW_SOURCE
			|VarsReader::INTO_MAP);
	rc.useMap(tempmap);
	rc.read(EIX_SYSTEMRC);

	// override with EIX_USERRC
	char *home = getenv("HOME");
	if(!home)
		WARNING("No $HOME found in environment.");
	else
	{
		string eixrc(home);
		eixrc.append(EIX_USERRC);
		rc.read(eixrc.c_str());
	}

	// override with ENV
	bool have_delayed = false;
	for(map<string,string>::iterator it = tempmap->begin();
		it != tempmap->end(); ++it)
	{
		char *val = getenv((it->first).c_str());
		if(val)
			it->second = string(val);
	}

	// Set new values as default and for printing with --dump.
	for(vector<EixRcOption>::iterator it = defaults.begin();
		it != defaults.end(); ++it) {
		it->local_value = (*tempmap)[it->key];
		(*this)[it->key] = it->local_value;
	}

	// Recursively join all delayed references to defaults.
	// Make sure that main map is kept up to date.
	for(default_index i = 0; i < defaults.size(); ++i)
	{
		string &str = defaults[i].local_value;
		string::size_type pos = 0;
		for(;;)
		{
			string::size_type length;
			pos = find_delayed(str, pos, &length);
			if(pos == string::npos)
				break;
			if(str[pos + 2] == '*') {
				string s = str.substr(pos + 3, length - 4);
				join_delayed(string(EIX_VARS_PREFIX) + s,
					props, tempmap);
				join_delayed(string(DIFF_EIX_VARS_PREFIX) + s,
					props, tempmap);
			}
			else {
				join_delayed(str.substr(pos + 2, length - 3),
					props, tempmap);
			}
			pos += length;
		}
	}

	// Now we can forget all other variables in the config files.
	delete tempmap;

	// Resolve delayed references recursively.
	for(default_index i = 0; i < defaults.size(); ++i)
	{
		set<string> visited;
		if(resolve_delayed(defaults[i].key, visited) == NULL)
		{
			cerr << "fatal error in config: self-reference in delayed substitution for "
				<< (defaults[i].key) << "\n";
			exit(2);
		}
	}
}

void EixRc::join_delayed(const string &key, map<string,EixRcReference> &props, map<string,string> *tempmap)
{
	EixRcReference &ref = props[key];
	if(ref.in_defaults)
		return;
	string val;
	map<string,string>::iterator f = tempmap->find(key);
	if(f != tempmap->end())
		val = f->second;
	else {
		char *envval = getenv(key.c_str());
		if(envval)
			val = string(envval);
	}
	ref.in_defaults = true;
	ref.ind = defaults.size();
	defaults.push_back(EixRcOption(EixRcOption::LOCAL, key, val, ""));
	(*this)[key] = val;
}

string *EixRc::resolve_delayed(string key, set<string> &visited)
{
	string &value = (*this)[key];
	string::size_type pos = 0;
	for(;;)
	{
		string::size_type length;
		pos = find_delayed(value, pos, &length);
		if(pos == string::npos)
			return &value;
		if(visited.find(key) != visited.end())
			return NULL;
		visited.insert(key);
		string *s = resolve_delayed(((value[pos + 2] == '*') ?
			(prefix + value.substr(pos + 3, length - 4)) :
			value.substr(pos + 2, length - 3)), visited);
		visited.erase(key);
		if(!s)
			return NULL;
		value.replace(pos, length, *s);
	}
}

string::size_type EixRc::find_delayed(const string &str, string::size_type pos, string::size_type *length)
{
	for(;; pos += 2)
	{
		pos = str.find("%{", pos);
		if(pos == string::npos)
			return string::npos;
		if(pos > 0) {
			if(str[pos - 1] == '%')
				continue;
		}
		string::size_type i = pos + 2;
		char c = str[i++];
		if((c != '*') && (c != '_') &&
			((c < 'A') || (c > 'Z')) &&
			((c < 'a') || (c > 'z')))
			continue;
		for(;;)
		{
			c = str[i++];
			if ((c != '_') &&
				((c < '0') || (c > '9')) &&
				((c < 'A') || (c > 'Z')) &&
				((c < 'a') || (c > 'z')))
				break;
		}
		if(c != '}')
			continue;
		if(length)
			*length = i - pos;
		return pos;
	}
}

void EixRc::clear()
{
	defaults.clear();
	((map<string,string>*) this)->clear();
}

void EixRc::addDefault(EixRcOption option)
{
	defaults.push_back(option);
}

bool EixRc::getBool(const char *key)
{
	const char *s = (*this)[key].c_str();
	if(strcasecmp(s, "true") == 0)
		return true;
	if(strcasecmp(s, "1") == 0)
		return true;
	if(strcasecmp(s, "yes") == 0)
		return true;
	if(strcasecmp(s, "y") == 0)
		return true;
	if(strcasecmp(s, "on") == 0)
		return true;
	return false;
}

void EixRc::getRedundantFlags(const char *key,
	Keywords::Redundant type,
	RedPair &p)
{
	string value=(*this)[key].c_str();
	vector<string> a=split_string(value);
	bool fail = false;

	for(;;)// a dummy loop for break on errors
	{
		vector<string>::iterator it = a.begin();
		if(it == a.end())
			break;
		if(!getRedundantFlagAtom(it->c_str(), type, p.first))
			break;
		++it;
		if(it == a.end())
		{
			getRedundantFlagAtom(NULL, type, p.second);
			return;
		}
		const char *s = it->c_str();
		if((strcasecmp(s, "or") == 0) ||
			(strcasecmp(s, "||") == 0) ||
			(strcasecmp(s, "|") == 0))
		{
			++it;
			if(it == a.end())
				break;
			s = it->c_str();
		}
		if(!getRedundantFlagAtom(s, type, p.first))
			break;
		++it;
		if(it == a.end())
			return;
		break;
	}
	WARNING("%s has unknown value \"%s\";\n"
		"\tassuming value \"all-installed\" instead.",
		key, value.c_str());
	getRedundantFlagAtom("all-installed", type, p.first);
	getRedundantFlagAtom(NULL, type, p.second);
}

int EixRc::getInteger(const char *key)
{
	return atoi((*this)[key].c_str());
}

string EixRc::as_comment(const char *s)
{
	string ret = s;
	string::size_type pos = 0;
	while(pos = ret.find("\n", pos), pos != string::npos) {
		ret.insert(pos + 1, "# ");
		pos += 2;
	}
	return ret;
}


void EixRc::dumpDefaults(FILE *s, bool use_defaults)
{
	const char *message = use_defaults ?
		"was locally changed to:" :
		"changed locally, default was:";
	for(unsigned int i = 0;
		i<defaults.size();
		++i)
	{
		const char *typestring = "UNKNOWN";
		switch(defaults[i].type) {
			case EixRcOption::BOOLEAN: typestring = "BOOLEAN";
						  break;
			case EixRcOption::STRING: typestring = "STRING";
						  break;
			case EixRcOption::INTEGER: typestring = "INTEGER";
						  break;
			case EixRcOption::LOCAL: typestring = NULL;
		}
		const char *key   = defaults[i].key.c_str();
		const char *value = defaults[i].local_value.c_str();
		if(!typestring) {
			fprintf(s,
				"# locally added:\n%s='%s'\n\n",
				key, value);
			continue;
		}
		const char *deflt = defaults[i].value.c_str();
		const char *output = (use_defaults ? deflt : value);
		const char *comment = (use_defaults ? value : deflt);
		fprintf(s,
				"# %s\n"
				"# %s\n"
				"%s='%s'\n",
				as_comment(typestring).c_str(),
				as_comment(defaults[i].description.c_str()).c_str(),
				key,
				output);
		if(strcmp(deflt,value) == 0)
			fprintf(s, "\n");
		else {
			fprintf(s,
				"# %s\n"
				"# %s='%s'\n\n",
				message,
				key,
				as_comment(comment).c_str());
		}
	}
}
