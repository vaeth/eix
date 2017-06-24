// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_ATTRIBUTE_H_
#define SRC_EIXTK_ATTRIBUTE_H_ 1

#include <config.h>

// check_includes: include "eixTk/attribute.h"

#ifdef HAVE_C11ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN [[noreturn]]  // NOLINT(whitespace/braces)
#else
#ifdef HAVE_ATTRIBUTE_NORETURN
#define ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
#define ATTRIBUTE_NORETURN
#endif  // HAVE_ATTRIBUTE_NORETURN
#endif  // HAVE_C11ATTRIBUTE_NORETURN

#ifdef HAVE_ATTRIBUTE_SIGNAL
#define ATTRIBUTE_SIGNAL __attribute__ ((signal))
#else
#define ATTRIBUTE_SIGNAL
#endif

#ifdef HAVE_ATTRIBUTE_CONST
#define ATTRIBUTE_CONST __attribute__ ((const))
#else
#define ATTRIBUTE_CONST
#endif

#ifdef HAVE_ATTRIBUTE_PURE
#define ATTRIBUTE_PURE __attribute__ ((pure))
#else
#define ATTRIBUTE_PURE
#endif

#if defined(USE_CONST_FOR_CONST_VIRTUALS) && defined(HAVE_ATTRIBUTE_CONST)
#define ATTRIBUTE_CONST_VIRTUAL ATTRIBUTE_CONST
#else
#if defined(USE_PURE_FOR_CONST_VIRTUALS) && defined(HAVE_ATTRIBUTE_PURE)
#define ATTRIBUTE_CONST_VIRTUAL ATTRIBUTE_PURE
#else
#define ATTRIBUTE_CONST_VIRTUAL
#endif  // USE PURE_FOR_CONST_VIRTUALS
#endif  // USE CONST_FOR_CONST_VIRTUALS

#ifdef HAVE_ATTRIBUTE_NONNULL_
#define ATTRIBUTE_NONNULL_ __attribute__ ((nonnull))
#else
#define ATTRIBUTE_NONNULL_
#endif

#ifdef HAVE_ATTRIBUTE_NONNULL
#define ATTRIBUTE_NONNULL(a) __attribute__ ((nonnull a))
#else
#define ATTRIBUTE_NONNULL(a)
#endif

#ifdef HAVE_ATTRIBUTE_FALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH [[fallthrough]];  // NOLINT(whitespace/braces)
#else
#ifdef HAVE_ATTRIBUTE_GNUFALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH [[gnu::fallthrough]];  // NOLINT(whitespace/braces)
#else
#ifdef HAVE_ATTRIBUTE_AFALLTHROUGH
#define ATTRIBUTE_FALLTHROUGH __attribute__ ((fallthrough));
#else
#define ATTRIBUTE_FALLTHROUGH
#endif  /* HAVE_ATTRIBUTE_AFALLTHROUGH */
#endif  /* HAVE_ATTRIBUTE_GNUFALLTHROUGH */
#endif  /* HAVE_ATTRIBUTE_FALLTHROUGH */

#endif  // SRC_EIXTK_ATTRIBUTE_H_

