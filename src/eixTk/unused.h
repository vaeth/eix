// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__UNUSED_H__
#define EIX__UNUSED_H__ 1

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

#ifdef HAVE_ATTRIBUTE_NORETURN
#ifndef ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN __attribute__ ((__noreturn__))
#endif
#else
#define ATTRIBUTE_NORETURN
#endif /* HAVE_ATTRIBUTE_NORETURN */

#endif /* EIX__UNUSED_H__ */
