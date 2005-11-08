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
#include <portage/package.h>

#include <eixTk/exceptions.h>

#include <search/algorithms.h>

#define FNMATCH_FLAGS FNM_CASEFOLD

/* And M is for "match".
 * So those are things you can match .. */
class PackageTest {

	public:
		enum MatchField {
			NONE          = 0x00, /* Search in name */
			NAME          = 0x01, /* Search in name */
			DESCRIPTION   = 0x02, /* Search in description */
			LICENSE       = 0x04, /* Search in license */
			CATEGORY      = 0x08, /* Search in category */
			CATEGORY_NAME = 0x10, /* Search in category/name */
			HOMEPAGE      = 0x20, /* Search in homepage */
			DEFAULT       = 0x40
		} field; /**< What to match. */

		bool installed, dup_versions, invert;

		static MatchField name2field(const string &p) throw(ExBasic);
		static MatchField get_matchfield(const char *p) throw(ExBasic);

	private:
		Package::InputStatus    need;      /**< What we need to check. */
		auto_ptr<BaseAlgorithm> algorithm; /**< Our string matching algorithm. */

	public:
		void setAlgorithm(BaseAlgorithm *p) {
			algorithm = auto_ptr<BaseAlgorithm>(p);
		}

		void setPattern(const char *p) {
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

	protected:
		/** Get the Fetched-value that is required to determin */
		void calculateNeeds();

		/** Return true if pkg matches test. */
		bool stringMatch(Package *pkg) throw(ExBasic){
			pkg->readNeeded(need);

			switch(field) {
				case NAME:
					return (*algorithm)(pkg->name.c_str(), pkg);
				case DESCRIPTION:
					return (*algorithm)(pkg->desc.c_str(), pkg);
				case LICENSE: 
					return (*algorithm)(pkg->licenses.c_str(), pkg);
				case CATEGORY:
					return (*algorithm)(pkg->category.c_str(), pkg);
				case CATEGORY_NAME:
					return (*algorithm)((pkg->category + "/" + pkg->name).c_str(), pkg);
				case HOMEPAGE:
					return (*algorithm)(pkg->homepage.c_str(), pkg);
				default:
					THROW("Hu? - I don't know what field I shall match!");
			}
		}

	public:
		VarDbPkg *vardbpkg; /**< Lookup stuff about installed packages here. */

		/** Set default values. */
		PackageTest(VarDbPkg *vdb = NULL) {
			vardbpkg = vdb;
			field    = NONE;
			need     = Package::NONE;
			invert   = installed = dup_versions = false;
		}

		bool match(Package *pkg) {
			bool is_match = true;
			if(algorithm.get() != NULL) {
				is_match = stringMatch(pkg);
			}

			/* Honour the C_O_INSTALLED, C_O_DUP_VERSIONS and the C_O_INVERT flags. */
			if(installed && is_match) {
				is_match = vardbpkg->isInstalled(pkg);
			}

			if(dup_versions && is_match)
				is_match = pkg->have_duplicate_versions;

			return (invert ? !is_match : is_match);
		}

		/** Compile regex and/or calculate needs. */
		void finalize() {
			calculateNeeds();
		}
};
#endif /* __PACKAGETEST_H__ */
