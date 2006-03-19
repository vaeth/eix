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

#include "levenshtein.h"

#include <vector>

using namespace std;

/**
 * Calculates the Levenshtein distance of two strings
 * @param str_a string a
 * @param str_b string b
 * @return int Levenshtein distance of str_a <> str_b
 */
int get_levenshtein_distance( const char *str_a, const char *str_b )
{
	int n,m;
	int cost;
	vector< vector<int> > matrix;

	n = strlen(str_a);
	m = strlen(str_b);
	if( n==0 ) return m;
	if( m==0 ) return n;

	// initialize the matrix' dimensions
	matrix.resize( n+1 );
	for( int i=0; i<=n; i++ )
	{
		matrix[i].resize( m+1 );
		fill( matrix[i].begin(), matrix[i].end(), 0);
	}

	// fill the 1st column/row with str_a / str_b
	for( int i=0; i<=n; i++ )
		matrix[i][0] = i;
	for( int j=0; j<=m; j++ )
		matrix[0][j] = j;

	// calculate the matrix
	for( int i=1; i<=n; i++ )
	{
		for( int j=1; j<=m; j++ )
		{
			char ci = str_a[i-1];
			char cj = str_b[j-1];
			cost = (ci==cj) ? 0:1;

			int a = matrix[i-1][j] + 1;
			int b = matrix[i][j-1] + 1;
			int c = matrix[i-1][j-1] + cost;

			matrix[i][j] = min( a, min(b,c) );
		}
	}

	return matrix[n][m];
}

