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

#include <fstream>
#include <string>

#include "cache/base.h"
#include "cache/common/assign_reader.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "portage/depend.h"
#include "portage/package.h"

using std::string;

using std::ifstream;

static WordMap *get_map_from_cache(const char *file) ATTRIBUTE_NONNULL_;

static WordMap *get_map_from_cache(const char *file) {
	static string *oldfile = NULLPTR;
	static WordMap *cf;
	if(unlikely(oldfile == NULLPTR)) {
		oldfile = new string(file);
		cf = new WordMap;
	} else {
		if(*oldfile == file) {
			return cf;
		}
		oldfile->assign(file);
		cf->clear();
	}

	ifstream is(file);
	if(unlikely(!is.is_open())) {
		return NULLPTR;
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
	return cf;
}

const char *assign_get_md5sum(const string& filename) {
	WordMap *cf(get_map_from_cache(filename.c_str()));
	if(unlikely(cf == NULLPTR)) {
		return NULLPTR;
	}
	WordMap::const_iterator md5(cf->find("_md5_"));
	if(md5 == cf->end()) {
		return NULLPTR;
	}
	return md5->second.c_str();
}

/** Read stability and other data from an "assign type" cache file. */
void assign_get_keywords_slot_iuse_restrict(const string& filename, string *keywords, string *slotname, string *iuse, string *restr, string *props,
	Depend *dep, BasicCache::ErrorCallback error_callback) {
	WordMap *cf(get_map_from_cache(filename.c_str()));
	if(unlikely(cf == NULLPTR)) {
		error_callback(eix::format(_("cannot read cache file %s: %s"))
			% filename % strerror(errno));
		return;
	}
	(*keywords) = (*cf)["KEYWORDS"];
	(*slotname) = (*cf)["SLOT"];
	(*iuse)     = (*cf)["IUSE"];
	(*restr)    = (*cf)["RESTRICT"];
	(*props)    = (*cf)["PROPERTIES"];
	if(Depend::use_depend) {
		dep->set((*cf)["DEPEND"], (*cf)["RDEPEND"], (*cf)["PDEPEND"], (*cf)["HDEPEND"], false);
	}
}

/** Read an "assign type" cache file. */
void assign_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback) {
	WordMap *cf(get_map_from_cache(filename));
	if(unlikely(cf == NULLPTR)) {
		error_callback(eix::format(_("cannot read cache file %s: %s"))
			% filename % strerror(errno));
		return;
	}
	pkg->homepage = (*cf)["HOMEPAGE"];
	pkg->licenses = (*cf)["LICENSE"];
	pkg->desc     = (*cf)["DESCRIPTION"];
}
