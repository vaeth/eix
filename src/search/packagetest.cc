// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "packagetest.h"

#include <database/package_reader.h>
#include <eixTk/exceptions.h>
#include <eixTk/filenames.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/regexp.h>
#include <eixTk/stringutils.h>
#include <eixTk/utils.h>
#include <eixrc/eixrc.h>
#include <eixrc/global.h>
#include <portage/basicversion.h>
#include <portage/conf/cascadingprofile.h>
#include <portage/conf/portagesettings.h>
#include <portage/extendedversion.h>
#include <portage/package.h>
#include <portage/vardbpkg.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <cstddef>
#include <cstring>

class DBHeader;
class SetStability;

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
		PackageTest::SET,
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
	vardbpkg = &vdb;
	portagesettings = &p;
	stability= &set_stability;
	header   = &dbheader;
	overlay_list = overlay_only_list = in_overlay_inst_list = NULL;
	from_overlay_inst_list = NULL;
	from_foreign_overlay_inst_list = NULL;
	portdir = (*portagesettings)["PORTDIR"].c_str();

	field    = PackageTest::NONE;
	need     = PackageReader::NONE;
	overlay = obsolete = upgrade = binary =
		installed = multi_installed =
		slotted = multi_slot =
		world = world_only_selected = world_only_file =
		worldset = worldset_only_selected =
		dup_versions = dup_packages = false;
	restrictions = ExtendedVersion::RESTRICT_NONE;
	properties = ExtendedVersion::PROPERTIES_NONE;
	test_installed = INS_NONE;
	test_instability =
	test_stability_default = test_stability_local = test_stability_nonlocal = STABLE_NONE;
}

PackageTest::~PackageTest() {
	if(overlay_list != NULL) {
		delete overlay_list;
		overlay_list = NULL;
	}
	if(overlay_only_list != NULL) {
		delete overlay_only_list;
		overlay_only_list = NULL;
	}
	if(in_overlay_inst_list != NULL) {
		delete in_overlay_inst_list;
		in_overlay_inst_list = NULL;
	}
	if(from_overlay_inst_list != NULL) {
		delete from_overlay_inst_list;
		from_overlay_inst_list = NULL;
	}
	if(from_foreign_overlay_inst_list != NULL) {
		delete from_foreign_overlay_inst_list;
		from_foreign_overlay_inst_list = NULL;
	}
}

void
PackageTest::calculateNeeds() {
	need = PackageReader::NONE;
	if(field & (SLOT | SET))
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
	if(field & (NAME | CATEGORY_NAME))
		setNeeds(PackageReader::NAME);
	if(field & IUSE)
		setNeeds(PackageReader::COLL_IUSE);
	if(field & (USE_ENABLED | USE_DISABLED | INSTALLED_SLOT))
		setNeeds(PackageReader::NAME);
	if(installed)
		setNeeds(PackageReader::NAME);
	if(dup_packages || dup_versions || slotted ||
		upgrade || overlay || obsolete || binary || world || worldset ||
		(from_overlay_inst_list != NULL) ||
		(from_foreign_overlay_inst_list != NULL) ||
		(overlay_list != NULL) || (overlay_only_list != NULL) ||
		(in_overlay_inst_list != NULL) ||
		(restrictions != ExtendedVersion::RESTRICT_NONE) ||
		(properties != ExtendedVersion::PROPERTIES_NONE) ||
		(test_instability != STABLE_NONE) ||
		(test_stability_default != STABLE_NONE) ||
		(test_stability_local != STABLE_NONE) ||
		(test_stability_nonlocal != STABLE_NONE))
		setNeeds(PackageReader::VERSIONS);
}

inline static const map<string,PackageTest::MatchField>&
static_match_field_map()
{
	static map<string,PackageTest::MatchField> match_field_map;
	match_field_map["NAME"]           = PackageTest::NAME;
	match_field_map["name"]           = PackageTest::NAME;
	match_field_map["DESCRIPTION"]    = PackageTest::DESCRIPTION;
	match_field_map["description"]    = PackageTest::DESCRIPTION;
	match_field_map["LICENSE"]        = PackageTest::LICENSE;
	match_field_map["license"]        = PackageTest::LICENSE;
	match_field_map["CATEGORY"]       = PackageTest::CATEGORY;
	match_field_map["category"]       = PackageTest::CATEGORY;
	match_field_map["CATEGORY_NAME"]  = PackageTest::CATEGORY_NAME;
	match_field_map["CATEGORY-NAME"]  = PackageTest::CATEGORY_NAME;
	match_field_map["CATEGORY/NAME"]  = PackageTest::CATEGORY_NAME;
	match_field_map["category_name"]  = PackageTest::CATEGORY_NAME;
	match_field_map["category-name"]  = PackageTest::CATEGORY_NAME;
	match_field_map["category/name"]  = PackageTest::CATEGORY_NAME;
	match_field_map["HOMEPAGE"]       = PackageTest::HOMEPAGE;
	match_field_map["homepage"]       = PackageTest::HOMEPAGE;
	match_field_map["PROVIDE"]        = PackageTest::PROVIDE;
	match_field_map["PROVIDES"]       = PackageTest::PROVIDE;
	match_field_map["provide"]        = PackageTest::PROVIDE;
	match_field_map["provides"]       = PackageTest::PROVIDE;
	match_field_map["VIRTUAL"]        = PackageTest::PROVIDE|PackageTest::CATEGORY_NAME;
	match_field_map["virtual"]        = PackageTest::PROVIDE|PackageTest::CATEGORY_NAME;
	match_field_map["IUSE"]           = PackageTest::IUSE;
	match_field_map["USE"]            = PackageTest::IUSE;
	match_field_map["iuse"]           = PackageTest::IUSE;
	match_field_map["use"]            = PackageTest::IUSE;
	match_field_map["WITH_USE"]              = PackageTest::USE_ENABLED;
	match_field_map["WITH-USE"]              = PackageTest::USE_ENABLED;
	match_field_map["INSTALLED_WITH_USE"]    = PackageTest::USE_ENABLED;
	match_field_map["INSTALLED-WITH-USE"]    = PackageTest::USE_ENABLED;
	match_field_map["with_use"]              = PackageTest::USE_ENABLED;
	match_field_map["with-use"]              = PackageTest::USE_ENABLED;
	match_field_map["installed_with_use"]    = PackageTest::USE_ENABLED;
	match_field_map["installed-with-use"]    = PackageTest::USE_ENABLED;
	match_field_map["WITHOUT_USE"]           = PackageTest::USE_DISABLED;
	match_field_map["WITHOUT-USE"]           = PackageTest::USE_DISABLED;
	match_field_map["INSTALLED_WITHOUT_USE"] = PackageTest::USE_DISABLED;
	match_field_map["INSTALLED-WITHOUT-USE"] = PackageTest::USE_DISABLED;
	match_field_map["without_use"]           = PackageTest::USE_DISABLED;
	match_field_map["without-use"]           = PackageTest::USE_DISABLED;
	match_field_map["installed_without_use"] = PackageTest::USE_DISABLED;
	match_field_map["installed-without-use"] = PackageTest::USE_DISABLED;
	match_field_map["SET"]            = PackageTest::SET;
	match_field_map["set"]            = PackageTest::SET;
	match_field_map["SLOT"]           = PackageTest::SLOT;
	match_field_map["slot"]           = PackageTest::SLOT;
	match_field_map["INSTALLED_SLOT"] = PackageTest::INSTALLED_SLOT;
	match_field_map["INSTALLED-SLOT"] = PackageTest::INSTALLED_SLOT;
	match_field_map["INSTALLEDSLOT"]  = PackageTest::INSTALLED_SLOT;
	match_field_map["installed_slot"] = PackageTest::INSTALLED_SLOT;
	match_field_map["installed-slot"] = PackageTest::INSTALLED_SLOT;
	match_field_map["installedslot"]  = PackageTest::INSTALLED_SLOT;
	return match_field_map;
}

inline static const map<string,PackageTest::MatchAlgorithm>&
static_match_algorithm_map()
{
	static map<string,PackageTest::MatchAlgorithm> match_algorithm_map;
	match_algorithm_map["REGEX"]      = PackageTest::ALGO_REGEX;
	match_algorithm_map["REGEXP"]     = PackageTest::ALGO_REGEX;
	match_algorithm_map["regex"]      = PackageTest::ALGO_REGEX;
	match_algorithm_map["regexp"]     = PackageTest::ALGO_REGEX;
	match_algorithm_map["EXACT"]      = PackageTest::ALGO_EXACT;
	match_algorithm_map["exact"]      = PackageTest::ALGO_EXACT;
	match_algorithm_map["BEGIN"]      = PackageTest::ALGO_BEGIN;
	match_algorithm_map["begin"]      = PackageTest::ALGO_BEGIN;
	match_algorithm_map["END"]        = PackageTest::ALGO_END;
	match_algorithm_map["end"]        = PackageTest::ALGO_END;
	match_algorithm_map["SUBSTRING"]  = PackageTest::ALGO_SUBSTRING;
	match_algorithm_map["substring"]  = PackageTest::ALGO_SUBSTRING;
	match_algorithm_map["PATTERN"]    = PackageTest::ALGO_PATTERN;
	match_algorithm_map["pattern"]    = PackageTest::ALGO_PATTERN;
	match_algorithm_map["FUZZY"]      = PackageTest::ALGO_FUZZY;
	match_algorithm_map["fuzzy"]      = PackageTest::ALGO_FUZZY;
	return match_algorithm_map;
}

PackageTest::MatchField
PackageTest::name2field(const string &p) throw(ExBasic)
{
	const static map<string,MatchField>& match_field_map(static_match_field_map());
	map<string,MatchField>::const_iterator it(match_field_map.find(p));
	if(unlikely(it == match_field_map.end())) {
		throw ExBasic(_("cannot find match field %r")) % p;
		return NAME;
	}
	return it->second;
}

PackageTest::MatchAlgorithm
PackageTest::name2algorithm(const string &p) throw(ExBasic)
{
	const static map<string,MatchAlgorithm>& match_algorithm_map(static_match_algorithm_map());
	map<string,MatchAlgorithm>::const_iterator it(match_algorithm_map.find(p));
	if(unlikely(it == match_algorithm_map.end())) {
		throw ExBasic(_("cannot find match algorithm %r")) % p;
		return ALGO_REGEX;
	}
	return it->second;
}

/// It is more convenient to make this a macro than a template,
/// because otherwise we would have to pass initialization functions

#define MatcherClassDefinition(n,t,f,d) \
class n \
{ \
	private: \
	vector<Regex*> m; \
	vector<t> v; \
	t default_value; \
 \
	public: \
	n(const string &s) throw(ExBasic) \
	{ \
		vector<string> pairs; \
		split_string(pairs, s, true); \
		for(vector<string>::iterator it(pairs.begin()); \
			likely(it != pairs.end()); ++it) { \
			string *s_ptr(&(*it)); \
			++it; \
			if(it == pairs.end()) { \
				default_value = f(*s_ptr); \
				return; \
			} \
			m.push_back(new Regex(s_ptr->c_str())); \
			v.push_back(f(*it)); \
		} \
		default_value = d; \
	} \
 \
	~n() \
	{ \
		for(vector<Regex*>::iterator it(m.begin()); likely(it != m.end()); ++it) \
			delete *it; \
	} \
 \
	t parse(const char *p) \
	{ \
		for(vector<Regex*>::size_type i(0); likely(i != m.size()); ++i) { \
			if(m[i]->match(p)) \
				return v[i]; \
		} \
		return default_value; \
	} \
}

MatcherClassDefinition(MatcherField, PackageTest::MatchField, PackageTest::name2field, PackageTest::NAME);
MatcherClassDefinition(MatcherAlgorithm, PackageTest::MatchAlgorithm, PackageTest::name2algorithm, PackageTest::ALGO_REGEX);

PackageTest::MatchField
PackageTest::get_matchfield(const char *p) throw(ExBasic)
{
	static auto_ptr<MatcherField> m;
	if(!m.get()) {
		EixRc &rc(get_eixrc(NULL));
		m = auto_ptr<MatcherField>(new MatcherField(rc["DEFAULT_MATCH_FIELD"]));
	}
	return m->parse(p);
}

PackageTest::MatchAlgorithm
PackageTest::get_matchalgorithm(const char *p) throw(ExBasic)
{
	static auto_ptr<MatcherAlgorithm> m;
	if(!m.get()) {
		EixRc &rc(get_eixrc(NULL));
		m = auto_ptr<MatcherAlgorithm>(new MatcherAlgorithm(rc["DEFAULT_MATCH_ALGORITHM"]));
	}
	return m->parse(p);
}

void
PackageTest::setAlgorithm(MatchAlgorithm a)
{
	switch(a) {
		case ALGO_REGEX:
			setAlgorithm(new RegexAlgorithm());
			break;
		case ALGO_EXACT:
			setAlgorithm(new ExactAlgorithm());
			break;
		case ALGO_BEGIN:
			setAlgorithm(new BeginAlgorithm());
			break;
		case ALGO_END:
			setAlgorithm(new EndAlgorithm());
			break;
		case ALGO_SUBSTRING:
			setAlgorithm(new SubstringAlgorithm());
			break;
		case ALGO_PATTERN:
			setAlgorithm(new PatternAlgorithm());
			break;
		case ALGO_FUZZY:
		default:
			setAlgorithm(new FuzzyAlgorithm(get_eixrc(NULL).getInteger("LEVENSHTEIN_DISTANCE")));
			break;
	}
}

void
PackageTest::setPattern(const char *p)
{
	if(!algorithm.get())
		setAlgorithm(get_matchalgorithm(p));

	if(field == NONE)
		field = get_matchfield(p);

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
			likely(it != pkg->end()); ++it) {
			if((*algorithm)(it->slotname.c_str(), pkg))
				return true;
		}
	}

	if(field & IUSE) {
		const set<IUse> &s(pkg->iuse.asSet());
		for(set<IUse>::const_iterator it = s.begin();
			it != s.end(); ++it) {
			if((*algorithm)(it->name().c_str(), NULL))
				return true;
		}
	}

	if(field & SET) {
		set<string> setnames;
		portagesettings->get_setnames(setnames, pkg);
		for(set<string>::const_iterator it(setnames.begin());
			likely(it != setnames.end()); ++it) {
			if((*algorithm)(it->c_str(), NULL))
				return true;
			if((*algorithm)((string("@") + *it).c_str(), NULL))
				return true;
		}
	}

	if(! (field & (USE_ENABLED | USE_DISABLED | INSTALLED_SLOT)))
		return false;

	vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*pkg));
	if(installed_versions == NULL)
		return false;

	if(field & INSTALLED_SLOT) {
		for(vector<InstVersion>::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			if(!vardbpkg->readSlot(*pkg, *it))
				continue;
			if((*algorithm)(it->slotname.c_str(), pkg))
				return true;
		}
	}

	if(field & (USE_ENABLED | USE_DISABLED)) {
		for(vector<InstVersion>::iterator it(installed_versions->begin());
			it != installed_versions->end(); ++it) {
			if(!vardbpkg->readUse(*pkg, *it))
				continue;
			if(field & USE_ENABLED) {
				for(set<string>::iterator uit((it->usedUse).begin());
					likely(uit != (it->usedUse).end()); ++uit) {
					if((*algorithm)(uit->c_str(), NULL))
						return true;
				}
			}
			if(field & USE_DISABLED) {
				for(vector<string>::iterator uit((it->inst_iuse).begin());
					likely(uit != (it->inst_iuse).end()); ++uit) {
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
	bool test_unrestricted(!(r & t.spc));
	bool test_uninstalled(!(r & t.ins));
	if(r & t.all)// test all, all-installed or all-uninstalled
	{
		bool rvalue(false);
		BasicVersion *prev_ver(NULL);
		for(Package::const_reverse_iterator pi(p.rbegin());
			likely(pi != p.rend());
			prev_ver = *pi, ++pi)
		{
			// "all" should also mean at least once:
			if((pi->get_redundant()) & r)
				rvalue = true;
			else// and no failure:
			{
				if(test_unrestricted)
					return false;
				short is_installed(vardbpkg->isInstalledVersion(p, *pi, *header, portdir));
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
		for(Package::const_iterator pi(p.begin());
			likely(pi != p.end()); ++pi) {
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
	if(likely(what == PackageTest::STABLE_NONE))
		return true;
	for(Package::const_iterator it(p->begin()); likely(it != p->end()); ++it) {
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

bool
PackageTest::instabilitytest(const Package *p, TestStability what) const
{
	if(likely(what == STABLE_NONE))
		return true;
	for(Package::const_iterator it(p->begin()); likely(it != p->end()); ++it) {
		TestStability have(STABLE_NONE);
		if(it->maskflags.isHardMasked())
			have |= (STABLE_FULL | STABLE_NONMASKED);
		if(!it->keyflags.isStable())
			have |= STABLE_FULL;
		if(it->keyflags.isUnstable())
			have |= (STABLE_FULL | STABLE_TESTING);
		if((what & have) != what)
			continue;
		if(vardbpkg->isInstalledVersion(*p, *it, *header, portdir))
			return true;
	}
	return false;
}

inline void
get_p(Package *&p, PackageReader *pkg)
{
	if(unlikely(p == NULL)) {
		p = pkg->get();
	}
}

bool
PackageTest::match(PackageReader *pkg) const
{
	Package *p(NULL);

	pkg->read(need);

	/**
	   Test the local options.
	   Each test must start with get_p(p, pkg) to get p; remember to modify
	   "need" in CalculateNeeds() to ensure that you will have all
	   required data in the (possibly only partly filled) package "p".
	   If a test fails, "return false";
	   if a test succeeds, pass to the next test,
	   i.e. within the same Matchatom, we always have "-a" concatenation.

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

	if(unlikely(algorithm.get() != NULL)) {
		get_p(p, pkg);
		if(!stringMatch(p))
			return false;
	}

	if(unlikely(slotted)) { // -1 or -2
		get_p(p, pkg);
		if(! (p->have_nontrivial_slots()))
			return false;
		if(multi_slot)
			if( (p->slotlist()).size() <= 1 )
				return false;
	}

	if(unlikely(overlay)) { // -O
		get_p(p, pkg);
		if(!(p->largest_overlay))
			return false;
	}

	if(unlikely(overlay_list != NULL)) { // --in-overlay
		get_p(p, pkg);
		bool have = false;
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(unlikely(overlay_list->find(it->overlay_key) == overlay_list->end()))
				continue;
			have = true;
			break;
		}
		if(!have)
			return false;
	}

	if(unlikely(overlay_only_list != NULL)) { // --only-in-overlay
		get_p(p, pkg);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(likely(overlay_only_list->find(it->overlay_key) == overlay_only_list->end()))
				return false;
		}
	}

	if(unlikely(installed)) { // -i or -I
		get_p(p, pkg);
		vector<BasicVersion>::size_type s(vardbpkg->numInstalled(*p));
		if(s == 0)
			return false;
		if(s == 1)
			if(multi_installed)
				return false;
	}

	if(unlikely(in_overlay_inst_list != NULL)) {
		// --installed-in-[some-]overlay
		get_p(p, pkg);
		bool have(false);
		bool get_installed(true);
		vector<InstVersion> *installed_versions(NULL);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(in_overlay_inst_list->find(it->overlay_key) == in_overlay_inst_list->end())
				continue;
			if(get_installed) {
				get_installed = false;
				installed_versions = vardbpkg->getInstalledVector(*p);
			}
			if(installed_versions == NULL)
				continue;
			if(VarDbPkg::isInVec(installed_versions, *it)) {
				have = true;
				break;
			}
		}
		if(!have)
			return false;
	}

	if(unlikely((from_overlay_inst_list != NULL) ||
	   (from_foreign_overlay_inst_list != NULL))) {
	   // -J or --installed-from-overlay
		get_p(p, pkg);
		bool have(false);
		vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*p));
		if(installed_versions == NULL)
			return false;
		for(vector<InstVersion>::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			if(vardbpkg->readOverlay(*p, *it, *header, portdir)) {
				if(from_overlay_inst_list == NULL)
					continue;
				if(from_overlay_inst_list->find(it->overlay_key) == from_overlay_inst_list->end())
					continue;
				have = true;
				break;
			}
			if((it->overlay_keytext).empty())
				continue;
			if(from_foreign_overlay_inst_list == NULL)
				continue;
			for(vector<string>::iterator foreign(from_foreign_overlay_inst_list->begin());
				foreign != from_foreign_overlay_inst_list->end(); ++foreign) {
				if(unlikely(foreign->empty() ||
					same_filenames(foreign->c_str(), (it->overlay_keytext).c_str(), true, false) ||
					same_filenames(foreign->c_str(), (it->overlay_keytext).c_str(), true, true))) {
					have = true;
					break;
				}
			}
			if(have)
				break;
		}
		if(!have)
			return false;
	}

	if(unlikely(dup_packages)) { // -d
		get_p(p, pkg);
		if(dup_packages_overlay)
		{
			if(!(p->at_least_two_overlays()))
				return false;
		}
		else if(p->have_same_overlay_key())
			return false;
	}

	if(unlikely(dup_versions)) { // -D
		get_p(p, pkg);
		Package::Duplicates testfor((dup_versions_overlay) ?
				Package::DUP_OVERLAYS : Package::DUP_SOME);
		if(((p->have_duplicate_versions) & testfor) != testfor)
			return false;
	}

	if(unlikely((restrictions != ExtendedVersion::RESTRICT_NONE) ||
		(properties != ExtendedVersion::PROPERTIES_NONE))) {
		get_p(p, pkg);
		bool found(false);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(unlikely((((it->restrictFlags) & restrictions) == restrictions) &&
				(((it->propertiesFlags) & properties) == properties))) {
				found = true;
				break;
			}
		}
		if(!found) {
			vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*p));
			if(installed_versions == NULL)
				return false;
			for(vector<InstVersion>::iterator it(installed_versions->begin());
				likely(it != installed_versions->end()); ++it) {
				vardbpkg->readRestricted(*p, *it, *header, portdir);
				if(unlikely((((it->restrictFlags) & restrictions) == restrictions) &&
					(((it->propertiesFlags) & properties) == properties))) {
					found = true;
					break;
				}
			}
			if(!found)
				return false;
		}
	}

	if(binary) { // --binary
		get_p(p, pkg);
		bool found(false);
		for(Package::iterator it(p->begin()); it != p->end(); ++it) {
			if(unlikely(it->have_bin_pkg(portagesettings, p))) {
				found = true;
				break;
			}
		}
		if(!found) {
			vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*p));
			if(installed_versions == NULL)
				return false;
			for(vector<InstVersion>::iterator it(installed_versions->begin());
				likely(it != installed_versions->end()); ++it) {
				if(unlikely(it->have_bin_pkg(portagesettings, p))) {
					found = true;
					break;
				}
			}
			if(!found)
				return false;
		}
	}

	while(unlikely(obsolete)) { // -T; loop, because we break in case of success
		// Can some test succeed at all?
		if((test_installed == INS_NONE) &&
			(redundant_flags == Keywords::RED_NOTHING))
			return false;

		get_p(p, pkg);

		Keywords::Redundant r(redundant_flags & Keywords::RED_ALL_MASKSTUFF);
		if(r != Keywords::RED_NOTHING) {
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
		if(r != Keywords::RED_NOTHING) {
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
		if(r != Keywords::RED_NOTHING) {
			r &= nowarn_use(*p);
			if(r && portagesettings->user_config->CheckUse(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_USE))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_USE))
					break;
			}
		}
		r = redundant_flags & Keywords::RED_ALL_ENV;
		if(r != Keywords::RED_NOTHING) {
			r &= nowarn_env(*p);
			if(r && portagesettings->user_config->CheckEnv(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_ENV))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_ENV))
					break;
			}
		}
		r = redundant_flags & Keywords::RED_ALL_CFLAGS;
		if(r != Keywords::RED_NOTHING) {
			r &= nowarn_cflags(*p);
			if(r && portagesettings->user_config->CheckCflags(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_CFLAGS))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_CFLAGS))
					break;
			}
		}
		if(test_installed == INS_NONE)
			return false;
		TestInstalled t(test_installed & nowarn_installed(*p));
		if(t == INS_NONE)
			return false;
		vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*p));
		if(installed_versions == NULL)
			return false;
		if(t & INS_MASKED) {
			portagesettings->user_config->setMasks(p);
			portagesettings->user_config->setKeyflags(p);
		}
		vector<InstVersion>::iterator current(installed_versions->begin());
		for( ; likely(current != installed_versions->end()); ++current)
		{
			bool not_all_found(true);
			for(Package::iterator version_it(p->begin());
				likely(version_it != p->end()); ++version_it) {
				Version *version(*version_it);
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
		return false;
	}

	if(unlikely(upgrade)) { // -u
		get_p(p, pkg);
		if(upgrade_local_mode == LOCALMODE_DEFAULT)
			StabilityDefault(p);
		else if(upgrade_local_mode == LOCALMODE_LOCAL)
			StabilityLocal(p);
		else if(upgrade_local_mode == LOCALMODE_NONLOCAL)
			StabilityNonlocal(p);
		if(! (p->recommend(vardbpkg, portagesettings, true, true)))
			return false;
	}

	if(unlikely(test_stability_default != STABLE_NONE))
	{ // --stable, --testing, --non-masked, --system
		get_p(p, pkg);
		StabilityDefault(p);
		if(!stabilitytest(p, test_stability_default))
			return false;
	}

	if(unlikely(test_stability_local != STABLE_NONE))
	{ // --stable+, --testing+, --non-masked+, --system+
		get_p(p, pkg);
		StabilityLocal(p);
		if(!stabilitytest(p, test_stability_local))
			return false;
	}

	if(unlikely(test_stability_nonlocal != STABLE_NONE))
	{ // --stable-, --testing-, --non-masked-, --system-
		get_p(p, pkg);
		StabilityNonlocal(p);
		if(!stabilitytest(p, test_stability_nonlocal))
			return false;
	}

	if(unlikely(test_instability != STABLE_NONE))
	{ // --installed-unstable --installed-testing --installed-masked
		get_p(p, pkg);
		StabilityNonlocal(p);
		if(!instabilitytest(p, test_instability))
			return false;
	}

	if(unlikely(world || worldset)) {
		// --world, --world-all, --world-set
		// --selected, --selected-all, --selected-set
		get_p(p, pkg);
		StabilityDefault(p);
		if(world && !p->is_world_package()) {
			if(world_only_file || !p->is_world_sets_package()) {
				if(world_only_selected || !p->is_system_package()) {
					return false;
				}
			}
		}
		if(worldset && !p->is_world_sets_package()) {
			if(worldset_only_selected || !p->is_system_package()) {
				return false;
			}
		}
	}

	// all tests succeeded:
	return true;
}

static void
getlines(const char *varname, vector<string> &lines)
{
	EixRc &rc(get_eixrc(NULL));
	lines.clear();
	vector<string> name;
	split_string(name, rc[varname], true);
	for(vector<string>::const_iterator it(name.begin()); it != name.end(); ++it)
		pushback_lines(it->c_str(), &lines, false, true);
}

Keywords::Redundant
PackageTest::nowarn_keywords(const Package &p)
{
	static bool know_file(false);
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(unlikely(!know_file)) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("KEYWORDS_NOWARN", lines);
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_KEYWORDS;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
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
	static bool know_file(false);
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(unlikely(!know_file)) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("MASK_NOWARN", lines);
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_MASKSTUFF;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
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
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			i = m.find(*et);
			Keywords::Redundant r;
			if(i == m.end())
				r = Keywords::RED_ALL_MASKSTUFF;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
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
	static bool know_file(false);
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(unlikely(!know_file)) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("USE_NOWARN", lines);
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_USE;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
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
PackageTest::nowarn_env(const Package &p)
{
	static bool know_file(false);
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(unlikely(!know_file)) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("ENV_NOWARN", lines);
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_ENV;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
				if(strcasecmp(et->c_str(), "in_env") == 0)
					r &= ~Keywords::RED_IN_ENV;
				else if(strcasecmp(et->c_str(), "double_env") == 0)
					r &= ~Keywords::RED_DOUBLE_ENV;
			}
			m[s[0]] = r;
		}
	}
	i = m.find(p.category + "/" + p.name);
	if(i == m.end())
		return Keywords::RED_ALL_ENV;
	return i->second;
}

Keywords::Redundant
PackageTest::nowarn_cflags(const Package &p)
{
	static bool know_file(false);
	static map<string, Keywords::Redundant> m;
	map<string, Keywords::Redundant>::const_iterator i;
	if(unlikely(!know_file)) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("CFLAGS_NOWARN", lines);
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = Keywords::RED_ALL_CFLAGS;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
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
	static bool know_file(false);
	static map<string, TestInstalled> m;
	map<string, TestInstalled>::const_iterator i;
	if(unlikely(!know_file)) {
		know_file=true; m.clear();
		vector<string> lines;
		getlines("INSTALLED_NOWARN", lines);
		for(vector<string>::const_iterator it(lines.begin());
			likely(it != lines.end()); ++it) {
			vector<string> s;
			split_string(s, *it);
			if(s.empty())
				continue;
			vector<string>::const_iterator et(s.begin());
			Keywords::Redundant r;
			i = m.find(*et);
			if(i == m.end())
				r = INS_SOME;
			else
				r = i->second;
			for(++et; likely(et != s.end()); ++et) {
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
