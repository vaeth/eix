// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <cerrno>
#include <cstring>

#include <fstream>
#include <limits>
#include <string>

#include "cache/base.h"
#include "cache/common/flat_reader.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "portage/depend.h"
#include "portage/package.h"

using std::string;

using std::ifstream;

static bool skip_lines(const eix::TinyUnsigned nr, ifstream *is, const string &filename, BasicCache::ErrorCallback error_callback) ATTRIBUTE_NONNULL_;

static bool
skip_lines(const eix::TinyUnsigned nr, ifstream *is, const string &filename, BasicCache::ErrorCallback error_callback)
{
	for(eix::TinyUnsigned i(nr); likely(i != 0); --i) {
		is->ignore(std::numeric_limits<int>::max(), '\n');
		if(is->fail()) {
			error_callback(eix::format(_("Can't read cache file %s: %s"))
				% filename % strerror(errno));
			return false;
		}
	}
	return true;
}

/** Read the keywords and slot from a flat cache file. */
void
flat_get_keywords_slot_iuse_restrict(const string &filename, string *keywords, string *slotname, string *iuse, string *restr, string *props, Depend *dep, BasicCache::ErrorCallback error_callback)
{
	ifstream is(filename.c_str());
	if(!is.is_open()) {
		error_callback(eix::format(_("Can't open %s: %s"))
			% filename % strerror(errno));
	}
	string depend, rdepend, pdepend;
	bool use_dep(Depend::use_depend);
	if(use_dep) {
		getline(is, depend);
		getline(is, rdepend);
	} else {
		skip_lines(2, &is, filename, error_callback);
	}
	getline(is, *slotname);
	skip_lines(1, &is, filename, error_callback);
	getline(is, *restr);
	skip_lines(3, &is, filename, error_callback);
	getline(is, *keywords);
	skip_lines(1, &is, filename, error_callback);
	getline(is, *iuse);
	if(use_dep) {
		skip_lines(1, &is, filename, error_callback);
		getline(is, pdepend);
		skip_lines(2, &is, filename, error_callback);
	} else {
		skip_lines(4, &is, filename, error_callback);
	}
	getline(is, *props);
	if(use_dep) {
		string hdepend;
		skip_lines(1, &is, filename, error_callback);
		getline(is, hdepend);
		dep->set(depend, rdepend, pdepend, hdepend, false);
	}
	is.close();
}

/** Read a flat cache file. */
void
flat_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback)
{
	ifstream is(filename);
	if(!is.is_open()) {
		error_callback(eix::format(_("Can't open %s: %s"))
			% filename % strerror(errno));
	}
	skip_lines(5, &is, filename, error_callback);
	string linebuf;
	// Read the rest
	for(eix::TinyUnsigned linenr(5); getline(is, linebuf); ++linenr) {
		switch(linenr) {
			case 5:  pkg->homepage = linebuf;
			         break;
			case 6:  pkg->licenses = linebuf;
			         break;
			case 7:  pkg->desc     = linebuf;
			         is.close();
			         return;
			default:
				break;
		}
	}
	is.close();
}
