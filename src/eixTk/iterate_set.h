// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_ITERATE_SET_H_
#define SRC_EIXTK_ITERATE_SET_H_ 1

#include <config.h>

// check_includes: #include "eixTk/iterate_set.h"

// Types which need besides hashing also iteration.
// Benachmarks suggest that unordered_set iterates even faster than set
#include "eixTk/unordered_set.h"
#define ITERATE_SET UNORDERED_SET

#endif  // SRC_EIXTK_ITERATE_SET_H_
