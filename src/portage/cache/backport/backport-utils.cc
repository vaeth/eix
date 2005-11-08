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

#include "backport-utils.h"
#include <eixTk/stringutils.h>
#include <map>

int
get_map_from_cache(const char *file, map<string,string> &x)
{
	string lbuf;
	ifstream is(file);
	if(!is.is_open())
		return -1;

	while(getline(is, lbuf))
	{
		string::size_type p = lbuf.find_first_of("=");
		if(p == string::npos)
			continue;
		x[lbuf.substr(0, p)] = lbuf.substr(p + 1);
	}
	is.close();
	return x.size();
}

/** Read the stability on 'arch' from a metadata cache file. */
Keywords::Type backportCacheGetKeywords(string arch, string filename) throw (ExBasic)
{
	map<string,string> cf;
	
	if( get_map_from_cache(filename.c_str(), cf) < 0 )
	{
		throw ExBasic("Can't read cache file %s: %s",
				filename.c_str(),
				strerror(errno));
	}

	return Keywords::get_type(arch, cf["KEYWORDS"]);
}

/** Read a metadata cache file. */
void readBackportCachefile(Package *pkg, const char *filename) throw (ExBasic)
{
	map<string,string> cf;
	
	if( get_map_from_cache(filename, cf) < 0 )
	{
		throw ExBasic("Can't read cache file %s: %s",
				filename, strerror(errno));
	}

	pkg->homepage = cf["HOMEPAGE"];
	pkg->licenses = cf["LICENSE"];
	pkg->desc     = cf["DESCRIPTION"];
	pkg->provide  = cf["PROVIDE"];
}

