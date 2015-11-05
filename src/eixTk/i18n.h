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

#include "eixTk/diagnostics.h"

GCC_DIAG_OFF(vla-extension)
GCC_DIAG_OFF(vla)
#include <gettext.h>  // NOLINT(build/include_order)
GCC_DIAG_ON(vla)
GCC_DIAG_ON(vla-extension)

#define _ gettext
#define N_ ngettext
#define P_ pgettext
#define NP_ npgettext

// const char *pgettext(const char *p, const char *a);
// const char *nppgettext(const char *p, const char *a, const char *b, unsigned long int n);

#else /* !defined(ENABLE_NLS) */
#define _(a) (a)
#define N_(a, b, n) (((n) == 1) ? (a) : (b))
#define P_(p, a) _(a)
#define NP_(p, a, b, n) N_((a), (b), (n))
#endif

#endif  // SRC_EIXTK_I18N_H_
