// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_DIALECT_H_
#define SRC_EIXTK_DIALECT_H_ 1

#include <config.h>

// define C++-11 dialect specific keywords (if available)
// or poor man's substitutes

// check_includes: include "eixTk/dialect.h" std::pair<int, int>

#ifdef HAVE_CONSTEXPR
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

#ifdef HAVE_OVERRIDE
#define OVERRIDE override
#else
#define OVERRIDE
#endif

#ifdef HAVE_DELETE
#define ASSIGN_DELETE = delete
#else
#define ASSIGN_DELETE
#endif

#ifdef HAVE_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#ifdef HAVE_EMPLACE_BACK
#define PUSH_BACK emplace_back
#define EMPLACE_BACK(a, b) emplace_back b
#else
#define PUSH_BACK push_back
#define EMPLACE_BACK(a, b) push_back(a b)
#endif

#ifdef HAVE_EMPLACE
#define INSERT emplace
#define EMPLACE(a, b) emplace b
#else
#define INSERT insert
#define EMPLACE(a, b) insert(a b)
#endif

#ifdef HAVE_MOVE
#include <utility>
#define MOVE(a) std::move(a)
#else
#define MOVE(a) (a)
#endif

#endif  // SRC_EIXTK_DIALECT_H_
