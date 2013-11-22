// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_CONSTEXPR_H_
#define SRC_EIXTK_CONSTEXPR_H_ 1

// include "eixTk/constexpr.h" make check_includes happy
#ifdef HAVE_CONSTEXPR
#define CONSTEXPR constexpr const
#else
#define CONSTEXPR const
#endif

#endif  // SRC_EIXTK_CONSTEXPR_H_
