// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    

#ifndef __LEVENSHTEIN_H__
#define __LEVENSHTEIN_H__

/** Calculates the Levenshtein distance of two strings.
 * Reference: http://www.merriampark.com/ld.htm
 * @param str_a string a
 * @param str_b string b
 * @return int Levenshtein distance of str_a <> str_b */
int get_levenshtein_distance( const char *str_a, const char *str_b );

#endif /* __LEVENSHTEIN_H__ */
