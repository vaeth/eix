// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "flat-reader.h"
#include <eixTk/stringutils.h>
#include <portage/package.h>

#include <fstream>
#include <limits>

using namespace std;

inline
bool skip_lines(const int nr, ifstream &is, const string &filename, BasicCache::ErrorCallback error_callback)
{
	for(int i=nr; i>0; --i)
	{
		is.ignore(numeric_limits<int>::max(), '\n');
		if(is.fail())
		{
			error_callback("Can't read cache file %s: %s",
					filename.c_str(), strerror(errno));
			return false;
		}
	}

	return true;
}

#define open_skipping(nr, is, filename, error_callback) \
	ifstream is(filename); \
	if(!is.is_open()) \
		error_callback("Can't open %s: %s", (filename), strerror(errno)); \
	skip_lines(nr, is, (filename), error_callback);


/** Read the keywords and slot from a flat cache file. */
void flat_get_keywords_slot_iuse_restrict(const string &filename, string &keywords, string &slotname, string &iuse, string &restr, BasicCache::ErrorCallback error_callback)
{
	open_skipping(2, is, filename.c_str(), error_callback);
	getline(is, slotname);
	skip_lines(1, is, filename, error_callback);
	getline(is, restr);
	skip_lines(3, is, filename, error_callback);
	getline(is, keywords);
	skip_lines(1, is, filename, error_callback);
	getline(is, iuse);
	is.close();
}

/** Read a flat cache file. */
void
flat_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback)
{
	open_skipping(5, is, filename, error_callback);
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
			default:
				break;
		}
	}
	is.close();
}
