/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
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

#include "cache-utils.h"
#include <eixTk/stringutils.h>
#include <portage/package.h>

#include <fstream>
#include <limits>

int package_selector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strcmp(dent->d_name, "CVS") != 0);
}

int ebuild_selector (SCANDIR_ARG3 dent)
{
	return package_selector(dent);
}


using namespace std;

inline
void skip_lines(const int nr, ifstream &is, const string &filename) throw (ExBasic)
{
	for(int i=nr; i>0; --i)
	{
		is.ignore(numeric_limits<int>::max(), '\n');
		if(is.fail())
			throw ExBasic("Can't read cache file %s: %s",
			              filename.c_str(), strerror(errno));
	}
}

#define open_skipping(nr, is, filename) \
	ifstream is(filename); \
	if(!is.is_open()) \
		throw ExBasic("Can't open %s: %s", (filename), strerror(errno)); \
	skip_lines(nr, is, (filename));


/** Read the keywords and slot from a flat cache file. */
void flat_get_keywords_slot(const string &filename, string &keywords, string &slot) throw (ExBasic)
{
	open_skipping(2, is, filename.c_str());
	getline(is, slot);
	skip_lines(5, is, filename);
	getline(is, keywords);
	is.close();
}

/** Read a flat cache file. */
void
flat_read_file(const char *filename, Package *pkg) throw (ExBasic)
{
	open_skipping(5, is, filename);
	string linebuf;
	// Read the rest
	for(int linenr = 5; getline(is, linebuf); ++linenr)
	{
		switch(linenr)
		{
			case 5:  pkg->homepage = linebuf; break;
			case 6:  pkg->licenses = linebuf; break;
			case 7:  pkg->desc     = linebuf; break;
			case 13: pkg->provide  = linebuf; is.close();
					 return;
		}
	}
	is.close();
}
