// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_NULL_H_
#define SRC_EIXTK_NULL_H_ 1

// include "eixTk/null.h" This comment satisfies check_include script

#ifdef HAVE_NULLPTR
#define NULLPTR nullptr
#else
#include <cstddef>
#define NULLPTR NULL
#endif

#endif  // SRC_EIXTK_NULL_H_
