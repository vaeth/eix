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

#ifndef __CRITERIA_H__
#define __CRITERIA_H__

#include <search/packagetest.h>
#include <portage/package.h>

using namespace std;

#if 0
/* I'am sure this could be made by using function-pointers .. */
#define CYCLE_MATCH(_flags, _data, _pkg) do { \
	if((_flags & NAME) && MATCH_WITH(_data, _pkg->name.c_str(), _pkg)) \
	return true; \
	if((_flags & DESCRIPTION) && MATCH_WITH(_data, _pkg->desc.c_str(), _pkg)) \
	return true; \
	if((_flags & LICENSE) && MATCH_WITH(_data, _pkg->licenses.c_str(), _pkg)) \
	return true; \
	if((_flags & CATEGORY) && MATCH_WITH(_data, (_pkg->category).c_str(), _pkg)) \
	return true; \
	if((_flags & CATEGORY_NAME) && MATCH_WITH(_data, (_pkg->category + "/" + _pkg->name).c_str(), _pkg)) \
	return true; \
	if((_flags & HOMEPAGE) && MATCH_WITH(_data, _pkg->homepage.c_str(), _pkg)) \
	return true; \
} while(0)
#endif

/** Recursively match packages agains a chain of tests. */
class Matchatom {

	private:
		auto_ptr<Matchatom> or_chain,  /**< OR'ed criteria */
			                      and_chain; /**< AND'ed criteria */
		auto_ptr<PackageTest>        test;      /**< Test for this criteria. */

	public:
		/** Create a new Matchatom, link it as OR to this criteria and set basic setting.
		 * @return the new criteria
		 * @note guaranted successfull */
		Matchatom *OR() {
			or_chain = auto_ptr<Matchatom>(new Matchatom());
			return or_chain.get();
		}

		/** Create a new Matchatom and link it as AND to this Matchatom.
		 * @return the new criteria
		 * @note guaranted successfull */
		Matchatom *AND() {
			and_chain = auto_ptr<Matchatom>(new Matchatom());
			return and_chain.get();
		}

		/** Set the PackageTest for this Matchatom. */
		void setTest(PackageTest *gtest) {
			test = auto_ptr<PackageTest>(gtest);
			test->finalize();
		}

		void finalize() throw(string) {
			// Please move along, nothin' to see here!
		}

		/** Check if this criteria matches.
		 * @param p Package to match
		 * @return true if match; else false */
		bool match(Package *p) {
			bool is_match = test->match(p);

			/* Check AND/OR'ed criteria */
			if(and_chain.get() && is_match)
				is_match = and_chain->match(p);
			if(or_chain.get() && !is_match)
				is_match = or_chain->match(p);

			return is_match;
		}
};

#endif /* __CRITERIA_H__ */
