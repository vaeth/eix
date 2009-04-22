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

const Version::EffectiveState
	Version::EFFECTIVE_UNSAVED,
	Version::EFFECTIVE_USED,
	Version::EFFECTIVE_UNUSED;

const string&
Version::iuse() const
{
	if(m_cached_iuse.empty() && ! m_iuse.empty())
		m_cached_iuse = join_vector(m_iuse);
	return m_cached_iuse;
}

void
Version::set_iuse(const string &i)
{
	m_iuse = split_string(i);
	for(vector<string>::iterator it = m_iuse.begin();
		it != m_iuse.end(); ) {
		// remove all leading +- symbols,
		// but guarantee nonempty strings (needed for proper hashing)
		string::size_type s = it->find_first_not_of("+-");
		if(s == string::npos) {
			m_iuse.erase(it);
			it = m_iuse.begin();
			continue;
		}
		if(s)
			it->erase(0, s);
		++it;
	}
	sort_uniquify(m_iuse);
	m_cached_iuse.clear(); // invalided cache
}

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
