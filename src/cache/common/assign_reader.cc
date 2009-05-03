// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "assign_reader.h"

#include <portage/package.h>
#include <config.h>

#include <fstream>

using namespace std;

static int
get_map_from_cache(const char *file, map<string,string> &x)
{
	string lbuf;
	ifstream is(file);
	if(!is.is_open())
		return -1;

	while(getline(is, lbuf))
	{
		string::size_type p = lbuf.find_first_of('=');
		if(p == string::npos)
			continue;
		x[lbuf.substr(0, p)] = lbuf.substr(p + 1);
	}
	is.close();
	return x.size();
}

/** Read stability and other data from an "assign type" cache file. */
void
assign_get_keywords_slot_iuse_restrict(const string &filename, string &keywords, string &slotname, string &iuse, string &restr, string &props, BasicCache::ErrorCallback error_callback)
{
	map<string,string> cf;

	if(get_map_from_cache(filename.c_str(), cf) < 0) {
		error_callback(eix::format(_("Can't read cache file %s: "))
			% filename % strerror(errno));
		return;
	}
	keywords = cf["KEYWORDS"];
	slotname = cf["SLOT"];
	iuse     = cf["IUSE"];
	restr    = cf["RESTRICT"];
	props    = cf["PROPERTIES"];
}

/** Read an "assign type" cache file. */
void
assign_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback)
{
	map<string,string> cf;

	if(get_map_from_cache(filename, cf) < 0) {
		error_callback(eix::format(_("Can't read cache file %s: "))
			% filename % strerror(errno));
		return;
	}
	pkg->homepage = cf["HOMEPAGE"];
	pkg->licenses = cf["LICENSE"];
	pkg->desc     = cf["DESCRIPTION"];
	pkg->provide  = cf["PROVIDE"];
}
