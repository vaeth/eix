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

#ifndef __PACKAGETEST_H__
#define __PACKAGETEST_H__

#include <set>
#include <string>

#include <regex.h>
#include <fnmatch.h>

#include <global.h>

#include <portage/vardbpkg.h>
#include <portage/version.h>
#include <portage/conf/portagesettings.h>
#include <database/package_reader.h>

#include <eixTk/exceptions.h>

#include <search/algorithms.h>
#include <search/redundancy.h>

class DBHeader;

/** Test a package if it matches some criteria. */
class PackageTest {

	public:
		typedef uint16_t MatchField;
		static const MatchField NONE          , /**< Search in name */
		                        NAME          , /**< Search in name */
		                        DESCRIPTION   , /**< Search in description */
		                        PROVIDE       , /**< Search in provides */
		                        LICENSE       , /**< Search in license */
		                        CATEGORY      , /**< Search in category */
		                        CATEGORY_NAME , /**< Search in category/name */
		                        HOMEPAGE      , /**< Search in homepage */
		                        IUSE          , /**< Search in iuse */
		                        USE_ENABLED   , /**< Search in enabled  useflags of installed packages */
		                        USE_DISABLED  ; /**< Search in disabled useflags of installed packages */

		typedef uint8_t TestInstalled;
		static const TestInstalled
					INS_NONE,
					INS_NONEXISTENT, /**< Test for nonexistent installed packages */
					INS_OVERLAY,     /**< Test for nonexistent overlays of installed packages */
					INS_MASKED;      /**< Test for masked installed packages */

		typedef uint8_t TestStability;
		static const TestStability
					STABLE_NONE,
					STABLE_FULL,     /**< Test for full stability */
					STABLE_NONMASKED,/**< Test for non-masked packages */
					STABLE_SYSTEM;   /**< Test for system packages */

		/** Set default values. */
		PackageTest(VarDbPkg &vdb, PortageSettings &p, const DBHeader &dbheader);

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
#if defined(USE_BZLIB)
			if(from_overlay_inst_list) {
				delete from_overlay_inst_list;
				from_overlay_inst_list = NULL;
			}
			if(from_foreign_overlay_inst_list) {
				delete from_foreign_overlay_inst_list;
				from_foreign_overlay_inst_list = NULL;
			}
#endif
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

		void Update(bool local_config)
		{  update = true; update_matches_local = local_config; }

		void Stability(TestStability require)
		{  stability |= require; }

		void Overlay()
		{ overlay = true; }

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

#if defined(USE_BZLIB)
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
#endif

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
		bool update, update_matches_local;
		bool dup_versions, dup_versions_overlay;
		bool dup_packages, dup_packages_overlay;

		std::set<Version::Overlay>
			*overlay_list, *overlay_only_list,
			*in_overlay_inst_list;

#if defined(USE_BZLIB)
		std::set<Version::Overlay> *from_overlay_inst_list;
		std::vector<std::string> *from_foreign_overlay_inst_list;
		const char *portdir;
#endif

		/** Lookup stuff about user flags here. */
		PortageSettings *portagesettings;
		/* Test for this redundancy: */
		Keywords::Redundant redundant_flags;
		RedAtom first_test, second_test;
		TestInstalled test_installed;
		TestStability stability;

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
};

#endif /* __PACKAGETEST_H__ */
