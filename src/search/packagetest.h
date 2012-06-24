// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__PACKAGETEST_H__
#define EIX__PACKAGETEST_H__ 1

#include <database/package_reader.h>
#include <eixTk/inttypes.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <portage/extendedversion.h>
#include <portage/keywords.h>
#include <portage/package.h>
#include <portage/set_stability.h>
#include <search/algorithms.h>
#include <search/redundancy.h>

#include <set>
#include <string>
#include <vector>

class MatcherAlgorithm;
class MatcherField;
class NowarnMaskList;
class PortageSettings;
class VarDbPkg;

/** Test a package if it matches some criteria. */
class PackageTest {
		friend class MatcherField;
		friend class MatcherAlgorithm;

	public:
		typedef uint32_t MatchField;
		static const MatchField
			NONE          = 0x00000, /**< Search in name */
			NAME          = 0x00001, /**< Search in name */
			DESCRIPTION   = 0x00002, /**< Search in description */
			LICENSE       = 0x00004, /**< Search in license */
			CATEGORY      = 0x00008, /**< Search in category */
			CATEGORY_NAME = 0x00010, /**< Search in category/name */
			HOMEPAGE      = 0x00020, /**< Search in homepage */
			IUSE          = 0x00040, /**< Search in iuse */
			USE_ENABLED   = 0x00080, /**< Search in enabled  useflags of installed packages */
			USE_DISABLED  = 0x00100, /**< Search in disabled useflags of installed packages */
			SLOT          = 0x00200, /**< Search in slots */
			FULLSLOT      = 0x00400, /**< Search in full slots */
			INST_SLOT     = 0x00800, /**< Search in installed slots */
			INST_FULLSLOT = 0x01000, /**< Search in installed full slots */
			SET           = 0x02000, /**< Search in sets */
			DEPEND        = 0x04000, /**< Search in DEPEND */
			RDEPEND       = 0x08000, /**< Search in RDEPEND */
			PDEPEND       = 0x10000, /**< Search in PDEPEND */
			DEPS          = (DEPEND|RDEPEND|PDEPEND),
			ANY           = (NAME|DESCRIPTION|LICENSE|CATEGORY|CATEGORY_NAME|HOMEPAGE|IUSE|USE_ENABLED|USE_DISABLED|SLOT|FULLSLOT|INST_SLOT|INST_FULLSLOT|SET|DEPS);

		enum MatchAlgorithm {
			ALGO_REGEX,
			ALGO_EXACT,
			ALGO_BEGIN,
			ALGO_END,
			ALGO_SUBSTRING,
			ALGO_PATTERN,
			ALGO_FUZZY
		};

		typedef uint8_t TestInstalled;
		static const TestInstalled
			INS_NONE        = 0x00,
			INS_NONEXISTENT = 0x01, /**< Test for nonexistent installed packages */
			INS_OVERLAY     = 0x02, /**< Test for nonexistent overlays of installed packages */
			INS_MASKED      = 0x04, /**< Test for masked installed packages */
			INS_SOME        = INS_NONEXISTENT|INS_OVERLAY|INS_MASKED;

		typedef uint8_t TestStability;
		static const TestStability
			STABLE_NONE         = 0x00,
			STABLE_FULL         = 0x01, /**< Test for stable keyword */
			STABLE_TESTING      = 0x02, /**< Test for testing keyword */
			STABLE_NONMASKED    = 0x04, /**< Test for non-masked packages */
			STABLE_SYSTEM       = 0x08; /**< Test for system packages */

		/** Set default values. */
		PackageTest(VarDbPkg &vdb, PortageSettings &p, const SetStability &stability, const DBHeader &dbheader);

		~PackageTest();

		void setAlgorithm(BaseAlgorithm *p)
		{
			if(algorithm != NULLPTR) {
				delete algorithm;
			}
			algorithm = p;
		}

		void setAlgorithm(MatchAlgorithm a);

		void setPattern(const char *p);

		bool match(PackageReader *pkg) const;

		/** Set defaults (e.g. matchfield if unspecified),
		    calculate needs. */
		void finalize();

		// The constructor of the class *must* set the least restrictive choice.
		// Since --selected --world must act like --selected, the less restrictive
		// choice (here --selected) must change the variable unconditionally,
		// and so the default set in the constructor must have been opposite.
		// We thus name the variables such that the less restrictive version
		// corresponds to false/NULLPTR and let the constructor set all to false/NULLPTR.

		void Installed()
		{ installed = true; }

		void MultiInstalled()
		{ installed = multi_installed = true; }

		void Slotted()
		{ slotted = true; }

		void MultiSlotted()
		{ slotted = multi_slot = true; }

		void Upgrade(LocalMode local_mode)
		{  upgrade = true; upgrade_local_mode = local_mode; }

		void SetStabilityDefault(TestStability require)
		{  test_stability_default |= require; }

		void SetStabilityLocal(TestStability require)
		{  test_stability_local |= require; }

		void SetStabilityNonlocal(TestStability require)
		{  test_stability_nonlocal |= require; }

		void SetInstability(TestStability avoid)
		{  test_instability |= avoid; }

		void Overlay()
		{ overlay = true; }

		void Restrictions(ExtendedVersion::Restrict flags)
		{ restrictions |= flags; }

		void Properties(ExtendedVersion::Properties flags)
		{ properties |= flags; }

		void Binary()
		{ binary = true; }

		void WorldAll()
		{ world = true; }

		void WorldFile()
		{ world = world_only_file = true; }

		void SelectedAll()
		{ world = world_only_selected = true; }

		void SelectedFile()
		{ world = world_only_selected = world_only_file = true; }

		void WorldSet()
		{ worldset = true; }

		void SelectedSet()
		{ worldset = worldset_only_selected = true; }

		void StabilityDefault(Package *p) const
		{ stability->set_stability(*p); }

		void StabilityLocal(Package *p) const
		{ stability->set_stability(true, *p); }

		void StabilityNonlocal(Package *p) const
		{ stability->set_stability(false, *p); }

		std::set<ExtendedVersion::Overlay> *OverlayList()
		{
			if(likely(overlay_list == NULLPTR))
				overlay_list = new std::set<ExtendedVersion::Overlay>;
			return overlay_list;
		}

		std::set<ExtendedVersion::Overlay> *OverlayOnlyList()
		{
			if(likely(overlay_only_list == NULLPTR))
				overlay_only_list = new std::set<ExtendedVersion::Overlay>;
			return overlay_only_list;
		}

		std::set<ExtendedVersion::Overlay> *InOverlayInstList()
		{
			if(likely(in_overlay_inst_list == NULLPTR))
				in_overlay_inst_list = new std::set<ExtendedVersion::Overlay>;
			return in_overlay_inst_list;
		}

		std::set<ExtendedVersion::Overlay> *FromOverlayInstList()
		{
			if(likely(from_overlay_inst_list == NULLPTR))
				from_overlay_inst_list = new std::set<ExtendedVersion::Overlay>;
			return from_overlay_inst_list;
		}

		std::vector<std::string> *FromForeignOverlayInstList()
		{
			if(likely(from_foreign_overlay_inst_list == NULLPTR))
				from_foreign_overlay_inst_list = new std::vector<std::string>;
			return from_foreign_overlay_inst_list;
		}

		void DuplVersions(bool only_overlay)
		{ dup_versions = true ; dup_versions_overlay = only_overlay; }

		void DuplPackages(bool only_overlay)
		{ dup_packages = true; dup_packages_overlay = only_overlay; }

		void ObsoleteCfg(const RedAtom &first, const RedAtom &second, TestInstalled test_ins)
		{
			obsolete           = true;
			redundant_flags    = first.red|second.red;
			first_test         = first;
			second_test        = second;
			test_installed     = test_ins;
		}

		MatchField operator |= (const MatchField m)
		{ return field |= m; }

		MatchField operator = (const MatchField m)
		{ return field = m; }

	private:
		static NowarnMaskList *nowarn_list;
		static void get_nowarn_list();

		/* What to match. */
		MatchField field;
		/** Lookup stuff about installed packages here. */
		VarDbPkg *vardbpkg;
		/** When reading overlay information use this: */
		const DBHeader *header;

		/** What we need to read so we can do our testing. */
		PackageReader::Attributes need;
		/** Our string matching algorithm. */
		BaseAlgorithm *algorithm;

		/** Other flags for tests */
		bool	overlay, obsolete, upgrade, binary,
			installed, multi_installed,
			slotted, multi_slot,
			world, world_only_file, world_only_selected,
			worldset, worldset_only_selected,
			know_pattern;
		LocalMode upgrade_local_mode;
		bool dup_versions, dup_versions_overlay;
		bool dup_packages, dup_packages_overlay;
		ExtendedVersion::Restrict restrictions;
		ExtendedVersion::Properties properties;

		std::set<ExtendedVersion::Overlay>
			*overlay_list, *overlay_only_list,
			*in_overlay_inst_list;

		std::set<ExtendedVersion::Overlay> *from_overlay_inst_list;
		std::vector<std::string> *from_foreign_overlay_inst_list;

		PortageSettings *portagesettings;
		/** Lookup stuff about user flags here. */
		const SetStability *stability,
			*stability_local, *stability_nonlocal;
		/* Test for this redundancy: */
		Keywords::Redundant redundant_flags;
		RedAtom first_test, second_test;
		TestInstalled test_installed;
		TestStability test_stability_default,
			test_stability_local, test_stability_nonlocal;
		TestStability test_instability;

		static MatchField     name2field(const std::string &p);
		static MatchAlgorithm name2algorithm(const std::string &p);
		static MatchField     get_matchfield(const char *p);
		static MatchAlgorithm get_matchalgorithm(const char *p);

		bool stringMatch(Package *pkg) const;

		void setNeeds(const PackageReader::Attributes i)
		{
			if(need < i)
				need = i;
		}

		/** Get the Fetched-value that is required to determine the match */
		void calculateNeeds();

		bool have_redundant(const Package &p, Keywords::Redundant r, const RedAtom &t) const;
		bool have_redundant(const Package &p, Keywords::Redundant r) const;
		bool instabilitytest(const Package *p, TestStability what) const;

		static Keywords::Redundant nowarn_keywords(const Package &p);
		static Keywords::Redundant nowarn_mask(const Package &p);
		static Keywords::Redundant nowarn_use(const Package &p);
		static Keywords::Redundant nowarn_env(const Package &p);
		static Keywords::Redundant nowarn_cflags(const Package &p);
		static TestInstalled nowarn_installed(const Package &p);
};

#endif /* EIX__PACKAGETEST_H__ */
