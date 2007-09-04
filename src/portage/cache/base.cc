/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
	bool somenum = false;
	for(const char *s = ver.c_str() + i + 2; *s; ++s)
	{
		if((*s < '0') || (*s >'9'))
			return string::npos;
		somenum = true;
	}
	if(somenum)
		return i;
	return string::npos;
}

void BasicCache::setScheme(const char *prefix, const char *prefixport, const char *prefixexec, std::string scheme)
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
	if(use_prefixexec() && prefixexec) {
		have_prefix_exec = true;
		m_prefix_exec = prefixexec;
	}
	else {
		have_prefix_exec = false;
		m_prefix_exec = "";
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
	if(have_prefix_exec) {
		ret.append(" (exec: ");
		ret.append(m_prefix_exec);
		ret.append(")");
	}
	return ret;
}

void BasicCache::env_add_package(map<string,string> &env, const Package &package, const Version &version, const string &ebuild_dir, const char *ebuild_full) const
{
	string full = version.getFull();
	string eroot;
	const char *root = getenv("ROOT");
	if(root) {
		env["ROOT"] = root;
		eroot = root + m_prefix;
	}
	else {
		env["ROOT"] = "/";
		eroot = m_prefix;
	}
	if(have_prefix) {
		env["EPREFIX"] = m_prefix;
		env["EROOT"]   = eroot;
	}
	env["PORTDIR_OVERLAY"] = (*portagesettings)["PORTDIR_OVERLAY"];
	string portdir         = (*portagesettings)["PORTDIR"];
	env["PORTDIR"]         = portdir;

	env["EBUILD"]       = ebuild_full;
	env["O"]            = ebuild_dir;
	env["FILESDIR"]     = ebuild_dir + "/files";
	env["ECLASSDIR"]    = eroot + portdir + "/eclass";
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

#if 0
Package *
addPackage(Category &v, const string &cat, const string &pkg)
{
	Package *p = new Package(cat, pkg);
	OOM_ASSERT(p);
	v.push_back(p);
	return p;
}

Package *
findPackage(Category &v, const char *pkg)
{
	Package *ret = NULL;
	for(Category::iterator i = v.begin();
		i != v.end();
		++i)
	{
		if((*i)->name == pkg) {
			ret = *i;
			break;
		}
	}
	return ret;
}

bool
deletePackage(Category &v, const string &pkg)
{
	bool ret = false;
	for(Category::iterator i = v.begin();
		i != v.end();
		++i)
	{
		if((*i)->name == pkg) {
			delete *i;
			v.erase(i);
			ret = true;
			break;
		}
	}
	return false;
}
#endif
