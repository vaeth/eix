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
#include <database/package_reader.h>

/** Recursively match packages agains a chain of tests. */
class Matchatom {

	private:
		std::auto_ptr<Matchatom> or_chain,  /**< OR'ed criteria */
		                    and_chain; /**< AND'ed criteria */
		std::auto_ptr<PackageTest>        test;      /**< Test for this criteria. */

	public:
		/** Create a new Matchatom, link it as OR to this criteria and set basic setting.
		 * @return the new criteria
		 * @note guaranted successfull */
		Matchatom *OR() {
			or_chain = std::auto_ptr<Matchatom>(new Matchatom());
			return or_chain.get();
		}

		/** Create a new Matchatom and link it as AND to this Matchatom.
		 * @return the new criteria
		 * @note guaranted successfull */
		Matchatom *AND() {
			and_chain = std::auto_ptr<Matchatom>(new Matchatom());
			return and_chain.get();
		}

		/** Set the PackageTest for this Matchatom. */
		void setTest(PackageTest *gtest) {
			test = std::auto_ptr<PackageTest>(gtest);
			test->finalize();
		}

		void finalize() throw(std::string) {
			// Please move along, nothin' to see here!
		}

		/** Check if this criteria matches.
		 * @param p Package to match
		 * @return true if match; else false */
		bool match(PackageReader *p) {
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
