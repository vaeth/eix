// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cerrno>
#include <cstring>
#include <ctime>

#include <fstream>
#include <string>

#include "cache/base.h"
#include "cache/common/assign_reader.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/depend.h"
#include "portage/package.h"
#include "portage/version.h"

using std::string;

using std::ifstream;

bool AssignReader::get_map(const char *file) {
	if(currfile == NULLPTR) {
		currfile = new string(file);
		cf = new WordMap;
	} else {
		if(*currfile == file) {
			return currstate;
		}
		currfile->assign(file);
		cf->clear();
	}

	ifstream is(file);
	if(unlikely(!is.is_open())) {
		return (currstate = false);
	}

	while(likely(is.good())) {
		string lbuf;
		getline(is, lbuf);
		string::size_type p(lbuf.find('='));
		if(p == string::npos) {
			continue;
		}
		(*cf)[lbuf.substr(0, p)].assign(lbuf, p + 1, string::npos);
	}
	is.close();
	return (currstate = true);
}

const char *AssignReader::get_md5sum(const char *filename) {
	if(unlikely(!get_map(filename))) {
		return NULLPTR;
	}
	WordMap::const_iterator md5(cf->find("_md5_"));
	if(md5 == cf->end()) {
		return NULLPTR;
	}
	return md5->second.c_str();
}

bool AssignReader::get_mtime(time_t *t, const char *filename) {
	if(unlikely(!get_map(filename))) {
		return false;
	}
	WordMap::const_iterator mt(cf->find("_mtime_"));
	if(mt == cf->end()) {
		return false;
	}
	return likely(((*t) = my_atois(mt->second.c_str())) != 0);
}

/**
Read stability and other data from an "assign type" cache file
**/
void AssignReader::get_keywords_slot_iuse_restrict(const string& filename, string *eapi, string *keywords,
	string *slotname, string *iuse, string *required_use, string *restr,
	string *props, Depend *dep) {
	if(unlikely(!get_map(filename.c_str()))) {
		m_cache->m_error_callback(eix::format(_("cannot read cache file %s: %s"))
			% filename % strerror(errno));
		return;
	}
	(*eapi)     = (*cf)["EAPI"];
	(*keywords) = (*cf)["KEYWORDS"];
	(*slotname) = (*cf)["SLOT"];
	(*iuse)     = (*cf)["IUSE"];
	(*restr)    = (*cf)["RESTRICT"];
	(*props)    = (*cf)["PROPERTIES"];
	if(Version::use_required_use) {
		(*required_use) = (*cf)["REQUIRED_USE"];
	}
	if(Depend::use_depend) {
		dep->set((*cf)["DEPEND"], (*cf)["RDEPEND"], (*cf)["PDEPEND"], (*cf)["HDEPEND"], false);
	}
}

/**
Read an "assign type" cache file
**/
void AssignReader::read_file(const char *filename, Package *pkg) {
	if(unlikely(!get_map(filename))) {
		m_cache->m_error_callback(eix::format(_("cannot read cache file %s: %s"))
			% filename % strerror(errno));
		return;
	}
	pkg->homepage = (*cf)["HOMEPAGE"];
	pkg->licenses = (*cf)["LICENSE"];
	pkg->desc     = (*cf)["DESCRIPTION"];
}
