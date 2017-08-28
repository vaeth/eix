// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/extendedversion.h"
#include <config.h>

#include <cstring>

#include <string>

#include "eixTk/assert.h"
#include "eixTk/attribute.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "eixTk/unordered_map.h"
#include "eixTk/utils.h"
#include "portage/conf/portagesettings.h"
#include "portage/package.h"

using std::equal;
using std::string;

class RestrictMap : public UNORDERED_MAP<string, ExtendedVersion::Restrict> {
	private:
		ATTRIBUTE_NONNULL_ void mapinit(const char *s, ExtendedVersion::Restrict r) {
			(*this)[s] = r;
		}

	public:
		RestrictMap() {
			mapinit("fetch",          ExtendedVersion::RESTRICT_FETCH);
			mapinit("mirror",         ExtendedVersion::RESTRICT_MIRROR);
			mapinit("primaryuri",     ExtendedVersion::RESTRICT_PRIMARYURI);
			mapinit("binchecks",      ExtendedVersion::RESTRICT_BINCHECKS);
			mapinit("bindist",        ExtendedVersion::RESTRICT_BINDIST);
			mapinit("installsources", ExtendedVersion::RESTRICT_INSTALLSOURCES);
			mapinit("strip",          ExtendedVersion::RESTRICT_STRIP);
			mapinit("test",           ExtendedVersion::RESTRICT_TEST);
			mapinit("userpriv",       ExtendedVersion::RESTRICT_USERPRIV);
			mapinit("parallel",       ExtendedVersion::RESTRICT_PARALLEL);
		}

		ATTRIBUTE_PURE ExtendedVersion::Restrict getRestrict(const string& s) const {
			RestrictMap::const_iterator i(find(s));
			if(i != end()) {
				return i->second;
			}
			return ExtendedVersion::RESTRICT_NONE;
		}
};

static RestrictMap *restrict_map = NULLPTR;

class PropertiesMap : public UNORDERED_MAP<string, ExtendedVersion::Properties> {
	private:
		ATTRIBUTE_NONNULL_ void mapinit(const char *s, ExtendedVersion::Properties p) {
			(*this)[s] = p;
		}
	public:
		PropertiesMap() {
			mapinit("interactive", ExtendedVersion::PROPERTIES_INTERACTIVE);
			mapinit("live",        ExtendedVersion::PROPERTIES_LIVE);
			mapinit("virtual",     ExtendedVersion::PROPERTIES_VIRTUAL);
			mapinit("set",         ExtendedVersion::PROPERTIES_SET);
		}

		ATTRIBUTE_PURE ExtendedVersion::Properties getProperties(const string& s) const {
			PropertiesMap::const_iterator i(find(s));
			if(i != end()) {
				return i->second;
			}
			return ExtendedVersion::PROPERTIES_NONE;
		}
};

static PropertiesMap *properties_map = NULLPTR;

void ExtendedVersion::init_static() {
	eix_assert_static(restrict_map == NULLPTR);
	restrict_map = new RestrictMap;
	properties_map = new PropertiesMap;
}

ExtendedVersion::Restrict ExtendedVersion::calcRestrict(const string& str) {
	eix_assert_static(restrict_map != NULLPTR);
	Restrict r(RESTRICT_NONE);
	WordVec restrict_words;
	split_string(&restrict_words, str);
	for(WordVec::const_iterator it(restrict_words.begin());
		likely(it != restrict_words.end()); ++it) {
		r |= restrict_map->getRestrict(*it);
	}
	return r;
}

ExtendedVersion::Properties ExtendedVersion::calcProperties(const string& str) {
	eix_assert_static(properties_map != NULLPTR);
	Properties p(PROPERTIES_NONE);
	WordVec properties_words;
	split_string(&properties_words, str);
	for(WordVec::const_iterator it(properties_words.begin());
		it != properties_words.end(); ++it) {
		p |= properties_map->getProperties(*it);
	}
	return p;
}

bool ExtendedVersion::have_bin_pkg(const PortageSettings *ps, const Package *pkg) const {
	return (have_tbz_pkg(ps, pkg) || (num_pak_pkg(ps, pkg) != 0));
}

bool ExtendedVersion::have_bin_pkg(const PortageSettings *ps, const Package *pkg, CountBinPkg minimal) const {
	// Call tests only if necessary; prefer have_tbz_pkg if sufficient
	if(unlikely(minimal == 0)) {
		return true;
	}
	if(minimal == 1) {
		return have_bin_pkg(ps, pkg);
	}
	CountBinPkg have(num_pak_pkg(ps, pkg));
	return ((have >= minimal) || ((have + 1 == minimal) && have_tbz_pkg(ps, pkg)));
}

bool ExtendedVersion::have_tbz_pkg(const PortageSettings *ps, const Package *pkg) const {
	switch(have_bin_pkg_m & HAVEBINPKG_TBZ) {
		case HAVEBINPKG_UNKNOWN: {
				const string& s((*ps)["PKGDIR"]);
				if((s.empty()) || !is_file((s + "/" + pkg ->category + "/" + pkg->name + "-" + getFull() + ".tbz2").c_str())) {
					have_bin_pkg_m |= HAVEBINPKG_TBZ_NO;
					return false;
				}
				have_bin_pkg_m |= HAVEBINPKG_TBZ_YES;
			}
			break;
		case HAVEBINPKG_TBZ_NO:
			return false;
		default:
		// case HAVEBINPKG_TBZ_YES:
			break;
	}
	return true;
}

ExtendedVersion::CountBinPkg ExtendedVersion::num_pak_pkg(const PortageSettings *ps, const Package *pkg) const {
	if(likely((have_bin_pkg_m & HAVEBINPKG_PAK) != HAVEBINPKG_UNKNOWN)) {
		return count_pak_m;
	}
	have_bin_pkg_m |= HAVEBINPKG_PAK;
	count_pak_m = 0;
	const string& s((*ps)["PKGDIR"]);
	if(unlikely(s.empty())) {
		return 0;
	}
	const string pkgs = s + "/" + pkg->category + "/" + pkg->name + "/";
	WordVec bin_packages;
	if(unlikely(!pushback_files(pkgs, &bin_packages, NULLPTR, 1))) {
		return 0;
	}
	const string pkg_search = pkgs + pkg->name + "-" + getFull() + "-";
	for(WordVec::const_iterator it(bin_packages.begin());
		it != bin_packages.end(); ++it) {
		const string &name(*it);
		string::size_type i(pkg_search.size());
		if((i > name.size()) || (std::strncmp(pkg_search.c_str(), name.c_str(), i) != 0)) {
			continue;
		}
		for(; (i != name.size()) && my_isdigit(name[i]); ++i) {
		}
		if((i + 5 == name.size()) && (strncasecmp(".xpak", name.c_str() + i, 5) == 0)) {
			++count_pak_m;
		}
	}
	return count_pak_m;
}
