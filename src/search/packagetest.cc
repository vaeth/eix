// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "database/package_reader.h"
#include "eixTk/assert.h"
#include "eixTk/eixint.h"
#include "eixTk/filenames.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/regexp.h"
#include "eixTk/stringutils.h"
#include "eixTk/utils.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "output/formatstring.h"
#include "portage/basicversion.h"
#include "portage/conf/cascadingprofile.h"
#include "portage/conf/portagesettings.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/package.h"
#include "portage/vardbpkg.h"
#include "search/algorithms.h"
#include "search/nowarn.h"
#include "search/packagetest.h"

class DBHeader;
class SetStability;

using std::map;
using std::set;
using std::string;
using std::vector;

using std::cerr;
using std::endl;

const PackageTest::MatchField
		PackageTest::NONE,
		PackageTest::NAME,
		PackageTest::DESCRIPTION,
		PackageTest::LICENSE,
		PackageTest::CATEGORY,
		PackageTest::CATEGORY_NAME,
		PackageTest::HOMEPAGE,
		PackageTest::IUSE,
		PackageTest::USE_ENABLED,
		PackageTest::USE_DISABLED,
		PackageTest::SLOT,
		PackageTest::FULLSLOT,
		PackageTest::INST_SLOT,
		PackageTest::INST_FULLSLOT,
		PackageTest::SET,
		PackageTest::DEPEND,
		PackageTest::RDEPEND,
		PackageTest::PDEPEND,
		PackageTest::DEPS,
		PackageTest::ANY;

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

NowarnMaskList *PackageTest::nowarn_list = NULLPTR;

static void init_match_field_map();
static void init_match_algorithm_map();
static bool stabilitytest(const Package *p, PackageTest::TestStability what) ATTRIBUTE_NONNULL_ ATTRIBUTE_PURE;
inline static void get_p(Package *&p, PackageReader *pkg) ATTRIBUTE_NONNULL_;

PackageTest::PackageTest(VarDbPkg *vdb, PortageSettings *p, const PrintFormat *f, const SetStability *set_stability, const DBHeader *dbheader)
{
	vardbpkg = vdb;
	portagesettings = p;
	print_format = f;
	stability= set_stability;
	header   = dbheader;
	overlay_list = overlay_only_list = in_overlay_inst_list = NULLPTR;
	algorithm = NULLPTR;
	from_overlay_inst_list = NULLPTR;
	from_foreign_overlay_inst_list = NULLPTR;

	field    = NONE;
	need     = PackageReader::NONE;
	overlay = obsolete = upgrade = binary =
		installed = multi_installed =
		slotted = multi_slot =
		world = world_only_selected = world_only_file =
		worldset = worldset_only_selected =
		dup_versions = dup_packages =
		have_virtual = have_nonvirtual =
		know_pattern = false;
	restrictions = ExtendedVersion::RESTRICT_NONE;
	properties = ExtendedVersion::PROPERTIES_NONE;
	test_installed = INS_NONE;
	test_instability =
	test_stability_default = test_stability_local = test_stability_nonlocal = STABLE_NONE;
}

PackageTest::~PackageTest() {
	setAlgorithm(NULLPTR);
	delete overlay_list;
	delete overlay_only_list;
	delete in_overlay_inst_list;
	delete from_overlay_inst_list;
	delete from_foreign_overlay_inst_list;
}

void
PackageTest::calculateNeeds() {
	need = PackageReader::NONE;
	if(field & (SLOT | FULLSLOT | SET))
		setNeeds(PackageReader::VERSIONS);
	if(field & HOMEPAGE)
		setNeeds(PackageReader::HOMEPAGE);
	if(field & LICENSE)
		setNeeds(PackageReader::LICENSE);
	if(field & DESCRIPTION)
		setNeeds(PackageReader::DESCRIPTION);
	if(field & CATEGORY)
		setNeeds(PackageReader::NONE);
	if(field & (NAME | CATEGORY_NAME))
		setNeeds(PackageReader::NAME);
	if(field & (USE_ENABLED | USE_DISABLED | INST_SLOT | INST_FULLSLOT))
		setNeeds(PackageReader::NAME);
	if(installed)
		setNeeds(PackageReader::NAME);
	if(!Depend::use_depend) {
		field &= ~DEPS;
	}
	if((field & (IUSE | DEPS)) ||
		dup_packages || dup_versions || slotted ||
		upgrade || overlay || obsolete || binary ||
		world || worldset ||
		have_virtual || have_nonvirtual ||
		(from_overlay_inst_list != NULLPTR) ||
		(from_foreign_overlay_inst_list != NULLPTR) ||
		(overlay_list != NULLPTR) || (overlay_only_list != NULLPTR) ||
		(in_overlay_inst_list != NULLPTR) ||
		(restrictions != ExtendedVersion::RESTRICT_NONE) ||
		(properties != ExtendedVersion::PROPERTIES_NONE) ||
		(test_instability != STABLE_NONE) ||
		(test_stability_default != STABLE_NONE) ||
		(test_stability_local != STABLE_NONE) ||
		(test_stability_nonlocal != STABLE_NONE))
		setNeeds(PackageReader::VERSIONS);
}

static map<string, PackageTest::MatchField> *static_match_field_map = NULLPTR;

static void
init_match_field_map()
{
	eix_assert_static(static_match_field_map == NULLPTR);
	static_match_field_map = new map<string, PackageTest::MatchField>;
	map<string, PackageTest::MatchField> &match_field_map(*static_match_field_map);
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
	match_field_map["FULLSLOT"]       = PackageTest::FULLSLOT;
	match_field_map["fullslot"]       = PackageTest::FULLSLOT;
	match_field_map["INSTALLED_SLOT"] = PackageTest::INST_SLOT;
	match_field_map["INSTALLED-SLOT"] = PackageTest::INST_SLOT;
	match_field_map["INSTALLEDSLOT"]  = PackageTest::INST_SLOT;
	match_field_map["installed_slot"] = PackageTest::INST_SLOT;
	match_field_map["installed-slot"] = PackageTest::INST_SLOT;
	match_field_map["installedslot"]  = PackageTest::INST_SLOT;
	match_field_map["INSTALLED_FULLSLOT"] = PackageTest::INST_FULLSLOT;
	match_field_map["INSTALLED-FULLSLOT"] = PackageTest::INST_FULLSLOT;
	match_field_map["INSTALLEDFULLSLOT"]  = PackageTest::INST_FULLSLOT;
	match_field_map["installed_fullslot"] = PackageTest::INST_FULLSLOT;
	match_field_map["installed-fullslot"] = PackageTest::INST_FULLSLOT;
	match_field_map["installedfullslot"]  = PackageTest::INST_FULLSLOT;
	match_field_map["DEP"]            = PackageTest::DEPS;
	match_field_map["DEPS"]           = PackageTest::DEPS;
	match_field_map["DEPENDENCIES"]   = PackageTest::DEPS;
	match_field_map["dep"]            = PackageTest::DEPS;
	match_field_map["deps"]           = PackageTest::DEPS;
	match_field_map["dependencies"]   = PackageTest::DEPS;
	match_field_map["DEPEND"]         = PackageTest::DEPEND;
	match_field_map["depend"]         = PackageTest::DEPEND;
	match_field_map["RDEPEND"]        = PackageTest::RDEPEND;
	match_field_map["rdepend"]        = PackageTest::RDEPEND;
	match_field_map["PDEPEND"]        = PackageTest::PDEPEND;
	match_field_map["pdepend"]        = PackageTest::PDEPEND;
}

static map<string, PackageTest::MatchAlgorithm> *static_match_algorithm_map = NULLPTR;

static void
init_match_algorithm_map()
{
	eix_assert_static(static_match_algorithm_map == NULLPTR);
	static_match_algorithm_map = new map<string, PackageTest::MatchAlgorithm>;
	map<string, PackageTest::MatchAlgorithm> &match_algorithm_map(*static_match_algorithm_map);
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
}

PackageTest::MatchField
PackageTest::name2field(const string &p)
{
	eix_assert_static(static_match_field_map != NULLPTR);
	map<string, MatchField>::const_iterator it(static_match_field_map->find(p));
	if(unlikely(it == static_match_field_map->end())) {
		cerr << eix::format(_("cannot find match field %r")) % p << endl;
		return NAME;
	}
	return it->second;
}

PackageTest::MatchAlgorithm
PackageTest::name2algorithm(const string &p)
{
	eix_assert_static(static_match_algorithm_map != NULLPTR);
	map<string, MatchAlgorithm>::const_iterator it(static_match_algorithm_map->find(p));
	if(unlikely(it == static_match_algorithm_map->end())) {
		cerr << eix::format(_("cannot find match algorithm %r")) % p << endl;
		return ALGO_REGEX;
	}
	return it->second;
}

/// It is more convenient to make this a macro than a template,
/// because otherwise we would have to pass initialization functions

#define MatcherClassDefinition(n, t, f, d) \
class n \
{ \
	private: \
	vector<Regex*> m; \
	vector<t> v; \
	t default_value; \
\
	public: \
	explicit n(const string &s) \
	{ \
		vector<string> pairs; \
		split_string(&pairs, s, true); \
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
		{ \
			(*it)->free(); \
			delete *it; \
		} \
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
PackageTest::get_matchfield(const char *p)
{
	static MatcherField *m = NULLPTR;
	if(m == NULLPTR) {
		EixRc &rc(get_eixrc());
		m = new MatcherField(rc["DEFAULT_MATCH_FIELD"]);
	}
	return m->parse(p);
}

PackageTest::MatchAlgorithm
PackageTest::get_matchalgorithm(const char *p)
{
	static MatcherAlgorithm *m = NULLPTR;
	if(m == NULLPTR) {
		EixRc &rc(get_eixrc());
		m = new MatcherAlgorithm(rc["DEFAULT_MATCH_ALGORITHM"]);
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
			setAlgorithm(new FuzzyAlgorithm(get_eixrc().getInteger("LEVENSHTEIN_DISTANCE")));
			break;
	}
}

void
PackageTest::setPattern(const char *p)
{
	if(!know_pattern) {
		if(algorithm == NULLPTR) {
			setAlgorithm(get_matchalgorithm(p));
		}

		if(field == NONE) {
			field = get_matchfield(p);
		}
		know_pattern = true;
	}
	algorithm->setString(p);
}

void
PackageTest::finalize()
{
	if(!know_pattern) {
		setPattern("");
	}
	calculateNeeds();
}

/** Return true if pkg matches test. */
bool
PackageTest::stringMatch(Package *pkg) const
{
	if(((field & NAME) != NONE) && (*algorithm)(pkg->name.c_str(), pkg))
		return true;

	if(((field & DESCRIPTION) != NONE)  && (*algorithm)(pkg->desc.c_str(), pkg))
		return true;

	if(((field & LICENSE) != NONE) && (*algorithm)(pkg->licenses.c_str(), pkg))
		return true;

	if(((field & CATEGORY) != NONE) && (*algorithm)(pkg->category.c_str(), pkg))
		return true;

	if(((field & CATEGORY_NAME) != NONE) && (*algorithm)((pkg->category + "/" + pkg->name).c_str(), pkg))
		return true;

	if(((field & HOMEPAGE) != NONE) && (*algorithm)(pkg->homepage.c_str(), pkg))
		return true;

	if((field & SLOT) != NONE) {
		for(Package::iterator it(pkg->begin());
			likely(it != pkg->end()); ++it) {
			if((*algorithm)(it->get_longslot().c_str(), pkg))
				return true;
		}
	}

	if((field & FULLSLOT) != NONE) {
		for(Package::iterator it(pkg->begin());
			likely(it != pkg->end()); ++it) {
			if((*algorithm)(it->get_longfullslot().c_str(), pkg))
				return true;
		}
	}

	if((field & IUSE) != NONE) {
		const set<IUse> &s(pkg->iuse.asSet());
		for(set<IUse>::const_iterator it(s.begin());
			it != s.end(); ++it) {
			if((*algorithm)(it->name().c_str(), NULLPTR))
				return true;
		}
	}

	if((field & DEPS) != NONE) {
		bool depend((field & DEPEND) != NONE);
		bool rdepend((field & RDEPEND) != NONE);
		bool pdepend((field & PDEPEND) != NONE);
		for(Package::iterator it(pkg->begin());
			likely(it != pkg->end()); ++it) {
			const Depend &dep(it->depend);
			if(depend) {
				if((*algorithm)(dep.get_depend().c_str(), pkg))
				return true;
			}
			if(rdepend) {
				if((*algorithm)(dep.get_rdepend().c_str(), pkg))
				return true;
			}
			if(pdepend) {
				if((*algorithm)(dep.get_pdepend().c_str(), pkg))
				return true;
			}
		}
	}

	if(field & SET) {
		set<string> setnames;
		portagesettings->get_setnames(&setnames, pkg);
		for(set<string>::const_iterator it(setnames.begin());
			likely(it != setnames.end()); ++it) {
			if((*algorithm)(it->c_str(), NULLPTR))
				return true;
			if((*algorithm)((string("@") + *it).c_str(), NULLPTR))
				return true;
		}
	}

	if((field & (USE_ENABLED | USE_DISABLED | INST_SLOT | INST_FULLSLOT)) == NONE)
		return false;

	vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*pkg));
	if(installed_versions == NULLPTR)
		return false;

	if((field & (INST_SLOT | INST_FULLSLOT)) != NONE) {
		for(vector<InstVersion>::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			if(!vardbpkg->readSlot(*pkg, &(*it)))
				continue;
			if((field & INST_SLOT) != NONE) {
				if((*algorithm)(it->get_longslot().c_str(), pkg)) {
					return true;
				}
			}
			if((field & INST_FULLSLOT) != NONE) {
				if((*algorithm)(it->get_longfullslot().c_str(), pkg)) {
					return true;
				}
			}
		}
	}

	if((field & (USE_ENABLED | USE_DISABLED)) != NONE) {
		for(vector<InstVersion>::iterator it(installed_versions->begin());
			it != installed_versions->end(); ++it) {
			if(!vardbpkg->readUse(*pkg, &(*it)))
				continue;
			if((field & USE_ENABLED) != NONE) {
				for(set<string>::iterator uit((it->usedUse).begin());
					likely(uit != (it->usedUse).end()); ++uit) {
					if((*algorithm)(uit->c_str(), NULLPTR))
						return true;
				}
			}
			if((field & USE_DISABLED) != NONE) {
				for(vector<string>::iterator uit((it->inst_iuse).begin());
					likely(uit != (it->inst_iuse).end()); ++uit) {
					if(!(*algorithm)(uit->c_str(), NULLPTR))
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
	if((t.only & r) != Keywords::RED_NOTHING) {
		if((vardbpkg->numInstalled(p) != 0) != ((t.oins & r) != 0))
			return false;
	}
	bool test_unrestricted((r & t.spc) == Keywords::RED_NOTHING);
	bool test_uninstalled((r & t.ins) == Keywords::RED_NOTHING);
	if(r & t.all) {
		// test all, all-installed or all-uninstalled
		bool rvalue(false);
		BasicVersion *prev_ver(NULLPTR);
		for(Package::const_reverse_iterator pi(p.rbegin());
			likely(pi != p.rend());
			prev_ver = *pi, ++pi) {
			// "all" should also mean at least once:
			if(((pi->get_redundant()) & r) != Keywords::RED_NOTHING) {
				rvalue = true;
			} else {
			// and no failure:
				if(test_unrestricted)
					return false;
				eix::SignedBool is_installed(vardbpkg->isInstalledVersion(p, *pi, *header));
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
	} else {
		// test some or some-installed
		for(Package::const_iterator pi(p.begin());
			likely(pi != p.end()); ++pi) {
			if((pi->get_redundant()) & r)
			{
				if(test_unrestricted)
					return true;
				// in contrast to the above loop, we now in doubt
				// do not distinguish overlays.
				if(test_uninstalled ==
					!(vardbpkg->isInstalledVersion(p, *pi, *header)))
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
			if(!it->maskflags.isSystem()) {
				continue;
			}
			if(!(what & ~PackageTest::STABLE_SYSTEM)) {
				return true;
			}
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
		if(vardbpkg->isInstalledVersion(*p, *it, *header))
			return true;
	}
	return false;
}

inline static void
get_p(Package *&p, PackageReader *pkg)
{
	if(unlikely(p == NULLPTR)) {
		p = pkg->get();
	}
}

bool
PackageTest::match(PackageReader *pkg) const
{
	Package *p(NULLPTR);

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

	if(unlikely(algorithm != NULLPTR)) {
		get_p(p, pkg);
		if(!stringMatch(p))
			return false;
	}

	if(unlikely(slotted)) {
		// -1 or -2
		get_p(p, pkg);
		if(!(p->have_nontrivial_slots()))
			return false;
		if(multi_slot)
			if( (p->slotlist()).size() <= 1 )
				return false;
	}

	if(unlikely(overlay)) {
		// -O
		get_p(p, pkg);
		if(!(p->largest_overlay))
			return false;
	}

	if(unlikely(overlay_list != NULLPTR)) {
		// --in-overlay
		get_p(p, pkg);
		bool have(false);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(unlikely(overlay_list->find(it->overlay_key) == overlay_list->end()))
				continue;
			have = true;
			break;
		}
		if(!have)
			return false;
	}

	if(unlikely(overlay_only_list != NULLPTR)) {
		// --only-in-overlay
		get_p(p, pkg);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(likely(overlay_only_list->find(it->overlay_key) == overlay_only_list->end()))
				return false;
		}
	}

	if(unlikely(have_virtual)) {
		// --virtual
		get_p(p, pkg);
		if(!print_format->have_virtual(p, false)) {
			return false;
		}
	}

	if(unlikely(have_nonvirtual)) {
		// --nonvirtual
		get_p(p, pkg);
		if(!print_format->have_virtual(p, true)) {
			return false;
		}
	}

	if(unlikely(installed)) {
		// -i or -I
		get_p(p, pkg);
		vector<BasicVersion>::size_type s(vardbpkg->numInstalled(*p));
		if(s == 0)
			return false;
		if(s == 1) {
			if(multi_installed)
				return false;
		}
	}

	if(unlikely(in_overlay_inst_list != NULLPTR)) {
		// --installed-in-[some-]overlay
		get_p(p, pkg);
		bool have(false);
		bool get_installed(true);
		vector<InstVersion> *installed_versions(NULLPTR);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(in_overlay_inst_list->find(it->overlay_key) == in_overlay_inst_list->end())
				continue;
			if(get_installed) {
				get_installed = false;
				installed_versions = vardbpkg->getInstalledVector(*p);
			}
			if(installed_versions == NULLPTR)
				continue;
			if(VarDbPkg::isInVec(installed_versions, *it)) {
				have = true;
				break;
			}
		}
		if(!have)
			return false;
	}

	if(unlikely((from_overlay_inst_list != NULLPTR) ||
	   (from_foreign_overlay_inst_list != NULLPTR))) {
		// -J or --installed-from-overlay
		get_p(p, pkg);
		bool have(false);
		vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*p));
		if(installed_versions == NULLPTR)
			return false;
		for(vector<InstVersion>::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			if(vardbpkg->readOverlay(*p, &(*it), *header)) {
				if(from_overlay_inst_list == NULLPTR)
					continue;
				if(from_overlay_inst_list->find(it->overlay_key) == from_overlay_inst_list->end())
					continue;
				have = true;
				break;
			}
		}
		if(!have)
			return false;
	}

	if(unlikely(dup_packages)) {
		// -d
		get_p(p, pkg);
		if(dup_packages_overlay) {
			if(!(p->at_least_two_overlays()))
				return false;
		} else if(p->have_same_overlay_key()) {
			return false;
		}
	}

	if(unlikely(dup_versions)) {
		// -D
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
			if(installed_versions == NULLPTR)
				return false;
			for(vector<InstVersion>::iterator it(installed_versions->begin());
				likely(it != installed_versions->end()); ++it) {
				vardbpkg->readRestricted(*p, &(*it), *header);
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

	if(binary) {
		// --binary
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
			if(installed_versions == NULLPTR)
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

	while(unlikely(obsolete)) {
		// -T; loop, because we break in case of success
		// Can some test succeed at all?
		if((test_installed == INS_NONE) &&
			(redundant_flags == Keywords::RED_NOTHING))
			return false;

		if(nowarn_list == NULLPTR) {
			get_nowarn_list();
		}
		get_p(p, pkg);
		Keywords::Redundant rflags(redundant_flags);
		TestInstalled test_ins(test_installed);
		nowarn_list->apply(p, &rflags, &test_ins, portagesettings);

		Keywords::Redundant r(rflags & Keywords::RED_ALL_MASKSTUFF);
		if(r != Keywords::RED_NOTHING) {
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
		r = rflags & Keywords::RED_ALL_KEYWORDS;
		if(r != Keywords::RED_NOTHING) {
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
			}
		}
		r = rflags & Keywords::RED_ALL_USE;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckUse(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_USE))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_USE))
					break;
			}
		}
		r = rflags & Keywords::RED_ALL_ENV;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckEnv(p, r))
			{
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_ENV))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_ENV))
					break;
			}
		}
		r = rflags & Keywords::RED_ALL_LICENSE;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckLicense(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_LICENSE))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_LICENSE))
					break;
			}
		}
		r = rflags & Keywords::RED_ALL_CFLAGS;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckCflags(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_CFLAGS))
					break;
				if(have_redundant(*p, r & Keywords::RED_IN_CFLAGS))
					break;
			}
		}
		if(test_ins == INS_NONE) {
			return false;
		}
		vector<InstVersion> *installed_versions(vardbpkg->getInstalledVector(*p));
		if(installed_versions == NULLPTR) {
			return false;
		}
		if(test_ins & INS_MASKED) {
			portagesettings->user_config->setMasks(p);
			portagesettings->user_config->setKeyflags(p);
		}
		vector<InstVersion>::iterator current(installed_versions->begin());
		for( ; likely(current != installed_versions->end()); ++current) {
			bool not_all_found(true);
			for(Package::iterator version_it(p->begin());
				likely(version_it != p->end()); ++version_it) {
				Version *version(*version_it);
				if(*version != *current)
					continue;
				if(test_ins & INS_MASKED) {
					if(version->maskflags.isHardMasked() || !version->keyflags.isStable())
						continue;
				}
				if(test_ins & INS_OVERLAY) {
					if(!vardbpkg->readOverlay(*p, &(*current), *header))
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

	if(unlikely(upgrade)) {
		// -u
		get_p(p, pkg);
		if(upgrade_local_mode == LOCALMODE_DEFAULT)
			StabilityDefault(p);
		else if(upgrade_local_mode == LOCALMODE_LOCAL)
			StabilityLocal(p);
		else if(upgrade_local_mode == LOCALMODE_NONLOCAL)
			StabilityNonlocal(p);
		if(!(p->recommend(vardbpkg, portagesettings, true, true)))
			return false;
	}

	if(unlikely(test_stability_default != STABLE_NONE)) {
		// --stable, --testing, --non-masked, --system
		get_p(p, pkg);
		StabilityDefault(p);
		if(!stabilitytest(p, test_stability_default))
			return false;
	}

	if(unlikely(test_stability_local != STABLE_NONE)) {
		// --stable+, --testing+, --non-masked+, --system+
		get_p(p, pkg);
		StabilityLocal(p);
		if(!stabilitytest(p, test_stability_local))
			return false;
	}

	if(unlikely(test_stability_nonlocal != STABLE_NONE)) {
		// --stable-, --testing-, --non-masked-, --system-
		get_p(p, pkg);
		StabilityNonlocal(p);
		if(!stabilitytest(p, test_stability_nonlocal))
			return false;
	}

	if(unlikely(test_instability != STABLE_NONE)) {
	// --installed-unstable --installed-testing --installed-masked
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

void
PackageTest::get_nowarn_list()
{
	NowarnPreList prelist;
	vector<string> name;
	EixRc &rc(get_eixrc());
	split_string(&name, rc["PACKAGE_NOWARN"], true);
	for(vector<string>::const_iterator it(name.begin()); it != name.end(); ++it) {
		vector<string> lines;
		pushback_lines(it->c_str(), &lines, false, true);
		prelist.handle_file(lines, *it, NULLPTR, true);
	}
	nowarn_list = new NowarnMaskList;
	prelist.initialize(nowarn_list);
}

void
PackageTest::init_static()
{
	NowarnMask::init_static();
	FuzzyAlgorithm::init_static();
	init_match_field_map();
	init_match_algorithm_map();
}
