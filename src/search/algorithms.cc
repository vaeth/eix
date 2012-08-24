// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <cassert>

#include <map>
#include <string>

#include "eixTk/null.h"
#include "search/algorithms.h"

using std::map;
using std::string;

map<string, unsigned int> *FuzzyAlgorithm::levenshtein_map = NULLPTR;

void
FuzzyAlgorithm::init_static()
{
	assert(levenshtein_map == NULLPTR);  // must be called only once
	levenshtein_map = new map<string, unsigned int>;
}

bool
FuzzyAlgorithm::operator()(const char *s, Package *p)
{
	assert(levenshtein_map != NULLPTR);  // has init_static() been called?
	unsigned int d(get_levenshtein_distance(search_string.c_str(), s));
	bool ok(d <= max_levenshteindistance);
	if(ok) {
		if(p != NULLPTR) {
			(*levenshtein_map)[p->category + "/" + p->name] = d;
		}
	}
	return ok;
}
