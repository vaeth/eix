// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "selectors.h"

#include <cstring>
#include <dirent.h>

using namespace std;

int package_selector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strcmp(dent->d_name, "CVS") != 0);
}

int ebuild_selector (SCANDIR_ARG3 dent)
{
	return package_selector(dent);
}

string::size_type ebuild_pos(const std::string &str)
{
	string::size_type pos = str.length();
	static const string::size_type append_size = 7;
	if(pos <= append_size)
		return string::npos;
	pos -= append_size;
	if(!(str.compare(pos, append_size, ".ebuild")))
		return pos;
	string::size_type epos = str.find(".ebuild-");
	if(epos == string::npos)
		return string::npos;
	if(epos + 1 == pos) // Empty EAPI is not admissible
		return string::npos;
	return epos;
}
