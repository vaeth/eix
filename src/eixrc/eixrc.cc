// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "eixrc.h"
#include <eixTk/exceptions.h>
#include <varsreader.h>
#include <cstdlib>

#include <portage/conf/portagesettings.h>


#define EIX_USERRC   "/.eixrc"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif /* SYSCONFDIR */

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

using namespace std;

EixRcOption::EixRcOption(OptionType t, std::string name, std::string val, std::string desc) {
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

LocalMode
EixRc::getLocalMode(const char *key)
{
	const char *s = (*this)[key].c_str();
	if((*s == '+') || (strcasecmp(s, "local") == 0))
		return LOCALMODE_LOCAL;
	if((*s == '-') || (strcasecmp(s, "non-local") == 0) ||
		(strcasecmp(s, "nonlocal") == 0) ||
		(strcasecmp(s, "global") == 0) ||
		(strcasecmp(s, "original") == 0))
		return LOCALMODE_NONLOCAL;
	return LOCALMODE_DEFAULT;
}

const char *
EixRc::cstr(const char *var) const
{
	map<string,string>::const_iterator s = find(var);
	if(s == end())
		return NULL;
	return (s->second).c_str();
}

const char *
EixRc::prefix_cstr(const char *var) const
{
	const char *s = cstr(var);
	if(!s)
		return NULL;
	if(s[0])
		return s;
	// Maybe later: Test whether some eprefix-variable is set,
	// and return "" instead of NULL in this case.
	return NULL;
}

void
EixRc::read()
{
	const char *name = "PORTAGE_CONFIGROOT";
	const char *configroot = getenv(name);
	if(configroot) {
		m_eprefixconf = configroot;
		modify_value(m_eprefixconf, name);
	}
	else {
		name = "EPREFIX";
		configroot = getenv(name);
		if(configroot)
			m_eprefixconf = configroot;
		else
			m_eprefixconf = EPREFIX_DEFAULT;
		modify_value(m_eprefixconf, name);
	}

	set<string> has_reference;

	// First, we create defaults and the main map with all variables
	// (including all values required by delayed references).
	read_undelayed(has_reference);

	// Resolve delayed references recursively.
	for(default_index i = 0; i < defaults.size(); ++i)
	{
		set<string> visited;
		const char *errtext;
		string errvar;
		if(resolve_delayed_recurse(defaults[i].key, visited,
			has_reference, &errtext, &errvar) == NULL)
		{
			cerr << "fatal config error: " << errtext
				<< " in delayed substitution of " << errvar
				<< "\n";
			exit(2);
		}
	}

	// Let %%{ expand to %{
	for(map<string,string>::iterator it = begin(); it != end(); ++it)
	{
		string &str = it->second;
		string::size_type pos = 0;
		for(;; pos += 2)
		{
			pos = str.find("%%{", pos);
			if(pos == string::npos)
				break;
			str.erase(pos,1);
		}
	}

	// set m_eprefixconf to possibly new settings:
	m_eprefixconf = (*this)["PORTAGE_CONFIGROOT"];
}

string
*EixRc::resolve_delayed_recurse(string key, set<string> &visited, set<string> &has_reference, const char **errtext, string *errvar)
{
	string *value = &((*this)[key]);
	if(has_reference.find(key) == has_reference.end()) {
		modify_value(*value, key);
		return value;
	}
	string::size_type pos = 0;
	for(;;)
	{
		string::size_type length;
		DelayedType type = find_next_delayed(*value, &pos, &length);
		if(type == DelayedNotFound) {
			has_reference.erase(key);
			modify_value(*value, key);
			return value;
		}
		if(type == DelayedFi) {
			*errtext = "FI without IF";
			*errvar = key;
			return NULL;
		}
		if(type == DelayedElse) {
			*errtext = "ELSE without IF";
			*errvar = key;
			return NULL;
		}
		bool will_test = false;
		string::size_type varpos = pos + 2;
		string::size_type varlength = length - 3;
		if((type == DelayedIfTrue) || (type == DelayedIfFalse)) {
			will_test = true;
			varpos++;
			varlength--;
		}
		else if((type == DelayedIfEmpty) || (type == DelayedIfNonempty)) {
			will_test = true;
			varpos+=2;
			varlength-=2;
		}
		if(visited.find(key) != visited.end()) {
			*errtext = "self-reference";
			*errvar = key;
			return NULL;
		}
		visited.insert(key);
		string *s = resolve_delayed_recurse(
			( ((*value)[varpos] == '*') ?
			(varprefix + value->substr(varpos + 1, varlength - 1)) :
			value->substr(varpos, varlength)),
			visited, has_reference, errtext, errvar);
		visited.erase(key);
		if(!s)
			return NULL;
		if(! will_test) {
			value->replace(pos, length, *s);
			pos += s->length();
			continue;
		}
		// will_test: type is necessarily one of
		// DelayedIfTrue/DelayedIfFalse/DelayedIfNonempty/DelayedIfEmpty
		string::size_type skippos = pos;
		bool result;
		if((type == DelayedIfTrue) || (type == DelayedIfFalse)) {
			result = istrue(s->c_str());
			if(type == DelayedIfFalse)
				result = !result;
		}
		else { // ((type == DelayedIfEmpty) || (type == DelayedIfNonempty))
			result = s->empty();
			if(type == DelayedIfNonempty)
				result = !result;
		}
		string::size_type delpos = string::npos;
		if(result)
			value->erase(skippos, length);
		else {
			delpos = skippos;
			skippos += length;
		}
		bool gotelse = false;
		unsigned int curr_count = 0;
		for(;; skippos += length)
		{
			type = find_next_delayed(*value, &skippos, &length);

			if(type == DelayedFi) {
				if(curr_count --)
					continue;
				if(delpos == string::npos)
					value->erase(skippos, length);
				else
					value->erase(delpos,
						(skippos + length) - delpos);
				break;
			}
			if(type == DelayedElse) {
				if(curr_count)
					continue;
				if(gotelse) {
					*errtext = "double ELSE";
					*errvar = key;
					return NULL;
				}
				gotelse = true;
				if(result) {
					value->erase(skippos, length);
					length = 0;
					delpos = skippos;
					continue;
				}
				value->erase(delpos,
					(skippos + length) - delpos);
				skippos = delpos;
				length = 0;
				delpos = string::npos;
				continue;
			}
			if((type == DelayedIfTrue) || (type == DelayedIfFalse)) {
				curr_count ++;
				continue;
			}
			if(type == DelayedNotFound) {
				*errtext = "IF without FI";
				*errvar = key;
				return NULL;
			}
		}
	}
}

inline static void
override_by_env(map<string,string> &m)
{
	for(map<string,string>::iterator it = m.begin(); it != m.end(); ++it) {
		char *val = getenv((it->first).c_str());
		if(val)
			it->second = string(val);
	}
}

/** Create defaults and the main map with all variables
   (including all values required by delayed references).
   @arg has_reference is initialized to corresponding keys */
void
EixRc::read_undelayed(set<string> &has_reference)
{
	map<string,string> tempmap;
	set<string> default_keys;

	// Initialize with the default variables
	for(default_index i = 0; i < defaults.size(); ++i) {
		default_keys.insert(defaults[i].key);
		tempmap[defaults[i].key] = defaults[i].value;
	}

	// override with ENV
	override_by_env(tempmap);

	VarsReader rc(//VarsReader::NONE
			VarsReader::SUBST_VARS
			|VarsReader::ALLOW_SOURCE_VARNAME
			|VarsReader::INTO_MAP);
	rc.useMap(&tempmap);
	rc.setPrefix("EIXRC_SOURCE");

	const char *rc_file = getenv("EIXRC");
	if(rc_file)
		rc.read(rc_file);
	else
	{
		// override with EIX_SYSTEMRC
		rc.read((m_eprefixconf + EIX_SYSTEMRC).c_str());

		// override with EIX_USERRC
		char *home = getenv("HOME");
		if(!home)
			cerr << "No $HOME found in environment." << endl;
		else {
			string eixrc(home);
			eixrc.append(EIX_USERRC);
			rc.read(eixrc.c_str());
		}
	}

	// override with ENV
	override_by_env(tempmap);

	// Set new values as default and for printing with --dump.
	for(vector<EixRcOption>::iterator it = defaults.begin();
		it != defaults.end(); ++it) {
		string &value = tempmap[it->key];
		modify_value(value, it->key);
		it->local_value = value;
		(*this)[it->key] = value;
	}

	// Recursively join all delayed references to defaults,
	// keeping main map up to date. Also initialize has_reference.
	for(default_index i = 0; i < defaults.size(); ++i)
	{
		string &str = defaults[i].local_value;
		string::size_type pos = 0;
		string::size_type length = 0;
		for(;; pos += length)
		{
			DelayedType type = find_next_delayed(str, &pos, &length);
			if (type == DelayedNotFound)
				break;
			else if (type == DelayedVariable) {
				pos += 2;
				length -= 2;
			}
			else if ((type == DelayedIfTrue) || (type == DelayedIfFalse)) {
				pos += 3;
				length -= 3;
			}
			else
				continue;
			has_reference.insert(defaults[i].key);
			if(str[pos] == '*') {
				string s = str.substr(pos + 1, length - 2);
				join_delayed(string(EIX_VARS_PREFIX) + s,
					default_keys, tempmap);
				join_delayed(string(DIFF_EIX_VARS_PREFIX) + s,
					default_keys, tempmap);
			}
			else {
				join_delayed(str.substr(pos, length - 1),
					default_keys, tempmap);
			}
		}
	}
}

void
EixRc::join_delayed(const string &key, set<string> &default_keys, const map<string,string> &tempmap)
{
	if(default_keys.find(key) != default_keys.end())
		return;
	string val;
	map<string,string>::const_iterator f = tempmap.find(key);
	if(f != tempmap.end()) {
	// Note that if a variable is defined in a file and in ENV,
	// its value was already overridden from ENV.
		val = f->second;
	}
	else {
	// If it was not defined in a file, it might be in ENV anyway:
		char *envval = getenv(key.c_str());
		if(envval)
			val = string(envval);
	}
	// If some day e.g. prefix_keys (variables with PREFIXSTRING)
	// should possibly also contain local variables, better modify it:
	modify_value(val, key);
	defaults.push_back(EixRcOption(EixRcOption::LOCAL, key, val, ""));
	default_keys.insert(key);
	(*this)[key] = val;
}

EixRc::DelayedType
EixRc::find_next_delayed(const string &str, string::size_type *posref, string::size_type *length)
{
	string::size_type pos = *posref;
	for(;; pos += 2)
	{
		pos = str.find("%{", pos);
		if(pos == string::npos)
		{
			*posref = string::npos;
			return DelayedNotFound;
		}
		if(pos > 0) {
			if(str[pos - 1] == '%')
				continue;
		}
		string::size_type i = pos + 2;
		char c = str[i++];
		DelayedType type;
		if(c == '}')
			type = DelayedFi;
		else
		{
			if(c == '?') {
				c = str[i++];
				if(c != '?')
					type = DelayedIfTrue;
				else {
					type = DelayedIfNonempty;
					c = str[i++];
				}
			}
			else if(c == '!') {
				type = DelayedIfFalse;
				c = str[i++];
				if(c != '?')
					type = DelayedIfFalse;
				else {
					type = DelayedIfEmpty;
					c = str[i++];
				}
			}
			else
				type = DelayedVariable;
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
			if(strcasecmp(
				(str.substr(pos + 2, i - pos - 3)).c_str(),
				"else") == 0)
				type = DelayedElse;
		}
		*posref = pos;
		if(length)
			*length = i - pos;
		return type;
	}
}

void
EixRc::modify_value(string &value, const string &key)
{
	if(value == "/") {
		if(prefix_keys.find(key) != prefix_keys.end())
			value.clear();
	}
}

void
EixRc::clear()
{
	defaults.clear();
	prefix_keys.clear();
	(dynamic_cast<map<string,string>*>(this))->clear();
}

void
EixRc::addDefault(EixRcOption option)
{
	if(option.type == EixRcOption::PREFIXSTRING)
		prefix_keys.insert(option.key);
	modify_value(option.value, option.key);
	defaults.push_back(option);
}

bool
EixRc::istrue(const char *s)
{
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

short
EixRc::getBeforeAfter(const char *key)
{
	const char *s = (*this)[key].c_str();
	if(!strcasecmp(s, "first"))
		return 2;
	if(!strcasecmp(s, "before"))
		return 1;
	if(!strcasecmp(s, "after"))
		return -1;
	if(!strcasecmp(s, "last"))
		return -2;
	if(istrue(s))
		return 1;
	return 0;
}

void
EixRc::getRedundantFlags(const char *key, Keywords::Redundant type, RedPair &p)
{
	string value=(*this)[key].c_str();
	vector<string> a=split_string(value);

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

	cerr << key << " has unknown value \"" << value << "\";" << endl
	     << "\tassuming value \"all-installed\" instead." << endl;

	getRedundantFlagAtom("all-installed", type, p.first);
	getRedundantFlagAtom(NULL, type, p.second);
}

int
EixRc::getInteger(const char *key)
{
	return atoi((*this)[key].c_str());
}

string
EixRc::as_comment(const char *s)
{
	string ret = s;
	string::size_type pos = 0;
	while(pos = ret.find("\n", pos), pos != string::npos) {
		ret.insert(pos + 1, "# ");
		pos += 2;
	}
	return ret;
}


void
EixRc::dumpDefaults(FILE *s, bool use_defaults)
{
	const char *message = use_defaults ?
		"was locally changed to:" :
		"changed locally, default was:";
	for(vector<EixRcOption>::size_type i = 0;
		i < defaults.size();
		++i)
	{
		const char *typestring = "UNKNOWN";
		switch(defaults[i].type) {
			case EixRcOption::BOOLEAN: typestring = "BOOLEAN";
						  break;
			case EixRcOption::STRING: typestring = "STRING";
						  break;
			case EixRcOption::PREFIXSTRING: typestring = "PREFIXSTRING";
						  break;
			case EixRcOption::INTEGER: typestring = "INTEGER";
						  break;
			case EixRcOption::LOCAL: typestring = NULL;
						  break;
			default:
						  break;
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

void
EixRc::print_var(const char *var)
{
	if(strcmp(var, "PORTDIR")) {
		const char *s = cstr(var);
		if(s) {
			std::cout << s;
			return;
		}
	}
	PortageSettings ps(*this, false);
	std::cout << ps[var];
}

