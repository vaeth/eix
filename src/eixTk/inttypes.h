// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef SRC_EIXTK_INTTYPES_H_
#define SRC_EIXTK_INTTYPES_H_ 1

#ifdef HAVE_TR1_CSTDINT
#include <tr1/cstdint>  // NOLINT(build/c++tr1)
#else
#ifndef HAVE_CSTDINT
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#else
#include <cstdint>
#endif
#endif

#endif  // SRC_EIXTK_INTTYPES_H_
