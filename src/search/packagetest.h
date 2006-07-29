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

#include <string.h>

#include <regex.h>
#include <fnmatch.h>

#include <global.h>

#include <portage/vardbpkg.h>
#include <portage/conf/portagesettings.h>
#include <database/package_reader.h>

#include <eixTk/exceptions.h>

#include <search/algorithms.h>

/** Test a package if it matches some criteria. */
class PackageTest {

	public:
		typedef char MatchField;
		static const MatchField NONE          , /* Search in name */
		                        NAME          , /* Search in name */
		                        DESCRIPTION   , /* Search in description */
		                        PROVIDE       , /* Search in provides */
		                        LICENSE       , /* Search in license */
		                        CATEGORY      , /* Search in category */
		                        CATEGORY_NAME , /* Search in category/name */
		                        HOMEPAGE      ; /* Search in homepage */

		/** Set default values. */
		PackageTest(VarDbPkg *vdb = NULL);

		void setAlgorithm(BaseAlgorithm *p)
		{ algorithm = std::auto_ptr<BaseAlgorithm>(p); }

		void setPattern(const char *p);

		bool match(PackageReader *pkg) const;

		/** Compile regex and/or calculate needs. */
		void finalize()
		{ calculateNeeds(); }

		void Installed()
		{ installed = !installed; }

		void DuplVersions()
		{ dup_versions = !dup_versions; }

		void ObsoleteCfg(PortageSettings &p, Keywords::Redundant red, Keywords::Redundant all, Keywords::Redundant ins)
		{
			redundant_flags = red;
			all_flags       = all;
			installed_flags = ins;
			portagesettings = &p;
			accept_keywords = p.getAcceptKeywords();
		}

		void Invert()
		{ invert = !invert; }

		MatchField operator |= (const MatchField m)
		{ return field |= m; }

	protected:

	private:
		/* What to match. */
		MatchField field;
		/** Lookup stuff about installed packages here. */
		VarDbPkg *vardbpkg;
		/* Test for this redundany. */
		Keywords::Redundant redundant_flags, all_flags, installed_flags;

		/** What we need to read so we can do our testing. */
		PackageReader::Attributes need;
		/** Our string matching algorithm. */
		std::auto_ptr<BaseAlgorithm> algorithm;
		bool installed, dup_versions, invert;

		/** Lookup stuff about obsolete user flags here (if non-null) */
		PortageSettings *portagesettings;
		Keywords accept_keywords;

		static MatchField name2field(const std::string &p) throw(ExBasic);
		static MatchField get_matchfield(const char *p) throw(ExBasic);

		bool stringMatch(Package *pkg) const;

		/** Get the Fetched-value that is required to determin */
		void calculateNeeds();

		bool have_redundant(const Package &p, Keywords::Redundant r) const;
};

#endif /* __PACKAGETEST_H__ */
