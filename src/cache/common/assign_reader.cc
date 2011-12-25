// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "assign_reader.h"

#include <cache/base.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <portage/package.h>

#include <fstream>
#include <map>
#include <string>

#include <cstddef>
#include <cstring>

using namespace std;

static map<string,string> *
get_map_from_cache(const char *file)
{
	static string oldfile;
	static map<string,string> cf;
	if(oldfile == file) {
		return &cf;
	}
	oldfile = file;
	cf.clear();

	ifstream is(file);
	if(!is.is_open())
		return NULL;

	string lbuf;
	while(likely(getline(is, lbuf) != 0)) {
		string::size_type p(lbuf.find('='));
		if(p == string::npos)
			continue;
		cf[lbuf.substr(0, p)].assign(lbuf, p + 1, string::npos);
	}
	is.close();
	return &cf;
}

const char *
assign_get_md5sum(const string &filename)
{
	map<string,string> *cf(get_map_from_cache(filename.c_str()));
	if(unlikely(cf == NULL)) {
		return NULL;
	}
	map<string,string>::const_iterator md5(cf->find("_md5_"));
	if(md5 == cf->end()) {
		return NULL;
	}
	return md5->second.c_str();
}

/** Read stability and other data from an "assign type" cache file. */
void
assign_get_keywords_slot_iuse_restrict(const string &filename, string &keywords, string &slotname, string &iuse, string &restr, string &props, BasicCache::ErrorCallback error_callback)
{
	map<string,string> *cf(get_map_from_cache(filename.c_str()));
	if(unlikely(cf == NULL)) {
		error_callback(eix::format(_("Can't read cache file %s: %s"))
			% filename % strerror(errno));
		return;
	}
	keywords = (*cf)["KEYWORDS"];
	slotname = (*cf)["SLOT"];
	iuse     = (*cf)["IUSE"];
	restr    = (*cf)["RESTRICT"];
	props    = (*cf)["PROPERTIES"];
}

/** Read an "assign type" cache file. */
void
assign_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback)
{
	map<string,string> *cf(get_map_from_cache(filename));
	if(unlikely(cf == NULL)) {
		error_callback(eix::format(_("Can't read cache file %s: %s"))
			% filename % strerror(errno));
		return;
	}
	pkg->homepage = (*cf)["HOMEPAGE"];
	pkg->licenses = (*cf)["LICENSE"];
	pkg->desc     = (*cf)["DESCRIPTION"];
	pkg->set_provide((*cf)["PROVIDE"]);
}
