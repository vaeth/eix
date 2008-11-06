// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PACKAGETEST_H__
#define __PACKAGETEST_H__

#include <portage/vardbpkg.h>
#include <portage/version.h>
#include <portage/conf/portagesettings.h>

#include <database/types.h>
#include <database/package_reader.h>

#include <eixTk/exceptions.h>
#include <eixTk/inttypes.h>

#include <search/algorithms.h>
#include <search/redundancy.h>

#include <portage/set_stability.h>

/** Test a package if it matches some criteria. */
class PackageTest {

	public:
		typedef uint16_t MatchField;
		static const MatchField
			NONE          =0x0000, /**< Search in name */
			NAME          =0x0001, /**< Search in name */
			DESCRIPTION   =0x0002, /**< Search in description */
			PROVIDE       =0x0004, /**< Search in provides */
			LICENSE       =0x0008, /**< Search in license */
			CATEGORY      =0x0010, /**< Search in category */
			CATEGORY_NAME =0x0020, /**< Search in category/name */
			HOMEPAGE      =0x0040, /**< Search in homepage */
			IUSE          =0x0080, /**< Search in iuse */
			USE_ENABLED   =0x0100, /**< Search in enabled  useflags of installed packages */
			USE_DISABLED  =0x0200, /**< Search in disabled useflags of installed packages */
			SLOT          =0x0400, /**< Search in slots */
			INSTALLED_SLOT=0x0800, /**< Search in installed slots */
			SET           =0x1000; /**< Search in sets */

		typedef uint8_t TestInstalled;
		static const TestInstalled
			INS_NONE        = 0x00,
			INS_NONEXISTENT = 0x01, /**< Test for nonexistent installed packages */
			INS_OVERLAY     = 0x02, /**< Test for nonexistent overlays of installed packages */
			INS_MASKED      = 0x04, /**< Test for masked installed packages */
			INS_SOME        = INS_NONEXISTENT|INS_OVERLAY|INS_MASKED;

		typedef uint8_t TestStability;
		static const TestStability
			STABLE_NONE      = 0x00,
			STABLE_FULL      = 0x01, /**< Test for stable keyword */
			STABLE_TESTING   = 0x02, /**< Test for testing keyword */
			STABLE_NONMASKED = 0x04, /**< Test for non-masked packages */
			STABLE_SYSTEM    = 0x08; /**< Test for system packages */

		/** Set default values. */
		PackageTest(VarDbPkg &vdb, PortageSettings &p, const SetStability &stability, const DBHeader &dbheader);

		~PackageTest() {
			if(overlay_list) {
				delete overlay_list;
				overlay_list = NULL;
			}
			if(overlay_only_list) {
				delete overlay_only_list;
				overlay_only_list = NULL;
			}
			if(in_overlay_inst_list) {
				delete in_overlay_inst_list;
				in_overlay_inst_list = NULL;
			}
			if(from_overlay_inst_list) {
				delete from_overlay_inst_list;
				from_overlay_inst_list = NULL;
			}
			if(from_foreign_overlay_inst_list) {
				delete from_foreign_overlay_inst_list;
				from_foreign_overlay_inst_list = NULL;
			}
		}

		void setAlgorithm(BaseAlgorithm *p)
		{ algorithm = std::auto_ptr<BaseAlgorithm>(p); }

		void setPattern(const char *p);

		bool match(PackageReader *pkg) const;

		/** Compile regex and/or calculate needs. */
		void finalize()
		{ calculateNeeds(); }

		void Installed(bool multi = false)
		{ installed = true; multi_installed = multi; }

		void Slotted(bool multi = false)
		{ slotted = true; multi_slot = multi; }

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

		void World(bool match_also_sets = false)
		{ world = true; world_both = match_also_sets; }

		void WorldSet()
		{ worldset = true; }

		void StabilityDefault(Package *p) const
		{ stability->set_stability(*p); }

		void StabilityLocal(Package *p) const
		{ stability->set_stability(true, *p); }

		void StabilityNonlocal(Package *p) const
		{ stability->set_stability(false, *p); }

		std::set<Version::Overlay> *OverlayList()
		{
			if(!overlay_list)
				overlay_list = new std::set<Version::Overlay>;
			return overlay_list;
		}

		std::set<Version::Overlay> *OverlayOnlyList()
		{
			if(!overlay_only_list)
				overlay_only_list = new std::set<Version::Overlay>;
			return overlay_only_list;
		}

		std::set<Version::Overlay> *InOverlayInstList()
		{
			if(!in_overlay_inst_list)
				in_overlay_inst_list = new std::set<Version::Overlay>;
			return in_overlay_inst_list;
		}

		std::set<Version::Overlay> *FromOverlayInstList()
		{
			if(!from_overlay_inst_list)
				from_overlay_inst_list = new std::set<Version::Overlay>;
			return from_overlay_inst_list;
		}

		std::vector<std::string> *FromForeignOverlayInstList()
		{
			if(!from_foreign_overlay_inst_list)
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

		void Invert()
		{ invert = !invert; }

		MatchField operator |= (const MatchField m)
		{ return field |= m; }

		MatchField operator = (const MatchField m)
		{ return field = m; }

	protected:

	private:
		/* What to match. */
		MatchField field;
		/** Lookup stuff about installed packages here. */
		VarDbPkg *vardbpkg;
		/** When reading overlay information use this: */
		const DBHeader *header;

		/** What we need to read so we can do our testing. */
		PackageReader::Attributes need;
		/** Our string matching algorithm. */
		std::auto_ptr<BaseAlgorithm> algorithm;

		/** Other flags for tests */
		bool installed, multi_installed, invert;
		bool slotted, multi_slot;
		bool overlay, obsolete;
		bool upgrade, world, world_both, worldset;
		LocalMode upgrade_local_mode;
		bool dup_versions, dup_versions_overlay;
		bool dup_packages, dup_packages_overlay;
		ExtendedVersion::Restrict restrictions;
		ExtendedVersion::Properties properties;

		std::set<Version::Overlay>
			*overlay_list, *overlay_only_list,
			*in_overlay_inst_list;

		std::set<Version::Overlay> *from_overlay_inst_list;
		std::vector<std::string> *from_foreign_overlay_inst_list;
		const char *portdir;

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

		static MatchField name2field(const std::string &p) throw(ExBasic);
		static MatchField get_matchfield(const char *p) throw(ExBasic);

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
		static Keywords::Redundant nowarn_cflags(const Package &p);
		static TestInstalled nowarn_installed(const Package &p);
};

#endif /* __PACKAGETEST_H__ */
