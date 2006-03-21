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

#include "flat-utils.h"
#include <eixTk/stringutils.h>

#include <fstream>
#include <limits>

using namespace std;

/** Read the stability on 'arch' from a metadata cache file. */
Keywords::Type get_keywords(string arch, string filename) throw (ExBasic)
{
	// NOTE: Switched from std::istream::getline to std::getline
	string linebuf;
	int linenr;
	ifstream is(filename.c_str());

	if(!is.is_open())
		throw ExBasic("Can't open %s: %s", filename.c_str(), strerror(errno));

	// Skip the first 8 lines
	for( linenr=0;linenr<8; linenr++ )
	{
		is.ignore(numeric_limits<int>::max(), '\n');
		if( is.fail() ) throw ExBasic("Can't read cache file %s: %s", filename.c_str(), strerror(errno));
	}
	// Read the keywords line
	getline(is, linebuf);
	is.close();

	return Keywords::get_type(arch, linebuf);
}

/** Read a metadata cache file. */
void read_file(Package *pkg, const char *filename) throw (ExBasic)
{
	string linebuf;
	int linenr;
	ifstream is(filename);

	if(!is.is_open()) throw ExBasic("Can't open %s: %s", filename, strerror(errno));

	// Skip the first 5 lines
	for( linenr=0;linenr<5; linenr++ )
	{
		is.ignore(numeric_limits<int>::max(), '\n');
		if( is.fail() ) throw ExBasic("Can't read metadata cache file: %s", filename);
	}

	// Read the rest
	for(; getline(is, linebuf); linenr++)
	{
		switch( linenr )
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

