// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_UNUSED_H_
#define SRC_EIXTK_UNUSED_H_ 1

#include <config.h>

// check_includes: include "eixTk/unused.h" include "eixTk/attribute.h"

#if defined(HAVE_C11ATTRIBUTE_UNUSED) || defined(HAVE_ATTRIBUTE_UNUSED)
#define UNUSED(p)
#else
#define UNUSED(p) ((void)(p))
#endif

#endif  // SRC_EIXTK_UNUSED_H_
