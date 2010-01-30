// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "eixrc.h"
#include <config.h>

#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>
#include <eixTk/varsreader.h>

#include <portage/conf/portagesettings.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

#define EIX_USERRC   "/.eixrc"

using namespace std;

EixRcOption::EixRcOption(OptionType t, string name, string val, string desc) {
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

short
EixRc::getBoolText(const string &key, const char *text)
{
	const char *s((*this)[key].c_str());
	if(!strcasecmp(s, text))
		return -1;
	if(istrue(s))
		return 1;
	return 0;
}

short
EixRc::getBoolTextlist(const string &key, const char **text)
{
	const char *s((*this)[key].c_str());
	for(int i(-1); likely(*text != NULL); ++text, --i) {
		if(!strcasecmp(s, *text))
			return i;
	}
	if(istrue(s))
		return 1;
	return 0;
}

bool
EixRc::getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom &r)
{
	r.only &= ~type;
	if(unlikely(s == NULL)) {
		r.red &= ~type;
		return true;
	}
	if(*s == '+') {
		++s;
		r.only |= type;
		r.oins |= type;
	}
	else if(*s == '-') {
		++s;
		r.only |= type;
		r.oins &= ~type;
	}
	if((strcasecmp(s, "no") == 0) ||
	   (strcasecmp(s, "false") == 0)) {
		r.red &= ~type;
	}
	else if(strcasecmp(s, "some") == 0) {
		r.red |= type;
		r.all &= ~type;
		r.spc &= ~type;
	}
	else if(strcasecmp(s, "some-installed") == 0) {
		r.red |= type;
		r.all &= ~type;
		r.spc |= type;
		r.ins |= type;
	}
	else if(strcasecmp(s, "some-uninstalled") == 0) {
		r.red |= type;
		r.all &= ~type;
		r.spc |= type;
		r.ins &= ~type;
	}
	else if(strcasecmp(s, "all") == 0) {
		r.red |= type;
		r.all |= type;
		r.spc &= ~type;
	}
	else if(strcasecmp(s, "all-installed") == 0) {
		r.red |= type;
		r.all |= type;
		r.spc |= type;
		r.ins |= type;
	}
	else if(strcasecmp(s, "all-uninstalled") == 0) {
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
EixRc::getLocalMode(const string &key)
{
	const char *s((*this)[key].c_str());
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
EixRc::cstr(const string &key) const
{
	map<string,string>::const_iterator s(main_map.find(key));
	if(s == main_map.end())
		return NULL;
	return (s->second).c_str();
}

const char *
EixRc::prefix_cstr(const string &key) const
{
	const char *s(cstr(key));
	if(unlikely(s == NULL))
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
	const char *name("EIX_PREFIX");
	const char *eixrc_prefix(getenv(name));
	if(eixrc_prefix)
		m_eprefixconf = eixrc_prefix;
	else {
		name = "PORTAGE_CONFIGROOT";
		eixrc_prefix = getenv(name);
		if(eixrc_prefix)
			m_eprefixconf = eixrc_prefix;
		else {
			name = "EIX_PREFIX";
			m_eprefixconf = EIX_PREFIX_DEFAULT;
		}
	}
	modify_value(m_eprefixconf, name);

	set<string> has_delayed;

	// First, we create defaults and main_map with all variables
	// (including all values required by delayed references).
	read_undelayed(has_delayed);

	// Resolve delayed references recursively.
	for(default_index i(0); i < defaults.size(); ++i)
		resolve_delayed(defaults[i].key, has_delayed);

	// set m_eprefixconf to possibly new settings:
	m_eprefixconf = (*this)["PORTAGE_CONFIGROOT"];
}

const string &
EixRc::operator[](const string &key)
{
	map<string,string>::const_iterator it(main_map.find(key));
	if(it != main_map.end())
		return it->second;
	add_later_variable(key);
	return main_map[key];
}

/** This will fetch a variable which was not set in the
    defaults (or its modification or its delayed references),
    i.e. it must be fetched from the config or ENV setting.
    Of course, it will be resolved for delayed substitutions,
    and delayed references are also be added similarly. */
void
EixRc::add_later_variable(const string &key)
{
	set<string> has_delayed;
	join_key(key, has_delayed, true, NULL);
	resolve_delayed(key, has_delayed);
}

void
EixRc::resolve_delayed(string key, set<string> &has_delayed)
{
	set<string> visited;
	const char *errtext;
	string errvar;
	if(unlikely(resolve_delayed_recurse(key, visited, has_delayed,
		&errtext, &errvar) == NULL)) {
		cerr << eix::format(_(
			"fatal config error: %s in delayed substitution of %s"))
			% errtext % errvar << endl;
		exit(EXIT_FAILURE);
	}
}

string *
EixRc::resolve_delayed_recurse(string key, set<string> &visited, set<string> &has_delayed, const char **errtext, string *errvar)
{
	string *value(&(main_map[key]));
	if(has_delayed.find(key) == has_delayed.end()) {
		modify_value(*value, key);
		return value;
	}
	string::size_type pos(0);
	for(;;) {
		string::size_type length;
		DelayedType type(find_next_delayed(*value, &pos, &length));
		bool will_test(false);
		string::size_type varpos(pos + 2);
		string::size_type varlength(length - 3);
		switch(type) {
			case DelayedNotFound:
				has_delayed.erase(key);
				modify_value(*value, key);
				return value;
			case DelayedFi:
				*errtext = _("FI without IF");
				*errvar = key;
				return NULL;
			case DelayedElse:
				*errtext = _("ELSE without IF");
				*errvar = key;
				return NULL;
			case DelayedQuote:
				pos += length - 1 ;
				value->erase(pos);
				continue;
			case DelayedIfTrue:
			case DelayedIfFalse:
				will_test = true;
				++varpos;
				--varlength;
				break;
			case DelayedIfEmpty:
			case DelayedIfNonempty:
				will_test = true;
				varpos += 2;
				varlength -= 2;
				break;
			default:
				break;
		}
		if(unlikely(visited.find(key) != visited.end())) {
			*errtext = _("self-reference");
			*errvar = key;
			return NULL;
		}
		visited.insert(key);
		bool have_star(false);
		bool have_escape(false);
		while(likely(varlength >= 1)) {
			bool check_next;
			switch((*value)[varpos]) {
				case '*':
					have_star = check_next = true;
					break;
				case '\\':
					have_escape = check_next = true;
					break;
				default:
					check_next = false;
					break;
			}
			if(likely(!check_next))
				break;
			++varpos;
			--varlength;
		}
		if(unlikely(varlength < 1))
			return NULL;
		string *s(resolve_delayed_recurse(
			(have_star ?
			(varprefix + value->substr(varpos, varlength)) :
			value->substr(varpos, varlength)),
			visited, has_delayed, errtext, errvar));
		visited.erase(key);
		if(unlikely(s == NULL))
			return NULL;
		string escaped;
		if(unlikely(have_escape)) {
			escaped = *s;
			escape_string(escaped);
			s = &escaped;
		}
		if(likely(!will_test)) {
			value->replace(pos, length, *s);
			pos += s->length();
			continue;
		}
		// will_test: type is necessarily one of
		// DelayedIfTrue/DelayedIfFalse/DelayedIfNonempty/DelayedIfEmpty
		string::size_type skippos(pos);
		bool result;
		if(likely((type == DelayedIfTrue) || (type == DelayedIfFalse))) {
			result = istrue(s->c_str());
			if(type == DelayedIfFalse)
				result = !result;
		}
		else { // ((type == DelayedIfEmpty) || (type == DelayedIfNonempty))
			result = s->empty();
			if(type == DelayedIfNonempty)
				result = !result;
		}
		string::size_type delpos(string::npos);
		if(result)
			value->erase(skippos, length);
		else {
			delpos = skippos;
			skippos += length;
		}
		bool gotelse(false);
		unsigned int curr_count(0);
		for(;; skippos += length) {
			type = find_next_delayed(*value, &skippos, &length);
			switch(type) {
				case DelayedFi:
					if(curr_count --)
						continue;
					if(delpos == string::npos)
						value->erase(skippos, length);
					else
						value->erase(delpos,
							(skippos + length) - delpos);
					break;
				case DelayedElse:
					if(curr_count != 0)
						continue;
					if(unlikely(gotelse)) {
						*errtext = _("double ELSE");
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
				case DelayedIfTrue:
				case DelayedIfFalse:
					++curr_count;
					continue;
				case DelayedNotFound:
					*errtext = _("IF without FI");
					*errvar = key;
					return NULL;
				default:
					continue;
			}
			break;
		}
	}
}

inline static void
override_by_env(map<string,string> &m)
{
	for(map<string,string>::iterator it(m.begin()); likely(it != m.end()); ++it) {
		char *val(getenv((it->first).c_str()));
		if(unlikely(val != NULL))
			it->second = string(val);
	}
}

/** Create defaults and the main_map with all variables
   (including all values required by delayed references).
   @arg has_delayed is initialized to corresponding keys */
void
EixRc::read_undelayed(set<string> &has_delayed)
{
	// Initialize with the default variables
	for(default_index i(0); likely(i < defaults.size()); ++i)
		filevarmap[defaults[i].key] = defaults[i].value;

	// override with ENV
	override_by_env(filevarmap);

	VarsReader rc(//VarsReader::NONE
			VarsReader::SUBST_VARS
			|VarsReader::ALLOW_SOURCE_VARNAME
			|VarsReader::INTO_MAP);
	rc.useMap(&filevarmap);
	rc.setPrefix("EIXRC_SOURCE");

	const char *rc_file(getenv("EIXRC"));
	if(unlikely(rc_file != NULL))
		rc.read(rc_file);
	else {
		// override with EIX_SYSTEMRC
		rc.read((m_eprefixconf + EIX_SYSTEMRC).c_str());

		// override with EIX_USERRC
		char *home(getenv("HOME"));
		if(unlikely(home == NULL)) {
			cerr << _("No $HOME found in environment.") << endl;
		}
		else {
			string eixrc(home);
			eixrc.append(EIX_USERRC);
			rc.read(eixrc.c_str());
		}
	}

	// override with ENV
	override_by_env(filevarmap);

	// Set new values as default and for printing with --dump.
	set<string> original_defaults;
	for(vector<EixRcOption>::iterator it(defaults.begin());
		likely(it != defaults.end()); ++it) {
		string &value(filevarmap[it->key]);
		modify_value(value, it->key);
		it->local_value = value;
		original_defaults.insert(it->key);
	}
	// Since join_key_if_new modifies the defaults vector, our loop
	// must go over a copy of the keys and not over the defaults vector.
	for(set<string>::const_iterator it(original_defaults.begin());
		likely(it != original_defaults.end()); ++it) {
		join_key(*it, has_delayed, false, &original_defaults);
	}
}

/** Recursively eval and join key and its delayed references to
    main_map and default; set has_delayed if appropriate */
void
EixRc::join_key(const string &key, set<string> &has_delayed, bool add_top_to_defaults, const set<string> *exclude_defaults)
{
	string &val(main_map[key]);
	map<string,string>::const_iterator f(filevarmap.find(key));
	if(unlikely(f != filevarmap.end())) {
	// Note that if a variable is defined in a file and in ENV,
	// its value was already overridden from ENV.
		val = f->second;
	}
	else {
	// If it was not defined in a file, it might be in ENV anyway:
		char *envval(getenv(key.c_str()));
		if(unlikely(envval != NULL))
			val = string(envval);
	}
	// for the case that some day e.g. prefix_keys (variables with
	// PREFIXSTRING) should possibly also allow to contain local variables,
	// better modify it:
	modify_value(val, key);

	if(unlikely(add_top_to_defaults)) {
		if(unlikely((!exclude_defaults) || (exclude_defaults->find(key) == exclude_defaults->end())))
			defaults.push_back(EixRcOption(EixRcOption::LOCAL, key, val, ""));
	}
	join_key_rec(key, val, has_delayed, exclude_defaults);
}

void
EixRc::join_key_rec(const string &key, const string &val, set<string> &has_delayed, const set<string> *exclude_defaults)
{
	string::size_type pos(0);
	string::size_type length(0);
	for(;; pos += length) {
		switch(find_next_delayed(val, &pos, &length)) {
			case DelayedNotFound:
				return;
			case DelayedVariable:
				pos += 2;
				length -= 2;
				break;
			case DelayedIfTrue:
			case DelayedIfFalse:
				pos += 3;
				length -= 3;
				break;
			default:
				has_delayed.insert(key);
				continue;
		}
		has_delayed.insert(key);
		bool have_star(false);
		while(likely(length > 1)) {
			bool check_next;
			switch(val[pos]) {
				case '*':
					have_star = check_next = true;
					break;
				case '\\':
					check_next = true;
					break;
				default:
					check_next = false;
					break;
			}
			if(likely(!check_next))
				break;
			++pos;
			--length;
		}
		if(unlikely(length <= 1))
			continue;
		string s(val, pos, length - 1);
		if(unlikely(have_star)) {
			join_key_if_new(string(EIX_VARS_PREFIX) + s,
				has_delayed, exclude_defaults);
			join_key_if_new(string(DIFF_VARS_PREFIX) + s,
				has_delayed, exclude_defaults);
		}
		else
			join_key_if_new(s, has_delayed, exclude_defaults);
	}
}

void
EixRc::join_key_if_new(const string &key, set<string> &has_delayed, const set<string> *exclude_defaults)
{
	if(unlikely(main_map.find(key) == main_map.end()))
		join_key(key, has_delayed, true, exclude_defaults);
}

EixRc::DelayedType
EixRc::find_next_delayed(const string &str, string::size_type *posref, string::size_type *length)
{
	string::size_type pos(*posref);
	for(;; pos += 2) {
		pos = str.find("%{", pos);
		if(pos == string::npos)
			return DelayedNotFound;
		string::size_type i(pos + 2);
		if(i >= str.length())
			return DelayedNotFound;
		DelayedType type;
		char c(str[i++]);
		bool findend(true);
		switch(c) {
			case '}':
				type = DelayedFi;
				findend = false;
				break;
			case '%':
				type = DelayedQuote;
				findend = false;
				break;
			case '?':
				if(i >= str.length())
					return DelayedNotFound;
				c = str[i++];
				if(c == '?') {
					if(unlikely(i >= str.length()))
						return DelayedNotFound;
					c = str[i++];
					type = DelayedIfNonempty;
				}
				else
					type = DelayedIfTrue;
				break;
			case '!':
				if(i >= str.length())
					return DelayedNotFound;
				c = str[i++];
				if(c == '?') {
					if(unlikely(i >= str.length()))
						return DelayedNotFound;
					c = str[i++];
					type = DelayedIfEmpty;
				}
				else
					type = DelayedIfFalse;
				break;
			default:
				type = DelayedVariable;
		}
		if(findend) {
			bool headsymbols(true);
			for(;;)
			{
				if(i >= str.length())
					return DelayedNotFound;
				c = str[i++];
				if(headsymbols) {
					switch(c) {
						case '*':
						case '\\':
							continue;
						default:
							headsymbols = false;
							break;
					}
				}
				if((!isalnum(c, localeC)) && (c != '_'))
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
	filevarmap.clear();
	main_map.clear();
}

void
EixRc::addDefault(EixRcOption option)
{
	if(unlikely(option.type == EixRcOption::PREFIXSTRING))
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
	if(strcasecmp(s, _("true")) == 0)
		return true;
	if(strcasecmp(s, _("yes")) == 0)
		return true;
	if(strcasecmp(s, _("on")) == 0)
		return true;
	return false;
}

void
EixRc::getRedundantFlags(const string &key, Keywords::Redundant type, RedPair &p)
{
	const string &value((*this)[key]);
	vector<string> a;
	split_string(a, value);

	for(vector<string>::iterator it(a.begin()); likely(it != a.end()); )
	// a dummy loop for break on errors
	{
		if(unlikely(!getRedundantFlagAtom(it->c_str(), type, p.first)))
			break;
		++it;
		if(it == a.end())
		{
			getRedundantFlagAtom(NULL, type, p.second);
			return;
		}
		const char *s(it->c_str());
		if((strcasecmp(s, "or") == 0) ||
			(strcasecmp(s, "||") == 0) ||
			(strcasecmp(s, "|") == 0) ||
			(strcasecmp(s, _("or")) == 0))
		{
			++it;
			if(unlikely(it == a.end()))
				break;
			s = it->c_str();
		}
		if(unlikely(!getRedundantFlagAtom(s, type, p.first)))
			break;
		++it;
		if(likely(it == a.end()))
			return;
		break;
	}

	cerr << eix::format(_(
		"%s has unknown value %r\n"
		"\tassuming value 'all-installed' instead."))
		% key % value << endl;

	getRedundantFlagAtom("all-installed", type, p.first);
	getRedundantFlagAtom(NULL, type, p.second);
}

unsigned int
EixRc::getInteger(const string &key)
{
	return my_atoi((*this)[key].c_str());
}

string
EixRc::as_comment(const string &s)
{
	string ret(s);
	string::size_type pos(0);
	while(pos = ret.find("\n", pos), likely(pos != string::npos)) {
		ret.insert(pos + 1, "# ");
		pos += 2;
	}
	return ret;
}

void
EixRc::dumpDefaults(FILE *s, bool use_defaults)
{
	string message(use_defaults ?
		_("was locally changed to:") :
		_("changed locally, default was:"));
	for(vector<EixRcOption>::size_type i(0); likely(i < defaults.size()); ++i) {
		const char *typestring("UNKNOWN");
		switch(defaults[i].type) {
			case EixRcOption::BOOLEAN:
				typestring = "BOOLEAN";
				break;
			case EixRcOption::STRING:
				typestring = "STRING";
				break;
			case EixRcOption::PREFIXSTRING:
				typestring = "PREFIXSTRING";
				break;
			case EixRcOption::INTEGER:
				typestring = "INTEGER";
				break;
			case EixRcOption::LOCAL:
				typestring = NULL;
				break;
			default:
				break;
		}
		const char *key(defaults[i].key.c_str());
		const char *value(defaults[i].local_value.c_str());
		if(unlikely(typestring == NULL)) {
			fprintf(s, "# %s\n%s='%s'\n\n",
				_("locally added:"),
				key, value);
			continue;
		}
		const char *deflt(defaults[i].value.c_str());
		const char *output(use_defaults ? deflt : value);
		const char *comment(use_defaults ? value : deflt);
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
				message.c_str(),
				key,
				as_comment(comment).c_str());
		}
	}
}

void
EixRc::print_var(const string &key)
{
	string print_append((*this)["PRINT_APPEND"]);
	unescape_string(print_append);
	if(likely(key != "PORTDIR")) {
		const char *s(cstr(key));
		if(likely(s != NULL)) {
			cout << s << print_append;
			return;
		}
	}
	PortageSettings ps(*this, false, true);
	cout << ps[key] << print_append;
}

