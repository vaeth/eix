// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "version.h"

using namespace std;

const IUse::Flags
	IUse::USEFLAGS_NIL,
	IUse::USEFLAGS_NORMAL,
	IUse::USEFLAGS_PLUS,
	IUse::USEFLAGS_MINUS;

IUse::Flags
IUse::split(string &s)
{
	Flags ret = USEFLAGS_NIL;
	string::size_type c;
	for(c = 0; c < s.length(); ++c) {
		switch(s[c]) {
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
	if(!c)
		return USEFLAGS_NORMAL;
	s.erase(0, c);
	if(ret == USEFLAGS_NIL)
		return USEFLAGS_NORMAL;
	return ret;
}

string
IUse::asString() const
{
	string prefix;
	switch(flags) {
		case USEFLAGS_PLUS:
			prefix = "+";
			break;
		case USEFLAGS_MINUS:
			prefix = "-";
			break;
		case USEFLAGS_NORMAL|USEFLAGS_PLUS:
			prefix = "(+)";
			break;
		case USEFLAGS_NORMAL|USEFLAGS_MINUS:
			prefix = "(-)";
			break;
		case USEFLAGS_PLUS|USEFLAGS_MINUS:
			prefix = "+-";
			break;
		case USEFLAGS_NORMAL|USEFLAGS_PLUS|USEFLAGS_MINUS:
			prefix = "(+-)";
			break;
		default:
		//case USEFLAGS_NIL:
		//case USEFLAGS_NORMAL:
			return name();
	}
	return prefix + name();
}

string
IUseSet::asString() const
{
	if(m_cached_iuse.empty() && ! m_iuse.empty()) {
		for(set<IUse>::const_iterator it = m_iuse.begin();
			it != m_iuse.end(); ++it) {
			if(!m_cached_iuse.empty())
				m_cached_iuse.append(" ");
			m_cached_iuse.append(it->asString());
		}
	}
	return m_cached_iuse;
}

void
IUseSet::insert(const std::set<IUse> &iuse)
{
	for(set<IUse>::const_iterator it = iuse.begin();
		it != iuse.end(); ++it)
		insert(*it);
	cacheString();
}

void
IUseSet::insert(const string &iuse)
{
	vector<string> vec = split_string(iuse);
	for(vector<string>::const_iterator it = vec.begin();
		it != vec.end(); ++it)
		insert_fast(*it);
	cacheString();
}

void
IUseSet::insert(const IUse &iuse)
{
	set<IUse>::iterator it = m_iuse.find(iuse);
	if(it == m_iuse.end()) {
		m_iuse.insert(iuse);
		return;
	}
	IUse::Flags oriflags = it->flags;
	IUse::Flags newflags = oriflags | (iuse.flags);
	if(newflags == oriflags)
		return;
	m_iuse.erase(it);
	m_iuse.insert(IUse(iuse.name(), newflags));
}

const Version::EffectiveState
	Version::EFFECTIVE_UNSAVED,
	Version::EFFECTIVE_USED,
	Version::EFFECTIVE_UNUSED;

void
Version::modify_effective_keywords(const string &modify_keys)
{
	if(effective_state == EFFECTIVE_UNUSED) {
		if(!modify_keywords(effective_keywords, full_keywords, modify_keys))
			return;
	}
	else if(!modify_keywords(effective_keywords, effective_keywords, modify_keys))
		return;
	if(effective_keywords == full_keywords) {
		effective_state = EFFECTIVE_UNUSED;
		effective_keywords.clear();
	}
	else
		effective_state = EFFECTIVE_USED;
}
