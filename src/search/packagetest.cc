// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "search/packagetest.h"
#include <config.h>  // IWYU pragma: keep

#include <string>

#include "database/package_reader.h"
#include "eixTk/attribute.h"
#include "eixTk/eixint.h"
#include "eixTk/filenames.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
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
#include "portage/mask_list.h"
#include "portage/package.h"
#include "portage/vardbpkg.h"
#include "search/algorithms.h"
#include "search/nowarn.h"

using std::string;

class DBHeader;
class SetStability;

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
		PackageTest::SRC_URI,
		PackageTest::EAPI,
		PackageTest::INST_EAPI,
		PackageTest::SLOT,
		PackageTest::FULLSLOT,
		PackageTest::INST_SLOT,
		PackageTest::INST_FULLSLOT,
		PackageTest::SET,
		PackageTest::DEPENDA,
		PackageTest::RDEPENDA,
		PackageTest::PDEPENDA,
		PackageTest::BDEPENDA,
		PackageTest::DEPENDI,
		PackageTest::RDEPENDI,
		PackageTest::PDEPENDI,
		PackageTest::BDEPENDI,
		PackageTest::DEPEND,
		PackageTest::RDEPEND,
		PackageTest::PDEPEND,
		PackageTest::BDEPEND,
		PackageTest::DEPSA,
		PackageTest::DEPSI,
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
		PackageTest::STABLE_SYSTEM,
		PackageTest::STABLE_PROFILE,
		PackageTest::STABLE_SYSTEMPROFILE;

NowarnMaskList *PackageTest::nowarn_list = NULLPTR;

ATTRIBUTE_NONNULL_ ATTRIBUTE_PURE static bool stabilitytest(const Package *p, PackageTest::TestStability what);
ATTRIBUTE_NONNULL_ inline static void get_p(Package **p, PackageReader *pkg);

PackageTest::PackageTest(VarDbPkg *vdb, PortageSettings *p, const PrintFormat *f, const SetStability *set_stability, const DBHeader *dbheader, const ParseError *e) {
	vardbpkg = vdb;
	portagesettings = p;
	print_format = f;
	stability = set_stability;
	header = dbheader;
	parse_error = e;
	overlay_list = overlay_only_list = in_overlay_inst_list = NULLPTR;
	algorithm = NULLPTR;
	from_overlay_inst_list = NULLPTR;
	from_foreign_overlay_inst_list = NULLPTR;
	marked_list = NULLPTR;

	field = NONE;
	need = PackageReader::NONE;
	overlay = obsolete = upgrade = installed = multi_installed =
		slotted = multi_slot =
		world = world_only_selected = world_only_file =
		worldset = worldset_only_selected =
		dup_versions = dup_packages =
		have_virtual = have_nonvirtual =
		know_pattern = false;
	restrictions = ExtendedVersion::RESTRICT_NONE;
	properties = ExtendedVersion::PROPERTIES_NONE;
	binarynum = 0;
	test_installed = INS_NONE;
	test_instability = test_stability_default =
		test_stability_local = test_stability_nonlocal = STABLE_NONE;
}

PackageTest::~PackageTest() {
	setAlgorithm(NULLPTR);
	delete overlay_list;
	delete overlay_only_list;
	delete in_overlay_inst_list;
	delete from_overlay_inst_list;
	delete from_foreign_overlay_inst_list;
}

void PackageTest::calculateNeeds() {
	need = PackageReader::NONE;
	if((field & (SRC_URI | EAPI | SLOT | FULLSLOT | SET)) != NONE) {
		setNeeds(PackageReader::VERSIONS);
	}
	if((field & HOMEPAGE) != NONE) {
		setNeeds(PackageReader::HOMEPAGE);
	}
	if((field & LICENSE) != NONE) {
		setNeeds(PackageReader::LICENSE);
	}
	if((field & DESCRIPTION) != NONE) {
		setNeeds(PackageReader::DESCRIPTION);
	}
	if((field & CATEGORY) != NONE) {
		setNeeds(PackageReader::NONE);
	}
	if((field & (NAME | CATEGORY_NAME)) != NONE) {
		setNeeds(PackageReader::NAME);
	}
	if((field & (USE_ENABLED | USE_DISABLED | INST_EAPI | INST_SLOT | INST_FULLSLOT | DEPSI)) != NONE) {
		setNeeds(PackageReader::NAME);
	}
	if(installed) {
		setNeeds(PackageReader::NAME);
	}
	if(!Depend::use_depend) {
		field &= ~DEPSA;
	}
	if(((field & (IUSE | DEPSA)) != NONE) ||
		dup_packages || dup_versions || slotted ||
		upgrade || overlay || obsolete ||
		world || worldset ||
		have_virtual || have_nonvirtual ||
		(from_overlay_inst_list != NULLPTR) ||
		(from_foreign_overlay_inst_list != NULLPTR) ||
		(overlay_list != NULLPTR) || (overlay_only_list != NULLPTR) ||
		(in_overlay_inst_list != NULLPTR) ||
		(marked_list != NULLPTR) ||
		(restrictions != ExtendedVersion::RESTRICT_NONE) ||
		(properties != ExtendedVersion::PROPERTIES_NONE) ||
		(binarynum != 0) ||
		(test_instability != STABLE_NONE) ||
		(test_stability_default != STABLE_NONE) ||
		(test_stability_local != STABLE_NONE) ||
		(test_stability_nonlocal != STABLE_NONE))
		setNeeds(PackageReader::VERSIONS);
}

void PackageTest::finalize() {
	if(!know_pattern) {
		setPattern("");
	}
	calculateNeeds();
}

/**
@return true if pkg matches test
**/
bool PackageTest::stringMatch(Package *pkg) const {
	if((((field & NAME) != NONE) && (*algorithm)(pkg->name.c_str(), pkg, true))
	|| (((field & DESCRIPTION) != NONE)  && (*algorithm)(pkg->desc.c_str(), pkg))
	|| (((field & LICENSE) != NONE) && (*algorithm)(pkg->licenses.c_str(), pkg))
	|| (((field & CATEGORY) != NONE) && (*algorithm)(pkg->category.c_str(), pkg, true))
	|| (((field & CATEGORY_NAME) != NONE) && (*algorithm)((pkg->category + "/" + pkg->name).c_str(), pkg, true))
	|| (((field & HOMEPAGE) != NONE) && (*algorithm)(pkg->homepage.c_str(), pkg))) {
		return true;
	}

	if((field & SRC_URI) != NONE) {
		for(Package::iterator it(pkg->begin());
			likely(it != pkg->end()); ++it) {
			if((*algorithm)(it->src_uri.c_str(), pkg))
				return true;
		}
	}

	if((field & EAPI) != NONE) {
		for(Package::iterator it(pkg->begin());
			likely(it != pkg->end()); ++it) {
			if((*algorithm)(it->eapi.get().c_str(), pkg))
				return true;
		}
	}

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
		const IUseSet::IUseStd& s(pkg->iuse.asStd());
		for(IUseSet::IUseStd::const_iterator it(s.begin());
			it != s.end(); ++it) {
			if((*algorithm)(it->name().c_str(), NULLPTR))
				return true;
		}
	}

	if((field & DEPSA) != NONE) {
		bool depend((field & DEPENDA) != NONE);
		bool rdepend((field & RDEPENDA) != NONE);
		bool pdepend((field & PDEPENDA) != NONE);
		bool bdepend((field & BDEPENDA) != NONE);
		for(Package::iterator it(pkg->begin());
			likely(it != pkg->end()); ++it) {
			const Depend &dep(it->depend);
			if((depend && (*algorithm)(dep.get_depend().c_str(), pkg))
			|| (rdepend && (*algorithm)(dep.get_rdepend().c_str(), pkg))
			|| (pdepend && (*algorithm)(dep.get_pdepend().c_str(), pkg))
			|| (bdepend && (*algorithm)(dep.get_bdepend().c_str(), pkg))) {
				return true;
			}
		}
	}

	if((field & SET) != NONE) {
		WordSet setnames;
		portagesettings->get_setnames(&setnames, pkg);
		for(WordSet::const_iterator it(setnames.begin());
			likely(it != setnames.end()); ++it) {
			if((*algorithm)(it->c_str(), NULLPTR)
			|| (*algorithm)((string("@") + *it).c_str(), NULLPTR)) {
				return true;
			}
		}
	}

	if((field & (USE_ENABLED | USE_DISABLED | INST_EAPI | INST_SLOT | INST_FULLSLOT | DEPSI)) == NONE) {
		return false;
	}

	InstVec *installed_versions(vardbpkg->getInstalledVector(*pkg));
	if(installed_versions == NULLPTR) {
		return false;
	}

	if((field & INST_EAPI) != NONE) {
		for(InstVec::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			vardbpkg->readEapi(*pkg, &(*it));
			if((*algorithm)(it->eapi.get().c_str(), pkg)) {
				return true;
			}
		}
	}

	if((field & (INST_SLOT | INST_FULLSLOT)) != NONE) {
		for(InstVec::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			if(!vardbpkg->readSlot(*pkg, &(*it))) {
				continue;
			}
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
		for(InstVec::iterator it(installed_versions->begin());
			it != installed_versions->end(); ++it) {
			if(!vardbpkg->readUse(*pkg, &(*it))) {
				continue;
			}
			if((field & USE_ENABLED) != NONE) {
				for(WordSet::iterator uit((it->usedUse).begin());
					likely(uit != (it->usedUse).end()); ++uit) {
					if((*algorithm)(uit->c_str(), NULLPTR)) {
						return true;
					}
				}
			}
			if((field & USE_DISABLED) != NONE) {
				for(WordVec::iterator uit((it->inst_iuse).begin());
					likely(uit != (it->inst_iuse).end()); ++uit) {
					if(!(*algorithm)(uit->c_str(), NULLPTR)) {
						continue;
					}
					if((it->usedUse).count(*uit) == 0) {
						return true;
					}
				}
			}
		}
	}

	if((field & DEPSI) != NONE) {
		bool depend((field & DEPENDI) != NONE);
		bool rdepend((field & RDEPENDI) != NONE);
		bool pdepend((field & PDEPENDI) != NONE);
		bool bdepend((field & BDEPENDI) != NONE);
		for(InstVec::iterator it(installed_versions->begin());
			it != installed_versions->end(); ++it) {
			vardbpkg->readDepend(*pkg, &(*it), *header);
			const Depend& dep(it->depend);
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
			if(bdepend) {
				if((*algorithm)(dep.get_bdepend().c_str(), pkg))
				return true;
			}
		}
	}

	return false;
}

bool PackageTest::have_redundant(const Package& p, Keywords::Redundant r, const RedAtom& t) const {
	r &= t.red;
	if(r == Keywords::RED_NOTHING) {
		return false;
	}
	if((t.only & r) != Keywords::RED_NOTHING) {
		if((vardbpkg->numInstalled(p) != 0) != ((t.oins & r) != 0)) {
			return false;
		}
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
				if(test_unrestricted) {
					return false;
				}
				eix::SignedBool is_installed(vardbpkg->isInstalledVersion(p, *pi, *header));
				// If in doubt about the overlay only consider as installed
				// if the current version was not treated yet, i.e. (since we use reverse_iterator)
				// if it is the highest version (and so usually from the last overlay)
				if(is_installed < 0) {
					if(prev_ver && (**pi == *prev_ver))
						is_installed = 0;
				}
				if(test_uninstalled == !is_installed) {
					return false;
				}
			}
		}
		return rvalue;
	} else {
		// test some or some-installed
		for(Package::const_iterator pi(p.begin());
			likely(pi != p.end()); ++pi) {
			if((pi->get_redundant()) & r) {
				if(test_unrestricted) {
					return true;
				}
				// in contrast to the above loop, we now in doubt
				// do not distinguish overlays.
				if(test_uninstalled ==
					!(vardbpkg->isInstalledVersion(p, *pi, *header))) {
					return true;
				}
			}
		}
		return false;
	}
}

bool PackageTest::have_redundant(const Package& p, Keywords::Redundant r) const {
	if(r == Keywords::RED_NOTHING) {
		return false;
	}
	if(have_redundant(p, r, first_test)) {
		return true;
	}
	if(have_redundant(p, r, second_test)) {
		return true;
	}
	return false;
}

static bool stabilitytest(const Package *p, PackageTest::TestStability what) {
	if(likely(what == PackageTest::STABLE_NONE)) {
		return true;
	}
	for(Package::const_iterator it(p->begin()); likely(it != p->end()); ++it) {
		if((what & PackageTest::STABLE_SYSTEMPROFILE) != PackageTest::STABLE_NONE) {
			if((what & PackageTest::STABLE_SYSTEM) != PackageTest::STABLE_NONE) {
				if(!it->maskflags.isSystem()) {
					continue;
				}
			}
			if((what & PackageTest::STABLE_PROFILE) != PackageTest::STABLE_NONE) {
				if(!it->maskflags.isProfile()) {
					continue;
				}
			}
			if(!(what & ~PackageTest::STABLE_SYSTEMPROFILE)) {
				return true;
			}
		}
		if(it->maskflags.isHardMasked()) {
			continue;
		}
		if(it->keyflags.isStable()) {
			return true;
		}
		if(what & PackageTest::STABLE_FULL) {
			continue;
		}
		if(it->keyflags.isUnstable()) {
			return true;
		}
		if(what & PackageTest::STABLE_TESTING) {
			continue;
		}
		// what & PackageTest::STABLE_NONMASKED
			return true;
	}
	return false;
}

bool PackageTest::instabilitytest(const Package *p, TestStability what) const {
	if(likely(what == STABLE_NONE)) {
		return true;
	}
	for(Package::const_iterator it(p->begin()); likely(it != p->end()); ++it) {
		TestStability have(STABLE_NONE);
		if(it->maskflags.isHardMasked()) {
			have |= (STABLE_FULL | STABLE_NONMASKED);
		}
		if(!it->keyflags.isStable()) {
			have |= STABLE_FULL;
		}
		if(it->keyflags.isUnstable()) {
			have |= (STABLE_FULL | STABLE_TESTING);
		}
		if((what & have) != what) {
			continue;
		}
		if(vardbpkg->isInstalledVersion(*p, *it, *header)) {
			return true;
		}
	}
	return false;
}

inline static void get_p(Package **p, PackageReader *pkg) {
	if(unlikely(*p == NULLPTR)) {
		*p = pkg->get();
	}
}

bool PackageTest::match(PackageReader *pkg) const {
	Package *p(NULLPTR);

	pkg->read(need);

	/* Test the local options.
	Each test must start with get_p(&p, pkg) to get p; remember to modify
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
	   ensure the versions really have been read for the package. */

	if(unlikely(algorithm != NULLPTR)) {
		get_p(&p, pkg);
		if(!stringMatch(p)) {
			return false;
		}
	}

	if(unlikely(slotted)) {
		// -1 or -2
		get_p(&p, pkg);
		if(!(p->have_nontrivial_slots())) {
			return false;
		}
		if(multi_slot) {
			if((p->slotlist()).size() <= 1) {
				return false;
			}
		}
	}

	if(unlikely(overlay)) {
		// -O
		get_p(&p, pkg);
		if(!(p->largest_overlay)) {
			return false;
		}
	}

	if(unlikely(overlay_list != NULLPTR)) {
		// --in-overlay
		get_p(&p, pkg);
		bool have(false);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(unlikely(overlay_list->count(it->overlay_key) == 0)) {
				continue;
			}
			have = true;
			break;
		}
		if(!have) {
			return false;
		}
	}

	if(unlikely(overlay_only_list != NULLPTR)) {
		// --only-in-overlay
		get_p(&p, pkg);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(likely(overlay_only_list->count(it->overlay_key) == 0)) {
				return false;
			}
		}
	}

	if(unlikely(have_virtual)) {
		// --virtual
		get_p(&p, pkg);
		if(!print_format->have_virtual(p, false)) {
			return false;
		}
	}

	if(unlikely(have_nonvirtual)) {
		// --nonvirtual
		get_p(&p, pkg);
		if(!print_format->have_virtual(p, true)) {
			return false;
		}
	}

	if(unlikely(installed)) {
		// -i or -I
		get_p(&p, pkg);
		InstVec::size_type s(vardbpkg->numInstalled(*p));
		if(s == 0) {
			return false;
		}
		if(s == 1) {
			if(multi_installed) {
				return false;
			}
		}
	}

	if(unlikely(in_overlay_inst_list != NULLPTR)) {
		// --installed-in-[some-]overlay
		get_p(&p, pkg);
		bool have(false);
		bool get_installed(true);
		InstVec *installed_versions(NULLPTR);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(in_overlay_inst_list->count(it->overlay_key) == 0) {
				continue;
			}
			if(get_installed) {
				get_installed = false;
				installed_versions = vardbpkg->getInstalledVector(*p);
			}
			if(installed_versions == NULLPTR) {
				continue;
			}
			if(VarDbPkg::isInVec(installed_versions, *it)) {
				have = true;
				break;
			}
		}
		if(!have) {
			return false;
		}
	}

	if(unlikely((from_overlay_inst_list != NULLPTR) ||
		(from_foreign_overlay_inst_list != NULLPTR))) {
		// -J or --installed-from-overlay
		get_p(&p, pkg);
		bool have(false);
		InstVec *installed_versions(vardbpkg->getInstalledVector(*p));
		if(installed_versions == NULLPTR) {
			return false;
		}
		for(InstVec::iterator it(installed_versions->begin());
			likely(it != installed_versions->end()); ++it) {
			if(vardbpkg->readOverlay(*p, &(*it), *header)) {
				if(from_overlay_inst_list == NULLPTR) {
					continue;
				}
				if(from_overlay_inst_list->count(it->overlay_key) == 0) {
					continue;
				}
				have = true;
				break;
			}
		}
		if(!have) {
			return false;
		}
	}

	if(unlikely(dup_packages)) {
		// -d
		get_p(&p, pkg);
		if(dup_packages_overlay) {
			if(!(p->at_least_two_overlays())) {
				return false;
			}
		} else if(p->have_same_overlay_key()) {
			return false;
		}
	}

	if(unlikely(dup_versions)) {
		// -D
		get_p(&p, pkg);
		Package::Duplicates testfor((dup_versions_overlay) ?
				Package::DUP_OVERLAYS : Package::DUP_SOME);
		if(((p->have_duplicate_versions) & testfor) != testfor) {
			return false;
		}
	}

	if(unlikely((restrictions != ExtendedVersion::RESTRICT_NONE) ||
		(properties != ExtendedVersion::PROPERTIES_NONE))) {
		get_p(&p, pkg);
		bool found(false);
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			if(unlikely((((it->restrictFlags) & restrictions) == restrictions) &&
				(((it->propertiesFlags) & properties) == properties))) {
				found = true;
				break;
			}
		}
		if(!found) {
			InstVec *installed_versions(vardbpkg->getInstalledVector(*p));
			if(installed_versions == NULLPTR) {
				return false;
			}
			for(InstVec::iterator it(installed_versions->begin());
				likely(it != installed_versions->end()); ++it) {
				vardbpkg->readRestricted(*p, &(*it), *header);
				if(unlikely((((it->restrictFlags) & restrictions) == restrictions) &&
					(((it->propertiesFlags) & properties) == properties))) {
					found = true;
					break;
				}
			}
			if(!found) {
				return false;
			}
		}
	}

	if(binarynum != 0) {
		// --binary --multibinary
		get_p(&p, pkg);
		bool found(false);
		for(Package::iterator it(p->begin()); it != p->end(); ++it) {
			if(unlikely(it->have_bin_pkg(portagesettings, p, binarynum))) {
				found = true;
				break;
			}
		}
		if(!found) {
			InstVec *installed_versions(vardbpkg->getInstalledVector(*p));
			if(installed_versions == NULLPTR) {
				return false;
			}
			for(InstVec::iterator it(installed_versions->begin());
				likely(it != installed_versions->end()); ++it) {
				if(unlikely(it->have_bin_pkg(portagesettings, p, binarynum))) {
					found = true;
					break;
				}
			}
			if(!found) {
				return false;
			}
		}
	}

	while(unlikely(obsolete)) {
		// -T; loop, because we break in case of success
		// Can some test succeed at all?
		if((test_installed == INS_NONE) &&
			(redundant_flags == Keywords::RED_NOTHING)) {
			return false;
		}

		if(nowarn_list == NULLPTR) {
			get_nowarn_list();
		}
		get_p(&p, pkg);
		Keywords::Redundant rflags(redundant_flags);
		TestInstalled test_ins(test_installed);
		nowarn_list->apply(p, &rflags, &test_ins, portagesettings);

		Keywords::Redundant r(rflags & Keywords::RED_ALL_MASKSTUFF);
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->setMasks(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_MASK)
				|| have_redundant(*p, r & Keywords::RED_DOUBLE_UNMASK)
				|| have_redundant(*p, r & Keywords::RED_MASK)
				|| have_redundant(*p, r & Keywords::RED_UNMASK)
				|| have_redundant(*p, r & Keywords::RED_IN_MASK)
				|| have_redundant(*p, r & Keywords::RED_IN_UNMASK)) {
					break;
				}
			}
		}
		r = rflags & Keywords::RED_ALL_KEYWORDS;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->setKeyflags(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE)
				|| have_redundant(*p, r & Keywords::RED_MIXED)
				|| have_redundant(*p, r & Keywords::RED_WEAKER)
				|| have_redundant(*p, r & Keywords::RED_STRANGE)
				|| have_redundant(*p, r & Keywords::RED_NO_CHANGE)
				|| have_redundant(*p, r & Keywords::RED_IN_KEYWORDS)
				|| have_redundant(*p, r & Keywords::RED_DOUBLE_LINE)) {
					break;
				}
			}
		}
		r = rflags & Keywords::RED_ALL_USE;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckUse(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_USE)
				|| have_redundant(*p, r & Keywords::RED_IN_USE)) {
					break;
				}
			}
		}
		r = rflags & Keywords::RED_ALL_ENV;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckEnv(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_ENV)
				|| have_redundant(*p, r & Keywords::RED_IN_ENV)) {
					break;
				}
			}
		}
		r = rflags & Keywords::RED_ALL_LICENSE;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckLicense(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_LICENSE)
				|| have_redundant(*p, r & Keywords::RED_IN_LICENSE)) {
					break;
				}
			}
		}
		r = rflags & Keywords::RED_ALL_RESTRICT;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckAcceptRestrict(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_RESTRICT)
				|| have_redundant(*p, r & Keywords::RED_IN_RESTRICT)) {
					break;
				}
			}
		}
		r = rflags & Keywords::RED_ALL_CFLAGS;
		if(r != Keywords::RED_NOTHING) {
			if(r && portagesettings->user_config->CheckCflags(p, r)) {
				if(have_redundant(*p, r & Keywords::RED_DOUBLE_CFLAGS)
				|| have_redundant(*p, r & Keywords::RED_IN_CFLAGS)) {
					break;
				}
			}
		}
		if(test_ins == INS_NONE) {
			return false;
		}
		InstVec *installed_versions(vardbpkg->getInstalledVector(*p));
		if(installed_versions == NULLPTR) {
			return false;
		}
		if(test_ins & INS_MASKED) {
			portagesettings->user_config->setMasks(p);
			portagesettings->user_config->setKeyflags(p);
		}
		InstVec::iterator current(installed_versions->begin());
		for( ; likely(current != installed_versions->end()); ++current) {
			bool not_all_found(true);
			for(Package::iterator version_it(p->begin());
				likely(version_it != p->end()); ++version_it) {
				Version *version(*version_it);
				if(*version != *current) {
					continue;
				}
				if(test_ins & INS_MASKED) {
					if(version->maskflags.isHardMasked() || !version->keyflags.isStable()) {
						continue;
					}
				}
				if(test_ins & INS_OVERLAY) {
					if(!vardbpkg->readOverlay(*p, &(*current), *header)) {
						continue;
					}
					if(current->overlay_key != version_it->overlay_key) {
						continue;
					}
				}
				not_all_found = false;
			}
			if(not_all_found) {
				break;
			}
		}
		if(current != installed_versions->end()) {
			break;
		}
		return false;
	}

	if(unlikely(upgrade)) {
		// -u
		get_p(&p, pkg);
		if(upgrade_local_mode == LOCALMODE_DEFAULT) {
			StabilityDefault(p);
		} else if(upgrade_local_mode == LOCALMODE_LOCAL) {
			StabilityLocal(p);
		} else if(upgrade_local_mode == LOCALMODE_NONLOCAL) {
			StabilityNonlocal(p);
		}
		if(!(p->recommend(vardbpkg, portagesettings, true, true))) {
			return false;
		}
	}

	if(unlikely(test_stability_default != STABLE_NONE)) {
		// --stable, --testing, --non-masked, --system
		get_p(&p, pkg);
		StabilityDefault(p);
		if(!stabilitytest(p, test_stability_default)) {
			return false;
		}
	}

	if(unlikely(test_stability_local != STABLE_NONE)) {
		// --stable+, --testing+, --non-masked+, --system+
		get_p(&p, pkg);
		StabilityLocal(p);
		if(!stabilitytest(p, test_stability_local)) {
			return false;
		}
	}

	if(unlikely(test_stability_nonlocal != STABLE_NONE)) {
		// --stable-, --testing-, --non-masked-, --system-
		get_p(&p, pkg);
		StabilityNonlocal(p);
		if(!stabilitytest(p, test_stability_nonlocal)) {
			return false;
		}
	}

	if(unlikely(test_instability != STABLE_NONE)) {
	// --installed-unstable --installed-testing --installed-masked
		get_p(&p, pkg);
		StabilityNonlocal(p);
		if(!instabilitytest(p, test_instability)) {
			return false;
		}
	}

	if(unlikely(world || worldset)) {
		// --world, --world-all, --world-set
		// --selected, --selected-all, --selected-set
		get_p(&p, pkg);
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

	if(unlikely(marked_list != NULLPTR)) {
		get_p(&p, pkg);
		if(likely(!marked_list->MaskMatches(p))) {
			return false;
		}
	}

	// all tests succeeded:
	return true;
}  // NOLINT(readability/fn_size)

void PackageTest::get_nowarn_list() const {
	NowarnPreList prelist;
	WordVec name;
	EixRc& rc(get_eixrc());
	split_string(&name, rc["PACKAGE_NOWARN"], true);
	for(WordVec::const_iterator it(name.begin()); it != name.end(); ++it) {
		LineVec lines;
		pushback_lines(it->c_str(), &lines, true);
		prelist.handle_file(lines, *it, NULLPTR, true, false, false);
	}
	nowarn_list = new NowarnMaskList;
	prelist.initialize(nowarn_list, parse_error);
}
