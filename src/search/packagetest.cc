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

#include "packagetest.h"
#include <portage/version.h>

using namespace std;

const PackageTest::MatchField PackageTest::NONE          = 0x00, /* Search in name */
	  PackageTest::NAME          = 0x01, /* Search in name */
	  PackageTest::DESCRIPTION   = 0x02, /* Search in description */
	  PackageTest::PROVIDE       = 0x04, /* Search in provides */
	  PackageTest::LICENSE       = 0x08, /* Search in license */
	  PackageTest::CATEGORY      = 0x10, /* Search in category */
	  PackageTest::CATEGORY_NAME = 0x20, /* Search in category/name */
	  PackageTest::HOMEPAGE      = 0x40; /* Search in homepage */

PackageTest::PackageTest(VarDbPkg *vdb)
{
	vardbpkg = vdb;
	field    = PackageTest::NONE;
	need     = PackageReader::NONE;
	invert   = installed = dup_versions = false;
	portagesettings = NULL;
}

void
PackageTest::calculateNeeds() {
	map<MatchField,PackageReader::Attributes> smap;
	smap[HOMEPAGE]      = PackageReader::VERSIONS;
	smap[PROVIDE]       = PackageReader::PROVIDE;
	smap[LICENSE]       = PackageReader::LICENSE;
	smap[DESCRIPTION]   = PackageReader::DESCRIPTION;
	smap[CATEGORY]      = PackageReader::NAME;
	smap[CATEGORY_NAME] = PackageReader::NAME;
	smap[NAME]          = PackageReader::NAME;

	need = PackageReader::NONE;

	for(MatchField x = HOMEPAGE;
		x > NONE;
		x >>= 1)
	{
		if (x & field)
		{
			need = smap[x];
			break;
		}
	}

	if(installed && need < PackageReader::NAME)
	{
		need = PackageReader::NAME;
	}

	if(dup_versions && need < PackageReader::VERSIONS)
	{
		need = PackageReader::VERSIONS;
	}

	if(portagesettings && need < PackageReader::VERSIONS)
	{
		need = PackageReader::VERSIONS;
	}
}

PackageTest::MatchField
PackageTest::name2field(const string &p) throw(ExBasic)
{
	MatchField ret = NONE;
	if(p == "NONE")               ret = NONE;
	else if(p == "NAME")          ret = NAME;
	else if(p == "DESCRIPTION")   ret = DESCRIPTION;
	else if(p == "LICENSE")       ret = LICENSE;
	else if(p == "CATEGORY")      ret = CATEGORY;
	else if(p == "CATEGORY_NAME") ret = CATEGORY_NAME;
	else if(p == "HOMEPAGE")      ret = HOMEPAGE;
	else if(p == "PROVIDE")       ret = PROVIDE;
	else THROW("Can't find MatchField called %s.", p.c_str());
	return ret;
}

PackageTest::MatchField
PackageTest::get_matchfield(const char *p) throw(ExBasic)
{
	EixRc &rc = get_eixrc();
	vector<string> order = split_string(rc["MATCH_ORDER"].c_str(), " \t\n\r", true);
	for(vector<string>::iterator it = order.begin();
		it != order.end();
		++it)
	{
		Regex re(rc["MATCH_" + *it + "_IF"].c_str());
		if(!regexec(re.get(), p, 0, NULL, 0))
		{
			return name2field(*it);
		}
	}
	return NAME;
}

void
PackageTest::setPattern(const char *p)
{
	if(algorithm.get() == NULL)
	{
		algorithm = auto_ptr<BaseAlgorithm>(new RegexAlgorithm());
	}

	if(field == NONE)
	{
		field = PackageTest::get_matchfield(p);
	}

	algorithm->setString(p);
}

/** Return true if pkg matches test. */
bool
PackageTest::stringMatch(Package *pkg) const
{
	if(field & NAME && (*algorithm)(pkg->name.c_str(), pkg))
	{
		return true;
	}

	if(field & DESCRIPTION && (*algorithm)(pkg->desc.c_str(), pkg))
	{
		return true;
	}

	if(field & LICENSE && (*algorithm)(pkg->licenses.c_str(), pkg))
	{
		return true;
	}

	if(field & CATEGORY && (*algorithm)(pkg->category.c_str(), pkg))
	{
		return true;
	}

	if(field & CATEGORY_NAME && (*algorithm)((pkg->category + "/" + pkg->name).c_str(), pkg))
	{
		return true;
	}

	if(field & HOMEPAGE && (*algorithm)(pkg->homepage.c_str(), pkg))
	{
		return true;
	}

	if(field & PROVIDE && (*algorithm)(pkg->provide.c_str(), pkg))
	{
		return true;
	}

	return false;
}

bool
PackageTest::have_redundant(const Package &p, Keywords::Redundant r, const RedAtom &t) const
{
	r &= t.red;
	if(r == Keywords::RED_NOTHING)
		return false;
	bool test_unrestricted = !(r & t.spc);
	bool test_uninstalled = !(r & t.ins);
	if(r & t.all)// test all, all-installed or all-uninstalled
	{
		bool rvalue = false;
		BasicVersion *prev_ver = NULL;
		for(Package::const_reverse_iterator pi = p.rbegin();
			pi != p.rend();
			prev_ver = *pi, ++pi)
		{
			// "all" should also mean at least once:
			if((pi->get_redundant()) & r)
				rvalue = true;
			else// and no failure:
			{
				if(test_unrestricted)
					return false;
				// If the current version was not yet treated (i.e.
				// we consider at most the last overlay as installed)
				if((prev_ver == NULL) ||
					(**pi != *prev_ver))
				{
					// And this version is installed
					if(vardbpkg->isInstalled(&p, *pi))
					{
						if(test_uninstalled)
							continue;
						return false;
					}
					else if(test_uninstalled)
						return false;
				}
				else if(test_uninstalled)
					return false;
			}
		}
		return rvalue;
	}
	else// test some or some-installed
	{
		for(Package::const_iterator pi = p.begin();
			pi != p.end(); ++pi)
		{
			if((pi->get_redundant()) & r)
			{
				if(test_unrestricted)
					return true;
				// in contrast to the above loop, we do not
				// distinguish overlays here.
				if(vardbpkg->isInstalled(&p, *pi))
				{
					if(test_uninstalled)
						continue;
					return true;
				}
				else if(test_uninstalled)
					return true;
			}
		}
		return false;
	}
}

bool
PackageTest::have_redundant(const Package &p, Keywords::Redundant r) const
{
	r &= redundant_flags;
	if(r == Keywords::RED_NOTHING)
		return false;
	if(have_redundant(p, r, first_test))
		return true;
	if(have_redundant(p, r, second_test))
		return true;
	return false;
}

bool
PackageTest::match(PackageReader *pkg) const
{
	bool is_match = true;
	Package *p = NULL;

	pkg->read(need);

	if(algorithm.get() != NULL) {
		p = pkg->get();
		is_match = stringMatch(p);
	}

	/* Honour the -I, -D, -T, and -! flags. */

	if(installed && is_match) {
		if(p == NULL)
			p = pkg->get();
		is_match = vardbpkg->isInstalled(p);
	}

	if(dup_versions && is_match) {
		if(p == NULL)
			p = pkg->get();
		is_match = p->have_duplicate_versions;
	}

	if(portagesettings && is_match) {
		if(p == NULL)
			p = pkg->get();
		Package user;
		if(redundant_flags != Keywords::RED_NOTHING)
		{
			user.deepcopy(*p);
			portagesettings->setStability(&user, accept_keywords);
		}
		is_match = false;
		if(redundant_flags & Keywords::RED_ALL_MASKSTUFF)
		{
			if(portagesettings->user_config->setMasks(&user, redundant_flags))
			{
				is_match = have_redundant(user, Keywords::RED_DOUBLE_MASK);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_DOUBLE_UNMASK);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_MASK);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_UNMASK);
			}
		}
		if((!is_match) &&
			(redundant_flags & Keywords::RED_ALL_KEYWORDS))
		{
			if(portagesettings->user_config->setStability(&user, accept_keywords, redundant_flags))
			{
				is_match = have_redundant(user, Keywords::RED_DOUBLE);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_MIXED);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_WEAKER);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_STRANGE);
				if(!is_match)
					is_match = have_redundant(user, Keywords::RED_NO_CHANGE);
			}
		}
	}

	return (invert ? !is_match : is_match);
}
