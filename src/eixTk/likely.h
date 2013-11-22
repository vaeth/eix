// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_LIKELY_H_
#define SRC_EIXTK_LIKELY_H_ 1

// include "eixTk/likely.h" This comment satisfies check_include script

#ifdef HAVE___BUILTIN_EXPECT
#define likely(x)	__builtin_expect((x), 1)
#define unlikely(x)	__builtin_expect((x), 0)
#else
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif

#endif  // SRC_EIXTK_LIKELY_H_
