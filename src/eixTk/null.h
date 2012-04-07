// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__NULL_H__
#define EIX__NULL_H__ 1

// include <eixTk/null.h> This comment satisfies check_include script

#include <config.h>

#ifdef HAVE_NULLPTR
#define NULLPTR nullptr
#else
#include <cstddef>
#define NULLPTR NULL
#endif

#endif /* EIX__NULL_H__ */
