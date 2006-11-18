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
#include <fnmatch.h>

using namespace std;

string normalize_path(const char *path)
{
	string name(path);
	for(string::size_type i = 0; i < name.size(); ++i)
	{
		// Erase all / following one /
		if(name[i] == '/') {
			string::size_type n = 0;
			for(string::size_type j = i + 1 ;
				j < name.size(); ++j) {
				if(name[j] != '/')
					break;
				++n;
			}
			if(n)
				name.erase(i + 1, n);
		}
	}
	// Erase trailing / if it is not the first character
	string::size_type s = name.size();
	if(s > 1) {
		if(name[--s] == '/')
			name.erase(s, 1);
	}
	return name;
}

/** Compare whether two filenames are identical */
bool same_filenames(const char *mask, const char *name, bool glob)
{
	string m = normalize_path(mask);
	string n = normalize_path(name);
	if(!glob)
		return (m == n);
	return (fnmatch(m.c_str(), n.c_str(), 0) == 0);
}
