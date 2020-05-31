// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_DIAGNOSTICS_H_
#define SRC_EIXTK_DIAGNOSTICS_H_ 1

#include <config.h>  // IWYU pragma: keep

// check_includes: include "eixTk/diagnostics.h"

#ifdef FULL_GCC_DIAG_PRAGMA
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)

#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)

#else

#define GCC_DIAG_OFF(x)
#define GCC_DIAG_ON(x)

#endif

#ifdef WSUGGEST_FINAL_METHODS
#define WSUGGEST_FINAL_METHODS_OFF GCC_DIAG_OFF(suggest-final-methods)
#define WSUGGEST_FINAL_METHODS_ON GCC_DIAG_ON(suggest-final-methods)
#else
#define WSUGGEST_FINAL_METHODS_OFF
#define WSUGGEST_FINAL_METHODS_ON
#endif

#ifdef WZERO_AS_NULL_POINTER_CONSTANT
#define WZERO_AS_NULL_POINTER_CONSTANT_OFF GCC_DIAG_OFF(zero-as-null-pointer-constant)
#define WZERO_AS_NULL_POINTER_CONSTANT_ON GCC_DIAG_ON(zero-as-null-pointer-constant)
#else
#define WZERO_AS_NULL_POINTER_CONSTANT_OFF
#define WZERO_AS_NULL_POINTER_CONSTANT_ON
#endif

#ifdef WSUGGEST_ATTRIBUTE_CONST
#define WSUGGEST_ATTRIBUTE_CONST_OFF GCC_DIAG_OFF(suggest-attribute=const)
#define WSUGGEST_ATTRIBUTE_CONST_ON GCC_DIAG_ON(suggest-attribute=const)
#else
#define WSUGGEST_ATTRIBUTE_CONST_OFF
#define WSUGGEST_ATTRIBUTE_CONST_ON
#endif

#ifdef WSUGGEST_ATTRIBUTE_PURE
#define WSUGGEST_ATTRIBUTE_PURE_OFF GCC_DIAG_OFF(suggest-attribute=pure)
#define WSUGGEST_ATTRIBUTE_PURE_ON GCC_DIAG_ON(suggest-attribute=pure)
#else
#define WSUGGEST_ATTRIBUTE_PURE_OFF
#define WSUGGEST_ATTRIBUTE_PURE_ON
#endif

#ifdef WGNU_STATEMENT_EXPRESSION
#define WGNU_STATEMENT_EXPRESSION_OFF GCC_DIAG_OFF(gnu-statement-expression)
#define WGNU_STATEMENT_EXPRESSION_ON GCC_DIAG_ON(gnu-statement-expression)
#else
#define WGNU_STATEMENT_EXPRESSION_OFF
#define WGNU_STATEMENT_EXPRESSION_ON
#endif

#endif  // SRC_EIXTK_DIAGNOSTICS_H_
