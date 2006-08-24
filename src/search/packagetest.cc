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

const PackageTest::MatchField
		PackageTest::NONE          = 0x00, /* Search in name */
		PackageTest::NAME          = 0x01, /* Search in name */
		PackageTest::DESCRIPTION   = 0x02, /* Search in description */
		PackageTest::PROVIDE       = 0x04, /* Search in provides */
		PackageTest::LICENSE       = 0x08, /* Search in license */
		PackageTest::CATEGORY      = 0x10, /* Search in category */
		PackageTest::CATEGORY_NAME = 0x20, /* Search in category/name */
		PackageTest::HOMEPAGE      = 0x40; /* Search in homepage */

const PackageTest::TestInstalled
		PackageTest::INS_NONE        = 0x00,
		PackageTest::INS_NONEXISTENT = 0x01,
		PackageTest::INS_MASKED      = 0x02;

PackageTest::PackageTest(VarDbPkg &vdb, PortageSettings &p)
{
	vardbpkg =  &vdb;
	portagesettings = &p;
	field    = PackageTest::NONE;
	need     = PackageReader::NONE;
	obsolete = overlay = installed = invert = update = slotted =
			dup_versions = dup_packages = false;
	test_installed = INS_NONE;
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

	if((need < PackageReader::NAME) &&
		installed )
	{
		need = PackageReader::NAME;
	}

	if((need < PackageReader::VERSIONS) &&
		(dup_packages || dup_versions || slotted ||
			update || overlay|| obsolete))
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
					if(vardbpkg->isInstalled(p, *pi))
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
				if(vardbpkg->isInstalled(p, *pi))
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

#define get_p() do { \
	if(!p) \
		p = pkg->get(); \
} while(0)

#define get_user_accept() do { \
	if(!user) { \
		accept_keywords = portagesettings->getAcceptKeywords(); \
		user = new Package; \
		user->deepcopy(*p); \
	} \
} while(0)

#define set_user_flags() do { \
	if(!mask_was_set) { \
		mask_was_set = true; \
		portagesettings->user_config->setMasks(user); \
	} \
	if(!keywords_was_set) { \
		keywords_was_set = true; \
		portagesettings->user_config->setStability(user, accept_keywords); \
	} \
} while(0)

bool
PackageTest::match(PackageReader *pkg) const
{
	Package *p = NULL;

	pkg->read(need);

	/*
	   Test the local options.
	   Each test must start with "get_p()" to get "p"; remember to modify
	   "need" in CalculateNeeds() to ensure that you will have all
	   required data in the (possibly only partly filled) package "p".
	   If a test fails, "return invert";
	   if a test succeeds, pass to the next test,
	   i.e. within the same Matchatom, we always have "-a" concatenation
	   and honor the "invert" flag.
	   (The latter might have to be modified if someday somebody wants
	   to introduce "tree-type" expressions for queries, i.e. with braces:
	   Then "abstract" subexpressions might have to be negated, too,
	   because you do not want to have "-! -( ... -)" behave like
	   "-! -a -( ... -)" or "-! -o -( ... -)").

	   If a test needs local settings of the keywords/mask, do not rely
	   that they are set in p, because the user might not want to see
	   the local settings. Instead, do three things.
	   1. Call
	      get_user_accept() (to make sure a copy of p is in "user" and
	                         "accept_keywords" is properly defined)
	      set_user_flags() (to make sure the local settings have applied)
	      Then use "user" instead of "p".
	      Note that both commands are time-consuming,
	      so do other tests first, if possible.
	   2. Make sure to place your test after the "obsolete" tests:
	      The "obsolete" tests need a special treatment, since they *must*
	      use non-user settings before (to observe the changes when
	      user-settings are applied).
	   3. Once more: remember to modify "need" in CalculateNeeds() to
	      ensure the versions really have been read for the package.
	*/

	if(algorithm.get() != NULL) {
		get_p();
		if(!stringMatch(p))
			return invert;
	}

	if(installed) { // -i or -I
		get_p();
		vector<BasicVersion>::size_type s = vardbpkg->numInstalled(*p);
		if(!s)
			return invert;
		if(s == 1)
			if(multi_installed)
				return invert;
	}

	if(slotted) { // -1 or -2
		get_p();
		if(! (p->have_nontrivial_slots))
			return invert;
		if(multi_slot)
			if( (p->slotlist).size() <= 1 )
				return invert;
	}

	if(overlay) { // -O
		get_p();
		if(!(p->largest_overlay))
			return invert;
	}

	if(dup_packages) { // -d
		get_p();
		if(dup_packages_overlay)
		{
			if(!(p->at_least_two_overlays))
				return invert;
		}
		else if(p->have_same_overlay_key)
			return invert;
	}

	if(dup_versions) { // -D
		get_p();
		Package::Duplicates testfor = ((dup_versions_overlay) ?
				Package::DUP_OVERLAYS : Package::DUP_SOME);
		if(((p->have_duplicate_versions) & testfor) != testfor)
			return invert;
	}

	Package *user = NULL;
	bool mask_was_set = false;
	bool keywords_was_set = false;
	Keywords accept_keywords;
	while(obsolete) {  // -T; loop, because we break in case of success
		// Can some test succeed at all?
		if((test_installed == INS_NONE) &&
			(redundant_flags == Keywords::RED_NOTHING))
			return invert;

		get_p();
		get_user_accept();
		portagesettings->setStability(user, accept_keywords);

		if(redundant_flags & Keywords::RED_ALL_MASKSTUFF)
		{
			mask_was_set = true;
			if(portagesettings->user_config->setMasks(user, redundant_flags))
			{
				if(have_redundant(*user, Keywords::RED_DOUBLE_MASK))
					break;
				if(have_redundant(*user, Keywords::RED_DOUBLE_UNMASK))
					break;
				if(have_redundant(*user, Keywords::RED_MASK))
					break;
				if(have_redundant(*user, Keywords::RED_UNMASK))
					break;
			}
		}
		if(redundant_flags & Keywords::RED_ALL_KEYWORDS)
		{
			keywords_was_set = true;
			if(portagesettings->user_config->setStability(user, accept_keywords, redundant_flags))
			{
				if(have_redundant(*user, Keywords::RED_DOUBLE))
					break;
				if(have_redundant(*user, Keywords::RED_MIXED))
					break;
				if(have_redundant(*user, Keywords::RED_WEAKER))
					break;
				if(have_redundant(*user, Keywords::RED_STRANGE))
					break;
				if(have_redundant(*user, Keywords::RED_NO_CHANGE))
					break;
			}
		}
		if(test_installed == INS_NONE)
			return invert;
		vector<BasicVersion> *installed_versions = vardbpkg->getInstalledVector(*p);
		if(!installed_versions)
			return invert;
		if(test_installed & INS_MASKED) {
			set_user_flags();
		}
		vector<BasicVersion>::iterator current = installed_versions->begin();
		for( ; current != installed_versions->end(); ++current)
		{
			TestInstalled found = INS_NONE;
			bool not_all_found = true;
			for(Package::iterator version_it = user->begin();
				version_it != user->end(); ++version_it)
			{
				Version *version = *version_it;
				if(*version != *current)
					continue;
				found |= INS_NONEXISTENT;
				if(version->isStable())
					found |= INS_MASKED;
				if((found & test_installed) == test_installed)
				{
					not_all_found = false;
					break;
				}
			}
			if(not_all_found)
				break;
		}
		if(current != installed_versions->end())
			break;
		return invert;
	}

	if(update) { // -u
		get_p();
		if(update_matches_local)
		{
			get_user_accept();
			set_user_flags();
			if(! user->recommend(vardbpkg, true, true))
				return invert;
		}
		else if(! p->recommend(vardbpkg, true, true))
			return invert;
	}

	// all tests succeeded:
	return (!invert);
}
