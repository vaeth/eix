// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "portage/eapi.h"
#include <config.h>

#include <string>

#include "eixTk/assert.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/unordered_map.h"

using std::string;

typedef UNORDERED_MAP<string, Eapi::EapiIndex> EapiMap;
static WordVec *eapi_vec(NULLPTR);
static EapiMap *eapi_map(NULLPTR);

void Eapi::init_static() {
	eix_assert_static(eapi_vec == NULLPTR);
	eapi_map = new EapiMap;
	eapi_vec = new WordVec;
	(*eapi_map)["0"] = 0;
	eapi_vec->push_back("0");
}

void Eapi::assign(const std::string& str) {
	eix_assert_static(eapi_map != NULLPTR);
	EapiMap::const_iterator it(eapi_map->find(str));
	if(likely(it != eapi_map->end())) {
		eapi_index = it->second;
		return;
	}
	(*eapi_map)[str] = eapi_index = eapi_vec->size();
	eapi_vec->push_back(str);
}

string Eapi::get() const {
	eix_assert_static(eapi_vec != NULLPTR);
	return (*eapi_vec)[eapi_index];
}
