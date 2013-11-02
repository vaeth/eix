// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <string>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringlist.h"
#include "eixTk/stringtypes.h"

void StringListContent::finalize() {
	if(likely(m_list.empty())) {
		return;
	}
	if(unlikely(m_list[0].empty()) || unlikely(m_list[m_list.size() - 1].empty())) {
		WordVec cp;
		WordVec::const_iterator it(m_list.begin());
		while(unlikely(it->empty())) {
			if(unlikely((++it) == m_list.end())) {
				m_list.clear();
				return;
			}
		}
		for(; likely(it != m_list.end()); ++it) {
			for(WordVec::const_iterator r(it); unlikely(r->empty()); ) {
				if(unlikely((++r) == m_list.end())) {
					m_list = cp;
					return;
				}
			}
			cp.push_back(*it);
		}
		m_list = cp;
	}
}

#ifdef STRINGLIST_FREE
StringList& StringList::operator=(const StringList& s) {
	ptr = s.ptr;
	if(ptr != NULLPTR) {
		++(ptr->usage);
	}
	return *this;
}

StringList::~StringList() {
	if(ptr != NULLPTR) {
		if(--(ptr->usage) == 0) {
			delete ptr;
		}
	}
}
#endif

void StringList::finalize() {
	if(ptr != NULLPTR) {
		ptr->finalize();
		if(ptr->empty()) {
			delete ptr;
			ptr = NULLPTR;
		}
	}
}

void StringList::push_back(const std::string& s) {
	if(ptr == NULLPTR) {
		ptr = new StringListContent;
#ifdef STRINGLIST_FREE
		ptr->usage = 1;
#endif
	}
	ptr->push_back(s);
}
