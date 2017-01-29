// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/compare.h"
#include <config.h>

#include <string>

#include "eixTk/eixint.h"

using std::string;

eix::SignedBool eix::numeric_compare(const string& left, const string& right) {
	// strip leading 0's
	string::size_type lstart(left.find_first_not_of('0'));
	string::size_type rstart(right.find_first_not_of('0'));
	// Special cases: number is 0 or string is empty
	if(lstart == string::npos) {
		if(rstart == string::npos) {
			return 0;
		}
		return -1;
	}
	if(rstart == string::npos) {
		return 1;
	}

	// check if one is longer, that one would be bigger
	eix::SignedBool size_result(eix::default_compare(left.size() - lstart, right.size() - rstart));
	if(size_result != 0) {
		return size_result;
	}
	// both strings have the same length, do string comparison
	return eix::toSignedBool(left.compare(lstart, string::npos, right, rstart, string::npos));
}
