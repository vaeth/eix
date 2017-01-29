// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "portage/packagesets.h"
#include <config.h>

#include <algorithm>

/**
@return true if something has changed
**/
bool SetsList::add_system() {
	if(have_system) {
		return false;
	}
	return (have_system = true);
}

bool SetsList::has(SetsIndex i) const {
	return (std::find(begin(), end(), i) != end());
}

/**
@return true if something has changed
**/
bool SetsList::add(SetsIndex i) {
	if(has(i)) {
		return false;
	}
	push_back(i);
	return true;
}

/**
@return true if something has changed
**/
bool SetsList::add(const SetsList& l) {
	bool r(false);
	if(l.has_system()) {
		if(add_system()) {
			r = true;
		}
	}
	for(SetsList::const_iterator it(l.begin()); it != l.end(); ++it) {
		if(add(*it)) {
			r = true;
		}
	}
	return r;
}

void SetsList::clear() {
	super::clear();
	have_system = false;
}
