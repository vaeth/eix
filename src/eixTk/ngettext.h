// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_NGETTEXT_H_
#define SRC_EIXTK_NGETTEXT_H_ 1

// If this file is included, it *must* be the very last!

#include "eixTk/i18n.h"
// include "eixTk/ngettext.h" _() make check_includes happy

#ifndef ENABLE_NLS
#define ngettext(a, b, c) (((c) == 1) ? (a) : (b))
#endif

#endif  // SRC_EIXTK_NGETTEXT_H_
