// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "cache/common/flat_reader.h"
#include <config.h>  // IWYU pragma: keep

#include <cerrno>
#include <cstring>

#include <fstream>
#include <limits>
#include <string>

#include "cache/base.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "portage/depend.h"
#include "portage/package.h"
#include "portage/version.h"

using std::string;

using std::ifstream;

bool FlatReader::skip_lines(const eix::TinyUnsigned nr, ifstream *is, const string& filename) const {
	for(eix::TinyUnsigned i(nr); likely(i != 0); --i) {
		is->ignore(std::numeric_limits<int>::max(), '\n');
		if(is->fail()) {
			m_cache->m_error_callback(eix::format(_("cannot read cache file %s: %s"))
				% filename % std::strerror(errno));
			return false;
		}
	}
	return true;
}

/**
Read the keywords and slot from a flat cache file
**/
void FlatReader::get_keywords_slot_iuse_restrict(const string& filename, string *eapi, string *keywords, string *slotname, string *iuse, string *required_use, string *restr, string *props, Depend *dep) {
	ifstream is(filename.c_str());
	if(!is.is_open()) {
		m_cache->m_error_callback(eix::format(_("cannot open %s: %s"))
			% filename % std::strerror(errno));
	}
	string depend, rdepend, pdepend;
	bool use_dep(Depend::use_depend);
	if(use_dep) {
		getline(is, depend);
		getline(is, rdepend);
	} else {
		skip_lines(2, &is, filename);
	}
	getline(is, *slotname);
	skip_lines(1, &is, filename);
	getline(is, *restr);
	skip_lines(3, &is, filename);
	getline(is, *keywords);
	skip_lines(1, &is, filename);
	getline(is, *iuse);
	bool use_required_use(Version::use_required_use);
	if(use_required_use) {
		getline(is, *required_use);
	}
	if(use_dep) {
		if(!use_required_use) {
			skip_lines(1, &is, filename);
		}
		getline(is, pdepend);
		skip_lines(1, &is, filename);
	} else {
		skip_lines((use_required_use ? 2 : 3), &is, filename);
	}
	getline(is, *eapi);
	getline(is, *props);
	if(use_dep) {
		string hdepend;
		skip_lines(1, &is, filename);
		getline(is, hdepend);
		dep->set(depend, rdepend, pdepend, hdepend, false);
	}
	is.close();
}

/**
Read a flat cache file
**/
void FlatReader::read_file(const string& filename, Package *pkg) {
	ifstream is(filename.c_str());
	if(!is.is_open()) {
		m_cache->m_error_callback(eix::format(_("cannot open %s: %s"))
			% filename % std::strerror(errno));
	}
	skip_lines(5, &is, filename);
	string linebuf;
	// Read the rest
	for(eix::TinyUnsigned linenr(5); is.good(); ++linenr) {
		getline(is, linebuf);
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
	// We should never get here. However, we do not spit errors if we do...
	is.close();
}
