// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_STRINGTYPES_H_
#define SRC_EIXTK_STRINGTYPES_H_

#include <config.h>

// check_includes: include "eixTk/stringtypes.h"

#include <set>
#include <string>
#include <vector>

#include "eixTk/iterate_map.h"
#include "eixTk/iterate_set.h"
#include "eixTk/unordered_map.h"
#include "eixTk/unordered_set.h"

typedef std::vector<std::string> WordVec;
typedef std::set<std::string> WordSet;
typedef ITERATE_MAP<std::string, std::string> WordIterateMap;
typedef ITERATE_SET<std::string> WordIterateSet;
typedef UNORDERED_MAP<std::string, std::string> WordUnorderedMap;
typedef UNORDERED_SET<std::string> WordUnorderedSet;
typedef std::string::size_type WordSize;
typedef WordVec LineVec;

#endif  // SRC_EIXTK_STRINGTYPES_H_
