/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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

#include <eixTk/filenames.h>

/** Compare whether two filenames are identical */
bool same_filenames(const std::string &a, const std::string &b)
{
	const char *s = a.c_str();
	const char *t = b.c_str();
	bool first = true;
	while(1)
	{
		const char c = *(s++);
		if(c == '/')
		{
			while(*s == '/')
				s++;
		}
		const char d = *(t++);
		if(d == '/')
		{
			while(*t == '/')
				t++;
		}
		if( c != d )
		{
			if(first)
				return false;
			if(( c == '\0' ) && (d == '/') && (*t == '\0'))
				return true;
			if(( d == '\0' ) && (c == '/') && (*s == '\0'))
				return true;
			return false;
		}
		first = false;
		if( c == '\0' )
			return true;
	}
}
