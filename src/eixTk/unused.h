// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_UNUSED_H_
#define SRC_EIXTK_UNUSED_H_ 1

// include "eixTk/unused.h" This comment satisfies check_include script

#include <config.h>

#ifdef ATTRIBUTE_UNUSED
#define UNUSED(p)

#else /* ifndef ATTRIBUTE_UNUSED */

#ifdef HAVE_ATTRIBUTE_UNUSED

#define ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#define UNUSED(p)

#else /* ifndef HAVE_ATTRIBUTE_UNUSED */

#define ATTRIBUTE_UNUSED
#define UNUSED(p) ((void)(p))

#endif /* HAVE_ATTRIBUTE_UNUSED */
#endif /* ATTRIBUTE_UNUSED */

#endif  // SRC_EIXTK_UNUSED_H_
