// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__LEVENSHTEIN_H__)
#define EIX__LEVENSHTEIN_H__

/** Calculates the Levenshtein distance of two strings.
 * Reference: http://www.merriampark.com/ld.htm
 * @param str_a string a
 * @param str_b string b
 * @return int Levenshtein distance of str_a <> str_b */
unsigned int get_levenshtein_distance( const char *str_a, const char *str_b );

#endif /* EIX__LEVENSHTEIN_H__ */
