// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "eixTk/varsreader.h"
#include "eixrc/eixrc.h"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

#define EIX_SYSTEMRC SYSCONFDIR"/eixrc"

#define EIX_USERRC   "/.eixrc"

using std::map;
using std::set;
using std::string;
using std::vector;

using std::cerr;
using std::endl;

const EixRc::DelayvarFlags
	EixRc::DELAYVAR_NONE,
	EixRc::DELAYVAR_STAR,
	EixRc::DELAYVAR_ESCAPE,
	EixRc::DELAYVAR_APPEND;

static void override_by_env(map<string, string> *m) ATTRIBUTE_NONNULL_;

eix::SignedBool EixRc::getBoolText(const string& key, const char *text) {
	const string& s((*this)[key]);
	if(casecontains(s, text)) {
		return -1;
	}
	return (istrue(s.c_str()) ? 1 : 0);
}

eix::TinySigned EixRc::getTinyTextlist(const string& key, const char *const *text) {
	const string& s((*this)[key]);
	for(eix::TinySigned i(-1); likely(*text != NULLPTR); ++text, --i) {
		if(casecontains(s, *text)) {
			return i;
		}
	}
	return (istrue(s.c_str()) ? 1 : 0);
}

bool EixRc::getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom *r) {
	r->only &= ~type;
	if(unlikely(s == NULLPTR)) {
		r->red &= ~type;
		return true;
	}
	if(*s == '+') {
		++s;
		r->only |= type;
		r->oins |= type;
	} else if(*s == '-') {
		++s;
		r->only |= type;
		r->oins &= ~type;
	}
	if(casecontains(s, "no") || casecontains(s, "false")) {
		r->red &= ~type;
	} else if(casecontains(s, "all")) {
		if(casecontains(s, "installed")) {
			if(casecontains(s, "un")) {
				r->red |= type;
				r->all |= type;
				r->spc |= type;
				r->ins &= ~type;
			} else {
				r->red |= type;
				r->all |= type;
				r->spc |= type;
				r->ins |= type;
			}
		} else {
			r->red |= type;
			r->all |= type;
			r->spc &= ~type;
		}
	} else if(casecontains(s, "some")) {
		if(casecontains(s, "installed")) {
			if(casecontains(s, "un")) {
				r->red |= type;
				r->all &= ~type;
				r->spc |= type;
				r->ins &= ~type;
			} else {
				r->red |= type;
				r->all &= ~type;
				r->spc |= type;
				r->ins |= type;
			}
		} else {
			r->red |= type;
			r->all &= ~type;
			r->spc &= ~type;
		}
	} else {
		return false;
	}
	return true;
}

LocalMode EixRc::getLocalMode(const string& key) {
	const char *s((*this)[key].c_str());
	if((*s == '-') || casecontains(s, "non-local") ||
		casecontains(s, "nonlocal") ||
		casecontains(s, "global") ||
		casecontains(s, "original")) {
		return LOCALMODE_NONLOCAL;
	}
	if((*s == '+') || casecontains(s, "local")) {
		return LOCALMODE_LOCAL;
	}
	return LOCALMODE_DEFAULT;
}

const char *EixRc::cstr(const string& key) const {
	my_map::const_iterator s(main_map.find(key));
	if(s == main_map.end()) {
		return NULLPTR;
	}
	return (s->second).c_str();
}

const char *EixRc::prefix_cstr(const string& key) const {
	const char *s(cstr(key));
	if(unlikely(s == NULLPTR)) {
		return NULLPTR;
	}
	if(s[0]) {
		return s;
	}
	// Maybe later: Test whether some eprefix-variable is set,
	// and return "" instead of NULLPTR in this case.
	return NULLPTR;
}

void EixRc::read() {
	const char *name("EIX_PREFIX");
	const char *eixrc_prefix(getenv(name));
	if(eixrc_prefix) {
		m_eprefixconf = eixrc_prefix;
	} else {
		name = "PORTAGE_CONFIGROOT";
		eixrc_prefix = getenv(name);
		if(eixrc_prefix) {
			m_eprefixconf = eixrc_prefix;
		} else {
			name = "EIX_PREFIX";
			m_eprefixconf = EIX_PREFIX_DEFAULT;
		}
	}
	modify_value(&m_eprefixconf, name);

	set<string> has_delayed;

	// First, we create defaults and main_map with all variables
	// (including all values required by delayed references).
	read_undelayed(&has_delayed);

	// Resolve delayed references recursively.
	for(default_index i(0); i < defaults.size(); ++i)
		resolve_delayed(defaults[i].key, &has_delayed);

	// set m_eprefixconf to possibly new settings:
	m_eprefixconf = (*this)["PORTAGE_CONFIGROOT"];
}

const string& EixRc::operator[](const string& key) {
	my_map::const_iterator it(main_map.find(key));
	if(it != main_map.end())
		return it->second;
	add_later_variable(key);
	return main_map[key];
}

/**
This will fetch a variable which was not set in the
defaults (or its modification or its delayed references),
i.e. it must be fetched from the config or ENV setting.
Of course, it will be resolved for delayed substitutions,
and delayed references are also be added similarly.
**/
void EixRc::add_later_variable(const string& key) {
	set<string> has_delayed;
	join_key(key, &has_delayed, true, NULLPTR);
	resolve_delayed(key, &has_delayed);
}

void EixRc::resolve_delayed(const string& key, set<string> *has_delayed) {
	set<string> visited;
	const char *errtext;
	string errvar;
	if(unlikely(resolve_delayed_recurse(key, &visited, has_delayed,
		&errtext, &errvar) == NULLPTR)) {
		cerr << eix::format(_(
			"fatal config error: %s in delayed substitution of %s"))
			% errtext % errvar << endl;
		exit(EXIT_FAILURE);
	}
}

string *EixRc::resolve_delayed_recurse(const string& key, set<string> *visited, set<string> *has_delayed, const char **errtext, string *errvar) {
	string *value(&(main_map[key]));
	if(has_delayed->find(key) == has_delayed->end()) {
		modify_value(value, key);
		return value;
	}
	string::size_type pos(0);
	for(;;) {
		string::size_type length;
		DelayvarFlags varflags;
		string varname, append;
		DelayedType type(find_next_delayed(*value, &pos, &length, &varname, &varflags, &append));
		bool will_test(false);
		switch(type) {
			case DelayedNotFound:
				has_delayed->erase(key);
				modify_value(value, key);
				return value;
			case DelayedFi:
				*errtext = _("FI without IF");
				*errvar = key;
				return NULLPTR;
			case DelayedElse:
				*errtext = _("ELSE without IF");
				*errvar = key;
				return NULLPTR;
			case DelayedQuote:
				pos += length - 1;
				value->erase(pos);
				continue;
			case DelayedIfTrue:
			case DelayedIfFalse:
			case DelayedIfEmpty:
			case DelayedIfNonempty:
				will_test = true;
			default:
				break;
		}
		if(unlikely(visited->find(key) != visited->end())) {
			*errtext = _("self-reference");
			*errvar = key;
			return NULLPTR;
		}
		visited->insert(key);
		const string *s(resolve_delayed_recurse(
			(((varflags & DELAYVAR_STAR) != DELAYVAR_NONE) ?
				(varprefix + varname) : varname),
			visited, has_delayed, errtext, errvar));
		visited->erase(key);
		if(unlikely(s == NULLPTR)) {
			return NULLPTR;
		}
		string local_copy;
		if(unlikely((varflags & DELAYVAR_ESCAPE) != DELAYVAR_NONE)) {
			local_copy = *s;
			s = &local_copy;
			escape_string(&local_copy);
		}
		if(unlikely((varflags & DELAYVAR_APPEND) != DELAYVAR_NONE)) {
			if(s != &local_copy) {
				local_copy = *s;
				s = &local_copy;
			}
			string::size_type add(append.size() + 1);
			for(string::size_type curr(0);
				(curr = (local_copy.find('|', curr))) != string::npos;
				curr += add) {
				local_copy.insert(curr, append);
			}
			local_copy.append(append);
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
		} else {  // ((type == DelayedIfEmpty) || (type == DelayedIfNonempty))
			result = s->empty();
			if(type == DelayedIfNonempty)
				result = !result;
		}
		string::size_type delpos(string::npos);
		if(result) {
			value->erase(skippos, length);
		} else {
			delpos = skippos;
			skippos += length;
		}
		bool gotelse(false);
		unsigned int curr_count(0);
		for(;; skippos += length) {
			type = find_next_delayed(*value, &skippos, &length, NULLPTR, NULLPTR, NULLPTR);
			switch(type) {
				case DelayedFi:
					if(curr_count != 0) {
						--curr_count;
						continue;
					}
					if(delpos == string::npos) {
						value->erase(skippos, length);
					} else {
						value->erase(delpos,
							(skippos + length) - delpos);
					}
					break;
				case DelayedElse:
					if(curr_count != 0)
						continue;
					if(unlikely(gotelse)) {
						*errtext = _("double ELSE");
						*errvar = key;
						return NULLPTR;
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
					return NULLPTR;
				default:
					continue;
			}
			break;
		}
	}
}

static void override_by_env(map<string, string> *m) {
	for(map<string, string>::iterator it(m->begin()); likely(it != m->end()); ++it) {
		char *val(getenv((it->first).c_str()));
		if(unlikely(val != NULLPTR))
			it->second = string(val);
	}
}

/**
Create defaults and the main_map with all variables
(including all values required by delayed references).
@arg has_delayed is initialized to corresponding keys
**/
void EixRc::read_undelayed(set<string> *has_delayed) {
	// Initialize with the default variables
	for(default_index i(0); likely(i < defaults.size()); ++i)
		filevarmap[defaults[i].key] = defaults[i].value;

	// override with ENV
	override_by_env(&filevarmap);

	VarsReader rc(  // VarsReader::NONE
			VarsReader::SUBST_VARS
			|VarsReader::ALLOW_SOURCE_VARNAME
			|VarsReader::INTO_MAP
			|VarsReader::RECURSE);
	rc.useMap(&filevarmap);
	rc.setPrefix("EIXRC_SOURCE");

	const char *rc_file(getenv("EIXRC"));
	string errtext;
	if(unlikely(rc_file != NULLPTR)) {
		if(unlikely(!rc.read(rc_file, &errtext, true))) {
			cerr << errtext << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		// override with EIX_SYSTEMRC
		if(unlikely(!rc.read((m_eprefixconf + EIX_SYSTEMRC).c_str(), &errtext, true))) {
			cerr << errtext << endl;
			exit(EXIT_FAILURE);
		}

		// override with EIX_USERRC
		char *home(getenv("HOME"));
		if(unlikely(home == NULLPTR)) {
			cerr << _("no $HOME found in environment.") << endl;
		} else {
			string eixrc(home);
			eixrc.append(EIX_USERRC);
			if(unlikely(!rc.read(eixrc.c_str(), &errtext, true))) {
				cerr << errtext << endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	// override with ENV
	override_by_env(&filevarmap);

	// set WIDETERM
	string& wide(filevarmap["WIDETERM"]);
	if(wide.empty() || (wide == "auto")) {
		string& c(filevarmap["COLUMNS"]);
		if(c.empty() || (c == "auto")) {
			unsigned int lines, columns;
			if(get_geometry(&lines, &columns)) {
				wide = ((columns > 80) ? "true" : "false");
			} else {
				wide.clear();
			}
		} else {
			wide = ((my_atoi(c.c_str()) > 80) ? "true" : "false");
		}
	}

	// Set new values as default and for printing with --dump.
	set<string> original_defaults;
	for(vector<EixRcOption>::iterator it(defaults.begin());
		likely(it != defaults.end()); ++it) {
		string *value(&filevarmap[it->key]);
		modify_value(value, it->key);
		it->local_value = *value;
		original_defaults.insert(it->key);
	}
	// Since join_key_if_new modifies the defaults vector, our loop
	// must go over a copy of the keys and not over the defaults vector.
	for(set<string>::const_iterator it(original_defaults.begin());
		likely(it != original_defaults.end()); ++it) {
		join_key(*it, has_delayed, false, &original_defaults);
	}
}

/**
Recursively eval and join key and its delayed references to
main_map and default; set has_delayed if appropriate
**/
void EixRc::join_key(const string& key, set<string> *has_delayed, bool add_top_to_defaults, const set<string> *exclude_defaults) {
	string *val(&main_map[key]);
	my_map::const_iterator f(filevarmap.find(key));
	if(unlikely(f != filevarmap.end())) {
	/*
	Note that if a variable is defined in a file and in ENV,
	its value was already overridden from ENV.
	*/
		*val = f->second;
	} else {
	// If it was not defined in a file, it might be in ENV anyway:
		char *envval(getenv(key.c_str()));
		if(unlikely(envval != NULLPTR))
			*val = string(envval);
	}
	/*
	For the case that some day e.g. prefix_keys (variables with
	PREFIXSTRING) should possibly also allow to contain local variables,
	better modify it:
	*/
	modify_value(val, key);

	if(unlikely(add_top_to_defaults)) {
		if(unlikely((exclude_defaults == NULLPTR) || (exclude_defaults->find(key) == exclude_defaults->end())))
			defaults.push_back(EixRcOption(key, *val));
	}
	join_key_rec(key, *val, has_delayed, exclude_defaults);
}

void EixRc::join_key_rec(const string& key, const string& val, set<string> *has_delayed, const set<string> *exclude_defaults) {
	string::size_type pos(0);
	string::size_type length;
	for(;; pos += length) {
		DelayvarFlags varflags;
		string varname;
		switch(find_next_delayed(val, &pos, &length, &varname, &varflags, NULLPTR)) {
			case DelayedNotFound:
				return;
			case DelayedVariable:
			case DelayedIfTrue:
			case DelayedIfFalse:
				break;
			default:
				has_delayed->insert(key);
				continue;
		}
		has_delayed->insert(key);
		if(unlikely((varflags & DELAYVAR_STAR) != DELAYVAR_NONE)) {
			static CONSTEXPR const char *prefixlist[] = {
				EIX_VARS_PREFIX,
				DIFF_VARS_PREFIX,
				UPDATE_VARS_PREFIX,
				DROP_VARS_PREFIX,
				NULLPTR
			};
			for(const char *const *prefix = prefixlist;
				*prefix != NULLPTR; ++prefix) {
				join_key_if_new(string(*prefix) + varname,
					has_delayed, exclude_defaults);
			}
		} else {
			join_key_if_new(varname, has_delayed, exclude_defaults);
		}
	}
}

void EixRc::join_key_if_new(const string& key, set<string> *has_delayed, const set<string> *exclude_defaults) {
	if(unlikely(main_map.find(key) == main_map.end())) {
		join_key(key, has_delayed, true, exclude_defaults);
	}
}

EixRc::DelayedType EixRc::find_next_delayed(const string& str, string::size_type *posref, string::size_type *length, string *varname, DelayvarFlags *varflags, string *append) {
	string::size_type pos(*posref);
	for(;; pos += 2) {
		pos = str.find("%{", pos);
		if(pos == string::npos) {
			return DelayedNotFound;
		}
		string::size_type i(pos + 2);
		if(i >= str.length()) {
			return DelayedNotFound;
		}
		DelayedType type;
		char c(str[i++]);
		bool findvar(true);
		switch(c) {
			case '}':
				type = DelayedFi;
				findvar = false;
				break;
			case '%':
				type = DelayedQuote;
				findvar = false;
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
				} else {
					type = DelayedIfTrue;
				}
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
				} else {
					type = DelayedIfFalse;
				}
				break;
			default:
				type = DelayedVariable;
		}
		if(findvar) {
			bool headsymbols(true);
			DelayvarFlags flags(DELAYVAR_NONE);
			string::size_type varstart(i - 1);
			for(;; c = str[i++]) {
				if(headsymbols) {
					switch(c) {
						case '*':
							flags |= DELAYVAR_STAR;
							varstart = i;
							continue;
						case '\\':
							flags |= DELAYVAR_ESCAPE;
							varstart = i;
							continue;
						default:
							headsymbols = false;
							break;
					}
				}
				if((!my_isalnum(c)) && (c != '_')) {
					break;
				}
				if(i >= str.length()) {
					return DelayedNotFound;
				}
			}
			string::size_type appendstart, appendlen, varlen;
			if(unlikely((type == DelayedVariable) &&
				((c == ',') || (c == ';')))) {
				string::size_type j(str.find('}', i));
				if(j == string::npos) {
					continue;
				}
				flags |= DELAYVAR_APPEND;
				appendstart = --i;
				varlen = i - varstart;
				appendlen = j - appendstart;
				i = j + 1;
			} else {
				if(unlikely(c != '}')) {
					continue;
				}
				varlen = i - varstart - 1;
			}
			if(caseequal(str.substr(pos + 2, i - pos - 3), "else")) {
				type = DelayedElse;
			} else {
				if(varlen == 0) {
					continue;
				}
				if(varname != NULLPTR) {
					varname->assign(str, varstart, varlen);
				}
				if(varflags != NULLPTR) {
					*varflags = flags;
				}
				if(unlikely(append != NULLPTR)) {
					if(unlikely((flags & DELAYVAR_APPEND) != DELAYVAR_NONE)) {
						append->assign(str, appendstart, appendlen);
					}
				}
			}
		}
		*posref = pos;
		*length = i - pos;
		return type;
	}
}

void EixRc::modify_value(string *value, const string& key) {
	if(*value == "/") {
		if(prefix_keys.find(key) != prefix_keys.end()) {
			value->clear();
		}
	}
}

void EixRc::clear() {
	defaults.clear();
	prefix_keys.clear();
	filevarmap.clear();
	main_map.clear();
}

void EixRc::addDefault(EixRcOption option) {
	if(unlikely(option.type == EixRcOption::PREFIXSTRING)) {
		prefix_keys.insert(option.key);
	}
	modify_value(&(option.value), option.key);
	defaults.push_back(option);
}

bool EixRc::istrue(const char *s) {
	switch(s[0]) {
		case 0:
		case 'N':
		case 'n':
		case 'f':
		case 'F':
		case '0':
		case '-':
			return false;
		case 'o':
		case 'O':
			switch(s[1]) {
				case 'f':
				case 'F':
					return false;
				default:
					break;
			}
		default:
			break;
	}
	return true;
}

void EixRc::getRedundantFlags(const string& key, Keywords::Redundant type, RedPair *p) {
	const string& value((*this)[key]);
	vector<string> a;
	split_string(&a, value);

	for(vector<string>::iterator it(a.begin()); likely(it != a.end()); ) {
		// a dummy loop for break on error
		if(unlikely(!getRedundantFlagAtom(it->c_str(), type, &(p->first)))) {
			break;
		}
		++it;
		if(it == a.end()) {
			getRedundantFlagAtom(NULLPTR, type, &(p->second));
			return;
		}
		const char *s(it->c_str());
		if(caseequal(*it, "or") ||
			(strcmp(s, "||") == 0) ||
			(strcmp(s, "|") == 0)) {
			++it;
			if(unlikely(it == a.end())) {
				break;
			}
		}
		if(unlikely(!getRedundantFlagAtom(s, type, &(p->first)))) {
			break;
		}
		++it;
		if(likely(it == a.end())) {
			return;
		}
		break;
	}

	cerr << eix::format(_(
		"%s has unknown value \"%s\"\n"
		"\tassuming value \"all-installed\" instead."))
		% key % value << endl;

	getRedundantFlagAtom("all-installed", type, &(p->first));
	getRedundantFlagAtom(NULLPTR, type, &(p->second));
}

unsigned int EixRc::getInteger(const string& key) {
	return my_atoi((*this)[key].c_str());
}

string EixRc::as_comment(const string& s) {
	string ret(s);
	string::size_type pos(0);
	while(pos = ret.find('\n', pos), likely(pos != string::npos)) {
		ret.insert(pos + 1, "# ");
		pos += 2;
	}
	return ret;
}

void EixRc::dumpDefaults(FILE *s, bool use_defaults) {
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
				typestring = NULLPTR;
				break;
			default:
				break;
		}
		const char *key(defaults[i].key.c_str());
		string value(defaults[i].local_value);
		escape_string(&value, doublequotes);
		string deflt(defaults[i].value);
		escape_string(&deflt, doublequotes);
		if(unlikely(typestring == NULLPTR)) {
			fprintf(s, "# %s\n%s=\"%s\"\n\n",
				_("locally added:"),
				key, value.c_str());
			continue;
		}
		const string& output(use_defaults ? deflt : value);
		const string& comment(use_defaults ? value : deflt);

		fprintf(s,
				"# %s\n"
				"# %s\n"
				"%s=\"%s\"\n",
				as_comment(typestring).c_str(),
				as_comment(defaults[i].description.c_str()).c_str(),
				key,
				output.c_str());
		if(deflt == value) {
			putc('\n', s);
		} else {
			fprintf(s,
				"# %s\n"
				"# %s=\"%s\"\n\n",
				message.c_str(),
				key,
				as_comment(comment).c_str());
		}
	}
}
