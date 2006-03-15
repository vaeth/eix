/***************************************************************************
 *   Copyright (C) 2005 by Wolfgang Frisch                                 *
 *   xororand@users.sf.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __LEVENSHTEIN_H__
#define __LEVENSHTEIN_H__

#include <string>

using namespace std;

/** Calculates the Levenshtein distance of two strings.
 * Reference: http://www.merriampark.com/ld.htm
 * @param str_a string a
 * @param str_b string b
 * @return int Levenshtein distance of str_a <> str_b */
int get_levenshtein_distance( const char *str_a, const char *str_b );

#endif /* __LEVENSHTEIN_H__ */
