// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <fnmatch.h>

#include <cstring>

#include <map>
#include <string>

#include "eixTk/assert.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/unused.h"
#include "portage/package.h"
#include "search/algorithms.h"
#include "search/levenshtein.h"

/*
Check if we have FNM_CASEFOLD ..
fnmatch(3) tells that this is a GNU extension.
*/
#ifdef FNM_CASEFOLD
#define FNMATCH_FLAGS FNM_CASEFOLD
#else
#define FNMATCH_FLAGS 0
#endif

using std::map;
using std::string;

typedef map<string, Levenshtein> LevenshteinMap;
LevenshteinMap *FuzzyAlgorithm::levenshtein_map = NULLPTR;

bool BaseAlgorithm::operator()(const char *s, Package *p, bool simplify) {
	if(can_simplify() && unlikely(!have_simplified) && likely(simplify)) {
		have_simplified = true;
		// cut out the first nonempty valid search string
		for(string::size_type i = 0; i < search_string.length(); ++i) {
			if(likely(is_valid_pkgpath(search_string[i]))) {
				if(unlikely(i > 0)) {
					search_string.erase(0, i);
				}
				break;
			}
		}
		for(string::size_type i = 0; i < search_string.length(); ++i) {
			if(unlikely(!is_valid_pkgpath(search_string[i]))) {
				if(likely(i > 0)) {
					search_string.erase(i);
				}
				break;
			}
		}
	}
	return (*this)(s, p);
}

void FuzzyAlgorithm::init_static() {
	eix_assert_static(levenshtein_map == NULLPTR);
	levenshtein_map = new LevenshteinMap;
}

bool FuzzyAlgorithm::compare(Package *p1, Package *p2) {
	return ((*levenshtein_map)[p1->category + "/" + p1->name]
			< (*levenshtein_map)[p2->category + "/" + p2->name]);
}


bool FuzzyAlgorithm::operator()(const char *s, Package *p) const {
	eix_assert_static(levenshtein_map != NULLPTR);
	Levenshtein d(get_levenshtein_distance(search_string.c_str(), s));
	bool ok(d <= max_levenshteindistance);
	if(ok) {
		if(p != NULLPTR) {
			(*levenshtein_map)[p->category + "/" + p->name] = d;
		}
	}
	return ok;
}

bool ExactAlgorithm::operator()(const char *s, Package *p ATTRIBUTE_UNUSED) const {
	UNUSED(p);
	return (strcmp(search_string.c_str(), s) == 0);
}

bool BeginAlgorithm::operator()(const char *s, Package *p ATTRIBUTE_UNUSED) const {
	UNUSED(p);
	return (strncmp(search_string.c_str(), s, search_string.size()) == 0);
}

bool EndAlgorithm::operator()(const char *s, Package *p ATTRIBUTE_UNUSED) const {
	UNUSED(p);
	string::size_type l(strlen(s));
	string::size_type sl(search_string.size());
	if(l < sl) {
		return false;
	}
	return (strcmp(search_string.c_str(), s + (l - sl)) == 0);
}

bool PatternAlgorithm::operator()(const char *s, Package *p ATTRIBUTE_UNUSED) const {
	UNUSED(p);
	return (fnmatch(search_string.c_str(), s, FNMATCH_FLAGS) == 0);
}
