// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Martin Väth <martin@mvath.de>

#include "search/levenshtein.h"
#include <config.h>

#include <sys/types.h>

#include <cstring>

#include <algorithm>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/stringutils.h"

using std::vector;

using std::min;

/**
Calculates the Levenshtein distance of two strings
@param str_a string a
@param str_b string b
@return int Levenshtein distance of str_a <> str_b
**/
Levenshtein get_levenshtein_distance(const char *str_a, const char *str_b) {
	size_t n(std::strlen(str_a));
	size_t m(std::strlen(str_b));
	if(n == 0) {
		return m;
	}
	if(m == 0) {
		return n;
	}

	typedef vector<Levenshtein> LevVec;
	typedef vector<LevVec> LevArr;

	// initialize the matrix' dimensions
	LevArr matrix(n + 1);
	matrix[0].resize(m + 1);
	for(LevArr::size_type i(1); likely(i <= n); ++i) {
		matrix[i].resize(m + 1);
		fill(matrix[i].begin(), matrix[i].end(), 0);
	}

	// fill the 1st column/row with str_a / str_b
	for(LevArr::size_type i(0); likely(i <= n); ++i) {
		matrix[i][0] = i;
	}
	for(LevVec::size_type j(0); likely(j <= m); ++j) {
		matrix[0][j] = j;
	}

	// calculate the matrix
	for(LevArr::size_type i(1); likely(i <= n); ++i) {
		for(LevVec::size_type j(1); likely(j <= m); ++j) {
			char ci(my_tolower(str_a[i - 1]));
			char cj(my_tolower(str_b[j - 1]));

			Levenshtein a(matrix[i - 1][j] + 1);
			Levenshtein b(matrix[i][j - 1] + 1);
			Levenshtein c(matrix[i - 1][j - 1] + (ci != cj));

			matrix[i][j] = min(a, min(b, c));
		}
	}

	return matrix[n][m];
}
