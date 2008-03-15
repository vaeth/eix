// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>

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
