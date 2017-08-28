// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_UNORDERED_SET_H_
#define SRC_EIXTK_UNORDERED_SET_H_ 1

#include <config.h>

#ifdef HAVE_UNORDERED_SET
#include <unordered_set>
#define UNORDERED_SET std::unordered_set
#else
#include <set>
#define UNORDERED_SET std::set
#endif

#endif  // SRC_EIXTK_UNORDERED_SET_H_
