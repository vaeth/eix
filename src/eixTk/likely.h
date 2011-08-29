// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__LIKELY_H__
#define EIX__LIKELY_H__ 1

// include <eixTk/likely.h> This comment satisfies check_include script

#include <config.h>

#ifdef HAVE___BUILTIN_EXPECT
#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)
#else
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif

#endif /* EIX__LIKELY_H__ */
