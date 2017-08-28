// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/version.h"
#include <config.h>

#include <string>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringlist.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"

using std::string;

const IUse::Flags
	IUse::USEFLAGS_NIL,
	IUse::USEFLAGS_NORMAL,
	IUse::USEFLAGS_PLUS,
	IUse::USEFLAGS_MINUS;

IUse::Flags IUse::parse(string *s) {
	Flags ret(USEFLAGS_NIL);
	string::size_type c(0);
	for( ; likely(c < s->length()); ++c) {
		switch((*s)[c]) {
			case '+':
				ret |= USEFLAGS_PLUS;
				continue;
			case '-':
				ret |= USEFLAGS_MINUS;
				continue;
			case '[':
			case '{':
			case '(':
				ret |= USEFLAGS_NORMAL;
				continue;
			case ']':
			case '}':
			case ')':
			case ' ':
				continue;
			default:
				break;
		}
		break;
	}
	if(c == 0)
		return USEFLAGS_NORMAL;
	s->erase(0, c);
	if(ret == USEFLAGS_NIL)
		return USEFLAGS_NORMAL;
	return ret;
}

const char *IUse::prefix() const {
/*
	For the case that you want to make prefixes/postfixes(?) customizable,
	you might need to do this independently of this function.
	This function is used to calculate the strings stored in the cachefile,
	so each change modifies the cachefile format.
	The corresponding function for reading the cachefile (or the string
	passed from the ebuild) is parse() which is intentionally a bit more
	sloppy about the syntax; so certain minor changes of the prefixes of
	the cachefile format do not harm.
*/
	switch(flags) {
		case USEFLAGS_PLUS:
			return "+";
		case USEFLAGS_MINUS:
			return "-";
		case USEFLAGS_NORMAL|USEFLAGS_PLUS:
			return "(+)";
		case USEFLAGS_NORMAL|USEFLAGS_MINUS:
			return "(-)";
		case USEFLAGS_PLUS|USEFLAGS_MINUS:
			return "+-";
		case USEFLAGS_NORMAL|USEFLAGS_PLUS|USEFLAGS_MINUS:
			return "(+-)";
		default:
		// case USEFLAGS_NIL:
		// case USEFLAGS_NORMAL:
			return NULLPTR;
	}
}

string IUse::asString() const {
	const char *p(prefix());
	if(p == NULLPTR) {
		return name();
	}
	string ret(p);
	ret.append(name());
	return ret;
}

string IUseSet::asString() const {
	string ret;
	for(IUseStd::const_iterator it(m_iuse.begin());
		likely(it != m_iuse.end()); ++it) {
		if(!ret.empty())
			ret.append(1, ' ');
		ret.append(it->asString());
	}
	return ret;
}

WordVec IUseSet::asVector() const {
	WordVec ret(m_iuse.size());
	WordVec::size_type i(0);
	for(IUseStd::const_iterator it(m_iuse.begin());
		likely(it != m_iuse.end()); ++i, ++it) {
		ret[i] = it->asString();
	}
	return ret;
}

void IUseSet::insert(const IUseStd& iuse) {
	for(IUseStd::const_iterator it(iuse.begin());
		likely(it != iuse.end()); ++it) {
		insert(*it);
	}
}

void IUseSet::insert(const string& iuse) {
	WordVec vec;
	split_string(&vec, iuse);
	for(WordVec::const_iterator it(vec.begin());
		likely(it != vec.end()); ++it) {
		insert_fast(*it);
	}
}

void IUseSet::insert(const IUse& iuse) {
	IUseStd::iterator it(m_iuse.find(iuse));
	if(it == m_iuse.end()) {
		m_iuse.insert(iuse);
		return;
	}
	IUse::Flags oriflags(it->flags);
	IUse::Flags newflags(oriflags | (iuse.flags));
	if(newflags == oriflags) {
		return;
	}
	m_iuse.erase(it);
	m_iuse.insert(IUse(iuse.name(), newflags));
}

const Version::EffectiveState
	Version::EFFECTIVE_UNSAVED,
	Version::EFFECTIVE_USED,
	Version::EFFECTIVE_UNUSED;

bool Version::use_required_use;

Version::Version() :
	effective_state(EFFECTIVE_UNUSED) {
	have_saved_keywords.fill(false);
	have_saved_masks.fill(false);
	states_effective.fill(EFFECTIVE_UNSAVED);
}

void Version::modify_effective_keywords(const string& modify_keys) {
	if(effective_state == EFFECTIVE_UNUSED) {
		if(!modify_keywords(&effective_keywords, full_keywords, modify_keys)) {
			return;
		}
	} else if(!modify_keywords(&effective_keywords, effective_keywords, modify_keys)) {
		return;
	}
	if(likely(effective_keywords == full_keywords)) {
		effective_state = EFFECTIVE_UNUSED;
		effective_keywords.clear();
	} else {
		effective_state = EFFECTIVE_USED;
	}
}

void Version::add_accepted_keywords(const std::string& accepted_keywords) {
	if(!m_accepted_keywords.empty()) {
		m_accepted_keywords.append(" ");
	}
	m_accepted_keywords.append(accepted_keywords);
}

void Version::add_reason(const StringList& reason) {
	if(reason.empty()) {
		return;
	}
	if(reasons.count(reason) != 0) {
		return;
	}
	reasons.insert(reason);
}
