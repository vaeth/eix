// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <cassert>

#include <map>
#include <string>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "portage/conf/portagesettings.h"
#include "portage/extendedversion.h"
#include "portage/package.h"

using std::map;
using std::string;
using std::vector;

class RestrictMap : public map<string, ExtendedVersion::Restrict> {
	private:
		void mapinit(const char *s, ExtendedVersion::Restrict r)
		{ (*this)[s] = r; }

	public:
		RestrictMap()
		{
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

		ExtendedVersion::Restrict getRestrict(const string& s) const ATTRIBUTE_PURE
		{
			RestrictMap::const_iterator i(find(s));
			if(i != end())
				return i->second;
			return ExtendedVersion::RESTRICT_NONE;
		}
};

static RestrictMap *restrict_map = NULLPTR;

class PropertiesMap : public map<string, ExtendedVersion::Properties> {
	private:
		void mapinit(const char *s, ExtendedVersion::Properties p)
		{ (*this)[s] = p; }
	public:
		PropertiesMap()
		{
			mapinit("interactive", ExtendedVersion::PROPERTIES_INTERACTIVE);
			mapinit("live",        ExtendedVersion::PROPERTIES_LIVE);
			mapinit("virtual",     ExtendedVersion::PROPERTIES_VIRTUAL);
			mapinit("set",         ExtendedVersion::PROPERTIES_SET);
		}

		ExtendedVersion::Properties getProperties(const string& s) const ATTRIBUTE_PURE
		{
			PropertiesMap::const_iterator i(find(s));
			if(i != end())
				return i->second;
			return ExtendedVersion::PROPERTIES_NONE;
		}
};

static PropertiesMap *properties_map = NULLPTR;

void
ExtendedVersion::init_static()
{
	if(unlikely(restrict_map != NULLPTR))
		return;
	restrict_map = new RestrictMap;
	properties_map = new PropertiesMap;
}

ExtendedVersion::Restrict
ExtendedVersion::calcRestrict(const string &str)
{
	assert(restrict_map != NULLPTR);  // has init_static() been called?
	Restrict r(RESTRICT_NONE);
	vector<string> restrict_words;
	split_string(&restrict_words, str);
	for(vector<string>::const_iterator it(restrict_words.begin());
		likely(it != restrict_words.end()); ++it) {
		r |= restrict_map->getRestrict(*it);
	}
	return r;
}

ExtendedVersion::Properties
ExtendedVersion::calcProperties(const string &str)
{
	assert(properties_map != NULLPTR);  // has init_static() been called?
	Properties p(PROPERTIES_NONE);
	vector<string> properties_words;
	split_string(&properties_words, str);
	for(vector<string>::const_iterator it(properties_words.begin());
		it != properties_words.end(); ++it) {
		p |= properties_map->getProperties(*it);
	}
	return p;
}

bool
ExtendedVersion::have_bin_pkg(const PortageSettings *ps, const Package *pkg) const
{
	switch(have_bin_pkg_m) {
		case HAVEBINPKG_UNKNOWN:
			{
				const string &s((*ps)["PKGDIR"]);
				if((s.empty()) || !is_file((s + "/" + pkg ->category + "/" + pkg->name + "-" + getFull() + ".tbz2").c_str())) {
					have_bin_pkg_m = HAVEBINPKG_NO;
					return false;
				}
				have_bin_pkg_m = HAVEBINPKG_YES;
			}
			break;
		case HAVEBINPKG_NO:
			return false;
		default:
		// case HAVEBINPKG_YES;
			break;
	}
	return true;
}
