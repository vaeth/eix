// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_SEARCH_LEVENSHTEIN_H_
#define SRC_SEARCH_LEVENSHTEIN_H_ 1

#include <config.h>

#include "eixTk/attribute.h"

typedef unsigned int Levenshtein;

/**
Calculates the Levenshtein distance of two strings.
Reference: http://www.merriampark.com/ld.htm
@param str_a string a
@param str_b string b
@return unsigned int Levenshtein distance of str_a <> str_b
**/
ATTRIBUTE_NONNULL_ ATTRIBUTE_PURE Levenshtein get_levenshtein_distance(const char *str_a, const char *str_b);

#endif  // SRC_SEARCH_LEVENSHTEIN_H_
