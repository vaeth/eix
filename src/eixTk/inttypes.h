// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __GUARD__INTTYPES_H__
#define __GUARD__INTTYPES_H__

#include <config.h>

#if defined(HAVE_TR1_CSTDINT)
#include <tr1/cstdint>
#else
#if defined(USE_STDINT_H)
#include <stdint.h>
#endif
#endif

#endif /* __GUARD__INTTYPES_H__ */
