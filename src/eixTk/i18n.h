// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__I18B_H__
#define EIX__I18N_H__ 1

#include <config.h>

#ifdef ENABLE_NLS

#include <libintl.h>
#define _(a) gettext(a)

#else /* !defined(ENABLE_NLS) */
#define _(a) a
#endif

#endif /* EIX__I18N_H__ */
