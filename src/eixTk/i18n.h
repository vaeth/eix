// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_I18N_H_
#define SRC_EIXTK_I18N_H_ 1

// include "eixTk/i18n.h" This comment satisfies check_include script

#ifdef ENABLE_NLS

#include <libintl.h>
#define _(a) gettext(a)
#define N_(a) ngettext(a)

#else /* !defined(ENABLE_NLS) */
#define _(a) a
#define N_(a) a
#endif

#endif  // SRC_EIXTK_I18N_H_
