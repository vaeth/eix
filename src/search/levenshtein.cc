// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Martin Väth <martin@mvath.de>

#include "search/levenshtein.h"
#include <config.h>  // IWYU pragma: keep

#include <sys/types.h>

#include <cstring>

#include <algorithm>
#include <utility>  // std::swap since C++11
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/stringutils.h"

using std::vector;

using std::min;
using std::swap;

/**
Calculates the Levenshtein distance of two strings
@param str_a string a
@param str_b string b
@return Levenshtein distance of strings a and b
**/
Levenshtein get_levenshtein_distance(const char *str_a, const char *str_b) {
	// We save space https://de.wikipedia.org/wiki/Hirschberg-Algorithmus
	size_t n(std::strlen(str_a));
	size_t m(std::strlen(str_b));
	if(n > m) {
		swap(n, m);
		swap(str_a, str_b);
	}
	if(n == 0) {
		return m;
	}

	vector<Levenshtein> arr(n + 1);
	for(Levenshtein i(0); likely(i <= n); ++i) {
		arr[i] = i;  // start with 0 and add 1 (insert) for each char
	}
	for(; m > 0; ++str_a, --m) {
		vector<Levenshtein>::iterator it(arr.begin());
		Levenshtein sub((*it)++);  // add 1 (delete *str_a)
		for(size_t i(0); i < n; ++i) {
			Levenshtein c(*it);
			// +1: insert str_b[i] or delete *str_a
			c = min(c, *(++it)) + 1;
			if (str_b[i] != *str_a) {
				++sub;  // +1: Substitute
			}
			if (c > sub) {
				c = sub;
			}
			sub = *it;
			*it = c;
		}
	}
	return arr[n];
}
