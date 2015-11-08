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

// We do not use gettext.h:
// The latter uses currently incompatible code which produces warnings.

#include <libintl.h>

#define _ gettext
#define N_ ngettext
#define P_ eix_pgettext
#define NP_ eix_npgettext

// We only use the static versions of pgettext and npgettext.
// For these, variants of the inline versions from gettext.h
// produce perhaps the shortest code

#define EIX_GETTEXT_CONTEXT_GLUE "\004"

#define eix_pgettext(p, a) eix::pgettext_aux(p EIX_GETTEXT_CONTEXT_GLUE a, a)
#define eix_npgettext(p, a, b, n) eix::npgettext_aux(p EIX_GETTEXT_CONTEXT_GLUE a, a, b, n)

namespace eix {

inline static const char *pgettext_aux(const char *msg_ctxt_id, const char *msgid) {
	const char *translation(gettext(msg_ctxt_id));
	return ((translation == msg_ctxt_id) ? msgid : translation);
}

inline static const char *npgettext_aux(const char *msg_ctxt_id, const char *msgid, const char *msgid_plural, unsigned long int n) {  // NOLINT(runtime/int)
	const char *translation(ngettext(msg_ctxt_id, msgid_plural, n));
	return (((translation == msg_ctxt_id) || (translation == msgid_plural)) ?
		(n == 1 ? msgid : msgid_plural) : translation);
}

}/* namespace eix */

#else /* !defined(ENABLE_NLS) */
#define _(a) (a)
#define N_(a, b, n) (((n) == 1) ? (a) : (b))
#define P_(p, a) _(a)
#define NP_(p, a, b, n) N_((a), (b), (n))
#endif

#endif  // SRC_EIXTK_I18N_H_
