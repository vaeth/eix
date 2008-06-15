// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "packagetest.h"
#include <portage/version.h>
#include <portage/set_stability.h>
#include <eixTk/filenames.h>

using namespace std;

const PackageTest::MatchField
		PackageTest::NONE,
		PackageTest::NAME,
		PackageTest::DESCRIPTION,
		PackageTest::PROVIDE,
		PackageTest::LICENSE,
		PackageTest::CATEGORY,
		PackageTest::CATEGORY_NAME,
		PackageTest::HOMEPAGE,
		PackageTest::IUSE,
		PackageTest::USE_ENABLED,
		PackageTest::USE_DISABLED,
		PackageTest::SLOT,
		PackageTest::INSTALLED_SLOT;

const PackageTest::TestInstalled
		PackageTest::INS_NONE,
		PackageTest::INS_NONEXISTENT,
		PackageTest::INS_OVERLAY,
		PackageTest::INS_MASKED;

const PackageTest::TestStability
		PackageTest::STABLE_NONE,
		PackageTest::STABLE_FULL,
		PackageTest::STABLE_TESTING,
		PackageTest::STABLE_NONMASKED,
		PackageTest::STABLE_SYSTEM;

PackageTest::PackageTest(VarDbPkg &vdb, PortageSettings &p, const SetStability &set_stability, const DBHeader &dbheader)
{
	vardbpkg =  &vdb;
	portagesettings = &p;
	stability= &set_stability;
	header   = &dbheader;
	overlay_list = overlay_only_list = in_overlay_inst_list = NULL;
	from_overlay_inst_list = NULL;
	from_foreign_overlay_inst_list = NULL;
	portdir = (*portagesettings)["PORTDIR"].c_str();

	field    = PackageTest::NONE;
	need     = PackageReader::NONE;
	obsolete = world = overlay = installed = invert = upgrade = slotted =
			dup_versions = dup_packages = false;
	restrictions = ExtendedVersion::RESTRICT_NONE;
	test_installed = INS_NONE;
	test_stability_default = test_stability_local = test_stability_nonlocal = STABLE_NONE;
}

void
PackageTest::calculateNeeds() {
	need = PackageReader::NONE;
	if(field & (SLOT | INSTALLED_SLOT))
		setNeeds(PackageReader::VERSIONS);
	if(field & HOMEPAGE)
		setNeeds(PackageReader::HOMEPAGE);
	if(field & PROVIDE)
		setNeeds(PackageReader::PROVIDE);
	if(field & LICENSE)
		setNeeds(PackageReader::LICENSE);
	if(field & DESCRIPTION)
		setNeeds(PackageReader::DESCRIPTION);
	if(field & CATEGORY)
		setNeeds(PackageReader::NONE);
	if(field & CATEGORY_NAME)
		setNeeds(PackageReader::NAME);
	if(field & NAME)
		setNeeds(PackageReader::NAME);
	if(field & IUSE)
		setNeeds(PackageReader::COLL_IUSE);
	if(field & (USE_ENABLED | USE_DISABLED))
		setNeeds(PackageReader::NAME);
	if(installed)
		setNeeds(PackageReader::NAME);
	if(dup_packages || dup_versions || slotted ||
		upgrade || overlay || obsolete || world ||
		from_overlay_inst_list || from_foreign_overlay_inst_list ||
		overlay_list || overlay_only_list || in_overlay_inst_list ||
		(restrictions != ExtendedVersion::RESTRICT_NONE) ||
		(test_stability_default != STABLE_NONE) ||
		(test_stability_local != STABLE_NONE) ||
		(test_stability_nonlocal != STABLE_NONE))
		setNeeds(PackageReader::VERSIONS);
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
	else if(p == "IUSE")          ret = IUSE;
	else if(p == "SLOT")          ret = SLOT;
	else if(p == "INSTALLED_SLOT")ret = INSTALLED_SLOT;
	else throw ExBasic("Can't find MatchField called %r") % p;
	return ret;
}

PackageTest::MatchField
PackageTest::get_matchfield(const char *p) throw(ExBasic)
{
	EixRc &rc = get_eixrc(NULL);
	vector<string> order = split_string(rc["MATCH_ORDER"].c_str(), " \t\n\r", true);
	for(vector<string>::iterator it = order.begin();
		it != order.end();
		++it)
	{
		Regex re(rc["MATCH_" + *it + "_IF"].c_str());
		if(re.match(p))
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
	if((field & NAME) && (*algorithm)(pkg->name.c_str(), pkg))
		return true;

	if((field & DESCRIPTION) && (*algorithm)(pkg->desc.c_str(), pkg))
		return true;

	if((field & LICENSE) && (*algorithm)(pkg->licenses.c_str(), pkg))
		return true;

	if((field & CATEGORY) && (*algorithm)(pkg->category.c_str(), pkg))
		return true;

	if(field & CATEGORY_NAME && (*algorithm)((pkg->category + "/" + pkg->name).c_str(), pkg))
		return true;

	if((field & HOMEPAGE) && (*algorithm)(pkg->homepage.c_str(), pkg))
		return true;

	if((field & PROVIDE) && (*algorithm)(pkg->provide.c_str(), pkg))
		return true;

	if(field & SLOT) {
		for(Package::iterator it(pkg->begin());
			it != pkg->end(); ++it) {
			if((*algorithm)(it->slotname.c_str(), pkg))
				return true;
		}
	}

	if(field & IUSE) {
		//vector<string> s=split_string(pkg->coll_iuse);
		for(set<string>::const_iterator it = pkg->coll_iuse_vector().begin();
			it != pkg->coll_iuse_vector().end(); ++it) {
			if((*algorithm)(it->c_str(), NULL))
				return true;
		}
	}

	if(! (field & (USE_ENABLED | USE_DISABLED | INSTALLED_SLOT)))
		return false;

	vector<InstVersion> *installed_versions = vardbpkg->getInstalledVector(*pkg);
	if(!installed_versions)
		return false;

	if(field & INSTALLED_SLOT) {
		for(vector<InstVersion>::iterator it = installed_versions->begin();
			it != installed_versions->end(); ++it) {
			if(!vardbpkg->readSlot(*pkg, *it))
				continue;
			if((*algorithm)(it->slotname.c_str(), pkg))
				return true;
		}
	}

	if(field & (USE_ENABLED | USE_DISABLED)) {
		for(vector<InstVersion>::iterator it = installed_versions->begin();
			it != installed_versions->end(); ++it) {
			if(!vardbpkg->readUse(*pkg, *it))
				continue;
			if(field & USE_ENABLED) {
				for(set<string>::iterator uit = (it->usedUse).begin();
					uit != (it->usedUse).end(); ++uit) {
					if((*algorithm)(uit->c_str(), NULL))
						return true;
				}
			}
			if(field & USE_DISABLED) {
				for(vector<string>::iterator uit = (it->inst_iuse).begin();
					uit != (it->inst_iuse).end(); ++uit) {
					if(!(*algorithm)(uit->c_str(), NULL))
						continue;
					if((it->usedUse).find(*uit) == (it->usedUse).end())
						return true;
				}
			}
		}
	}

	return false;
}

bool
PackageTest::have_redundant(const Package &p, Keywords::Redundant r, const RedAtom &t) const
{
	r &= t.red;
	if(r == Keywords::RED_NOTHING)
		return false;
	if(t.only & r)
	{
		if((vardbpkg->numInstalled(p) != 0) != ((t.oins & r) != 0))
			return false;
	}
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
				short is_installed = vardbpkg->isInstalledVersion(p, *pi, *header, portdir);
				// If in doubt about the overlay only consider as installed
				// if the current version was not treated yet, i.e. (since we use reverse_iterator)
				// if it is the highest version (and so usually from the last overlay)
				if(is_installed < 0) {
					if(prev_ver && (**pi == *prev_ver))
						is_installed = 0;
				}
				if(test_uninstalled == !is_installed)
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
				// in contrast to the above loop, we now in doubt
				// do not distinguish overlays.
				if(test_uninstalled ==
					!(vardbpkg->isInstalledVersion(p, *pi, *header, portdir)))
					return true;
			}
		}
		return false;
	}
}

bool
PackageTest::have_redundant(const Package &p, Keywords::Redundant r) const
{
	if(r == Keywords::RED_NOTHING)
		return false;
	if(have_redundant(p, r, first_test))
		return true;
	if(have_redundant(p, r, second_test))
		return true;
	return false;
}

static bool
stabilitytest(const Package *p, PackageTest::TestStability what)
{
	if(what == PackageTest::STABLE_NONE)
		return true;
	for(Package::const_iterator it = p->begin(); it != p->end(); ++it) {
		if(what & PackageTest::STABLE_SYSTEM) {
			if(!it->maskflags.isSystem())
				continue;
			if(what == PackageTest::STABLE_SYSTEM)
				return true;
		}
		if(it->maskflags.isHardMasked())
			continue;
		if(it->keyflags.isStable()) {
			return true;
		}
		if(what & PackageTest::STABLE_FULL)
			continue;
		if(it->keyflags.isUnstable())
			return true;
		if(what & PackageTest::STABLE_TESTING)
			continue;
		// what & PackageTest::STABLE_NONMASKED
			return true;
	}
	return false;
}

#define get_p() do { \
	if(!p) \
		p = pkg->get(); \
} while(0)


bool
PackageTest::match(PackageReader *pkg) const
{
	Package *p = NULL;

	pkg->read(need);

	/**
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

	   If a test needs that keywords/mask are set correctly,
	   recall that they are not set in the database.
	   You have to do three things in such a case.
	   (Note that these are time consuming, so do other tests first,
	   if possible.)
	   1. Place your test after the "obsolete" tests.
	      These need a special treatment, since they certainly must
	      calculate StabilityLocal. It might save time to use their
	      results (implicitly in step 2.)
	   2. Call one of
		StabilityDefault(p)
		StabilityLocal(p)
		StabilityNonlocal(p)
	      depending on which type of stability you want.
	      (Default means according to LOCAL_PORTAGE_CONFIG,
	      Nonlocal means as with LOCAL_PORTAGE_CONFIG=false)
	   3. Once more: remember to modify "need" in CalculateNeeds() to
	      ensure the versions really have been read for the package.
	*/

	if(algorithm.get() != NULL) {
		get_p();
		if(!stringMatch(p))
			return invert;
	}

	if(slotted) { // -1 or -2
		get_p();
		if(! (p->have_nontrivial_slots()))
			return invert;
		if(multi_slot)
			if( (p->slotlist()).size() <= 1 )
				return invert;
	}

	if(overlay) { // -O
		get_p();
		if(!(p->largest_overlay))
			return invert;
	}

	if(overlay_list) { // --in-overlay
		get_p();
		bool have = false;
		for(Package::iterator it = p->begin(); it != p->end(); ++it) {
			if(overlay_list->find(it->overlay_key) == overlay_list->end())
				continue;
			have = true;
			break;
		}
		if(!have)
			return invert;
	}

	if(overlay_only_list) { // --only-in-overlay
		get_p();
		for(Package::iterator it = p->begin(); it != p->end(); ++it) {
			if(overlay_only_list->find(it->overlay_key) == overlay_only_list->end())
				return invert;
		}
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

	if(in_overlay_inst_list) { // --installed-in-[some-]overlay
		get_p();
		bool have = false;
		bool get_installed = true;
		vector<InstVersion> *installed_versions = NULL;
		for(Package::iterator it = p->begin(); it != p->end(); ++it) {
			if(in_overlay_inst_list->find(it->overlay_key) == in_overlay_inst_list->end())
				continue;
			if(get_installed) {
				get_installed = false;
				installed_versions = vardbpkg->getInstalledVector(*p);
			}
			if(!installed_versions)
				continue;
			if(VarDbPkg::isInVec(installed_versions,*it)) {
				have = true;
				break;
			}
		}
		if(!have)
			return invert;
	}

	if(from_overlay_inst_list ||
	   from_foreign_overlay_inst_list) { // -J or --installed-from-overlay
		get_p();
		bool have = false;
		vector<InstVersion> *installed_versions = vardbpkg->getInstalledVector(*p);
		if(!installed_versions)
			return invert;
		for(vector<InstVersion>::iterator it = installed_versions->begin();
			it != installed_versions->end(); ++it) {
			if(vardbpkg->readOverlay(*p, *it, *header, portdir)) {
				if(!from_overlay_inst_list)
					continue;
				if(from_overlay_inst_list->find(it->overlay_key) == from_overlay_inst_list->end())
					continue;
				have = true;
				break;
			}
			if((it->overlay_keytext).empty())
				continue;
			if(!from_foreign_overlay_inst_list)
				continue;
			for(vector<string>::iterator foreign = from_foreign_overlay_inst_list->begin();
				foreign != from_foreign_overlay_inst_list->end(); ++foreign) {
				if(foreign->empty() ||
					same_filenames(foreign->c_str(), (it->overlay_keytext).c_str(), true, false) ||
					same_filenames(foreign->c_str(), (it->overlay_keytext).c_str(), true, true)) {
					have = true;
					break;
				}
			}
			if(have)
				break;
		}
		if(!have)
			return invert;
	}

	if(dup_packages) { // -d
		get_p();
		if(dup_packages_overlay)
		{
			if(!(p->at_least_two_overlays()))
				return invert;
		}
		else if(p->have_same_overlay_key())
			return invert;
	}

	if(dup_versions) { // -D
		get_p();
		Package::Duplicates testfor = ((dup_versions_overlay) ?
				Package::DUP_OVERLAYS : Package::DUP_SOME);
		if(((p->have_duplicate_versions) & testfor) != testfor)
			return invert;
	}

	if(restrictions != ExtendedVersion::RESTRICT_NONE) {
		get_p();
		bool found = false;
		for(Package::iterator it = p->begin(); it != p->end(); ++it) {
			if(((it->restrictFlags) & restrictions) == restrictions) {
				found = true;
				break;
			}
		}
		if(!found) {
			vector<InstVersion> *installed_versions = vardbpkg->getInstalledVector(*p);
			if(!installed_versions)
				return invert;
			for(vector<InstVersion>::iterator it = installed_versions->begin();
				it != installed_versions->end(); ++it) {
				vardbpkg->readRestricted(*p, *it, *header, portdir);
				if(((it->restrictFlags) & restrictions) == restrictions) {
					found = true;
					break;
				}
			}
			if(!found)
				return invert;
		}
	}

	while(obsolete) {  // -T; loop, because we break in case of success
		// Can some test succeed at all?
		if((test_installed == INS_NONE) &&
			(redundant_flags == Keywords::RED_NOTHING))
			return invert;

		get_p();

		Keywords::Redundant r = redundant_flags & Keywords::RED_ALL_MASKSTUFF;
		if(r)
		{
			r &= nowarn_mask(*p);
			if(r && portagesettings->user_config->setMasks(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_MASK))
					break;
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_UNMASK))
					break;
				if(have_redundant(*p, r & Keywords::RED_MASK))
					break;
				if(have_redundant(*p, r & Keywords::RED_UNMASK))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_MASK))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_UNMASK))
					break;
			}
		}
		r = redundant_flags & Keywords::RED_ALL_KEYWORDS;
		if(r)
		{
			r &= nowarn_keywords(*p);
			if(r && portagesettings->user_config->setKeyflags(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE))
					break;
				if(have_redundant(*p, r & Keywords::RED_MIXED))
					break;
				if(have_redundant(*p, r & Keywords::RED_WEAKER))
					break;
				if(have_redundant(*p, r & Keywords::RED_STRANGE))
					break;
				if(have_redundant(*p, r & Keywords::RED_NO_CHANGE))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_KEYWORDS))
					break;
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_LINE))
					break;
				if(have_redundant(*p, r & Keywords::RED_MINUSASTERISK))
					break;
			}
		}
		r = redundant_flags & Keywords::RED_ALL_USE;
		if(r) {
			r &= nowarn_use(*p);
			if(r && portagesettings->user_config->CheckUse(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_USE))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_USE))
					break;
			}
		}
		r = redundant_flags & Keywords::RED_ALL_CFLAGS;
		if(r) {
			r &= nowarn_cflags(*p);
			if(r && portagesettings->user_config->CheckCflags(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_CFLAGS))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_CFLAGS))
					break;
			}
		}
		if(test_installed == INS_NONE)
			return invert;
		TestInstalled t = test_installed & nowarn_installed(*p);
		if(t == INS_NONE)
			return invert;
		vector<InstVersion> *installed_versions = vardbpkg->getInstalledVector(*p);
		if(!installed_versions)
			return invert;
		if(t & INS_MASKED) {
			portagesettings->user_config->setMasks(p);
			portagesettings->user_config->setKeyflags(p);
		}
		vector<InstVersion>::iterator current = installed_versions->begin();
		for( ; current != installed_versions->end(); ++current)
		{
			bool not_all_found = true;
			for(Package::iterator version_it = p->begin();
				version_it != p->end(); ++version_it)
			{
				Version *version = *version_it;
				if(*version != *current)
					continue;
				if(t & INS_MASKED) {
					if(!version->keyflags.isStable())
						continue;
				}
				if(t & INS_OVERLAY) {
					if(!vardbpkg->readOverlay(*p, *current, *header, portdir))
						continue;
					if(current->overlay_key != version_it->overlay_key)
						continue;
				}
				not_all_found = false;
			}
			if(not_all_found)
				break;
		}
		if(current != installed_versions->end())
			break;
		return invert;
	}

	if(upgrade) { // -u
		get_p();
		if(upgrade_local_mode == LOCALMODE_DEFAULT)
			StabilityDefault(p);
		else if(upgrade_local_mode == LOCALMODE_LOCAL)
			StabilityLocal(p);
		else if(upgrade_local_mode == LOCALMODE_NONLOCAL)
			StabilityNonlocal(p);
		if(! (p->recommend(vardbpkg, true, true)))
			return invert;
	}

	if(test_stability_default != STABLE_NONE)
	{ // --stable, --testing, --non-masked, --system
		get_p();
		StabilityDefault(p);
		if(!stabilitytest(p, test_stability_default))
			return invert;
	}

	if(test_stability_local != STABLE_NONE)
	{ // --stable+, --testing+, --non-masked+, --system+
		get_p();
		StabilityLocal(p);
		if(!stabilitytest(p, test_stability_local))
			return invert;
	}

	if(test_stability_nonlocal != STABLE_NONE)
	{ // --stable-, --testing-, --non-masked-, --system-
		get_p();
		StabilityNonlocal(p);
		if(!stabilitytest(p, test_stability_nonlocal))
			return invert;
	}

	if(world) { // --world
		get_p();
		StabilityDefault(p);
		if(!p->is_world_package())
			return invert;
	}

	// all tests succeeded:
	return (!invert);
}

static void
getlines(const char *varname, vector<string> &lines)
{
	EixRc &rc = get_eixrc(NULL);
	lines.clear();
	string &name = rc[varname];
	if(name.empty())
		return;
	pushback_lines(name.c_str(), &lines, false, true);
}

Keywords::Redundant
PackageTest::nowarn_keywords(const Package &p)
{
	static bool know_file = false;
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(!know_file) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("KEYWORDS_NOWARN", lines);
		for(vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
			vector<string> s = split_string(*it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et = s.begin();
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_KEYWORDS;
			else
				r = i->second;
			for(++et; et != s.end(); ++et) {
				if(strcasecmp(et->c_str(), "in_keywords") == 0)
					r &= ~Keywords::RED_IN_KEYWORDS;
				else if(strcasecmp(et->c_str(), "no_change") == 0)
					r &= ~Keywords::RED_NO_CHANGE;
				else if(strcasecmp(et->c_str(), "double") == 0)
					r &= ~Keywords::RED_DOUBLE;
				else if(strcasecmp(et->c_str(), "mixed") == 0)
					r &= ~Keywords::RED_MIXED;
				else if(strcasecmp(et->c_str(), "weaker") == 0)
					r &= ~Keywords::RED_WEAKER;
				else if(strcasecmp(et->c_str(), "strange") == 0)
					r &= ~Keywords::RED_STRANGE;
				else if(strcasecmp(et->c_str(), "minusasterisk") == 0)
					r &= ~Keywords::RED_MINUSASTERISK;
				else if(strcasecmp(et->c_str(), "double_line") == 0)
					r &= ~Keywords::RED_DOUBLE_LINE;
			}
			m[s[0]] = r;
		}
	}
	i = m.find(p.category + "/" + p.name);
	if(i == m.end())
		return Keywords::RED_ALL_KEYWORDS;
	return i->second;
}

Keywords::Redundant
PackageTest::nowarn_mask(const Package &p)
{
	static bool know_file = false;
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(!know_file) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("MASK_NOWARN", lines);
		for(vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
			vector<string> s = split_string(*it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et = s.begin();
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_MASKSTUFF;
			else
				r = i->second;
			for(++et; et != s.end(); ++et) {
				if(strcasecmp(et->c_str(), "in_mask") == 0)
					r &= ~Keywords::RED_IN_MASK;
				else if(strcasecmp(et->c_str(), "mask_no_change") == 0)
					r &= ~Keywords::RED_MASK;
				else if(strcasecmp(et->c_str(), "double_masked") == 0)
					r &= ~Keywords::RED_DOUBLE_MASK;
			}
			m[s[0]] = r;
		}
		getlines("UNMASK_NOWARN", lines);
		for(vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
			vector<string> s = split_string(*it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et = s.begin();
			i = m.find(*et);
			Keywords::Redundant r;
			if(i == m.end())
				r = Keywords::RED_ALL_MASKSTUFF;
			else
				r = i->second;
			for(++et; et != s.end(); ++et) {
				if(strcasecmp(et->c_str(), "in_unmask") == 0)
					r &= ~Keywords::RED_IN_UNMASK;
				else if(strcasecmp(et->c_str(), "unmask_no_change") == 0)
					r &= ~Keywords::RED_UNMASK;
				else if(strcasecmp(et->c_str(), "double_unmasked") == 0)
					r &= ~Keywords::RED_DOUBLE_UNMASK;
			}
			m[s[0]] = r;
		}
	}
	i = m.find(p.category + "/" + p.name);
	if(i == m.end())
		return Keywords::RED_ALL_MASKSTUFF;
	return i->second;
}

Keywords::Redundant
PackageTest::nowarn_use(const Package &p)
{
	static bool know_file = false;
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(!know_file) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("USE_NOWARN", lines);
		for(vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
			vector<string> s = split_string(*it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et = s.begin();
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_USE;
			else
				r = i->second;
			for(++et; et != s.end(); ++et) {
				if(strcasecmp(et->c_str(), "in_use") == 0)
					r &= ~Keywords::RED_IN_USE;
				else if(strcasecmp(et->c_str(), "double_use") == 0)
					r &= ~Keywords::RED_DOUBLE_USE;
			}
			m[s[0]] = r;
		}
	}
	i = m.find(p.category + "/" + p.name);
	if(i == m.end())
		return Keywords::RED_ALL_USE;
	return i->second;
}

Keywords::Redundant
PackageTest::nowarn_cflags(const Package &p)
{
	static bool know_file = false;
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(!know_file) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("CFLAGS_NOWARN", lines);
		for(vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
			vector<string> s = split_string(*it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et = s.begin();
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_CFLAGS;
			else
				r = i->second;
			for(++et; et != s.end(); ++et) {
				if(strcasecmp(et->c_str(), "in_cflags") == 0)
					r &= ~Keywords::RED_IN_CFLAGS;
				else if(strcasecmp(et->c_str(), "double_cflags") == 0)
					r &= ~Keywords::RED_DOUBLE_CFLAGS;
			}
			m[s[0]] = r;
		}
	}
	i = m.find(p.category + "/" + p.name);
	if(i == m.end())
		return Keywords::RED_ALL_CFLAGS;
	return i->second;
}

PackageTest::TestInstalled
PackageTest::nowarn_installed(const Package &p)
{
	static bool know_file = false;
	static map<string, TestInstalled> m;
	map<string, TestInstalled>::const_iterator i;
	if(!know_file) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("INSTALLED_NOWARN", lines);
		for(vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
			vector<string> s = split_string(*it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et = s.begin();
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = INS_SOME;
			else
				r = i->second;
			for(++et; et != s.end(); ++et) {
				if(strcasecmp(et->c_str(), "nonexistent") == 0)
					r = INS_NONE;
				else if(strcasecmp(et->c_str(), "masked") == 0)
					r &= ~INS_OVERLAY;
				else if(strcasecmp(et->c_str(), "other_overlay") == 0)
					r &= ~INS_MASKED;
			}
			m[s[0]] = r;
		}
	}
	i = m.find(p.category + "/" + p.name);
	if(i == m.end())
		return INS_SOME;
	return i->second;
}
