// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_STRINGTYPES_H_
#define SRC_EIXTK_STRINGTYPES_H_

// include "eixTk/stringtypes.h" make check_includes happy

#include <map>
#include <set>
#include <string>
#include <vector>

typedef std::vector<std::string> WordVec;
typedef std::set<std::string> WordSet;
typedef std::map<std::string, std::string> WordMap;
typedef std::string::size_type WordSize;
typedef WordVec LineVec;

#endif  // SRC_EIXTK_STRINGTYPES_H_
