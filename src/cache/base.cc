// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "base.h"

#include <portage/package.h>
#include <portage/packagetree.h>
#include <portage/conf/portagesettings.h>

using namespace std;

static inline
string::size_type revision_index(const string &ver)
{
	string::size_type i = ver.rfind("-r");
	if(i == string::npos)
		return string::npos;
	if (ver.find_first_not_of("0123456789", i + 2) == string::npos)
		return i;
	return string::npos;
}

void BasicCache::setScheme(const char *prefix, const char *prefixport, std::string scheme)
{
	m_scheme = scheme;
	if(use_prefixport())
		prefix = prefixport;
	if(prefix) {
		have_prefix = true;
		m_prefix = prefix;
	}
	else {
		have_prefix = false;
		m_prefix = "";
	}
}

string BasicCache::getPrefixedPath() const
{
	if(have_prefix) {
		return m_prefix + m_scheme;
	}
	return m_scheme;
}

string BasicCache::getPathHumanReadable() const
{
	string ret = m_scheme;
	if(have_prefix) {
		ret.append(" in ");
		ret.append(m_prefix);
	}
	return ret;
}

void BasicCache::env_add_package(map<string,string> &env, const Package &package, const Version &version, const string &ebuild_dir, const char *ebuild_full) const
{
	string full = version.getFull();
	string eroot;

	// Set default variables

	const char *envptr = getenv("PATH");
	if(envptr)
		env["PATH"] = envptr;
	envptr = getenv("ROOT");
	if(envptr) {
		env["ROOT"] = envptr;
		eroot = envptr + m_prefix;
	}
	else {
		env["ROOT"] = "/";
		eroot = m_prefix;
	}
	if(have_prefix) {
		env["EPREFIX"] = m_prefix;
		env["EROOT"]   = eroot;
	}
	string portdir      = (*portagesettings)["PORTDIR"];
	env["ECLASSDIR"]    = eroot + portdir + "/eclass";

	// Set variables from portagesettings (make.globals/make.conf/...)
	// (Possibly overriding defaults)

	for(PortageSettings::const_iterator it = portagesettings->begin();
		it != portagesettings->end(); ++it) {
		env[it->first] = it->second;
	}

	// Set ebuild-specific variables

	env["EBUILD"]       = ebuild_full;
	env["O"]            = ebuild_dir;
	env["FILESDIR"]     = ebuild_dir + "/files";
	env["EBUILD_PHASE"] = "depend";
	env["CATEGORY"]     = package.category;
	env["PN"]           = package.name;
	env["PVR"]          = full;
	env["PF"]           = package.name + "-" + full;
	string mainversion;
	string::size_type ind = revision_index(full);
	if(ind == string::npos) {
		env["PR"]   = "r0";
		mainversion = full;
	}
	else {
		env["PR"]   = full.substr(ind + 1);
		mainversion = full.substr(0, ind);
	}
	env["PV"]           = mainversion;
	env["P"]            = package.name + "-" + mainversion;
}
