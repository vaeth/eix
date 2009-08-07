// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef EIX__INTTYPES_H__
#define EIX__INTTYPES_H__ 1

#include <config.h>

#ifdef HAVE_TR1_CSTDINT
#include <tr1/cstdint>
#else
#ifdef HAVE_CSTDINT
#include <cstdint>
#else
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#endif
#endif

#endif /* EIX__INTTYPES_H__ */
