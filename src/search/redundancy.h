// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_SEARCH_REDUNDANCY_H_
#define SRC_SEARCH_REDUNDANCY_H_ 1

#include "portage/keywords.h"

class RedAtom {
	public:
		Keywords::Redundant
			red,   /**< Do we search for the redundancy  */
			all,   /**< Test for some or all occurrences */
			spc,   /**< Test everywhere or only (un-)installed */
			ins,   /**< If spc: Only installed or un-installed */
			only,  /**< Test only if there is none/some version installed */
			oins;  /**< If only: Only installed or not installed */
		RedAtom() :
			red(Keywords::RED_NOTHING),
			all(Keywords::RED_NOTHING),
			spc(Keywords::RED_NOTHING),
			ins(Keywords::RED_NOTHING),
			only(Keywords::RED_NOTHING),
			oins(Keywords::RED_NOTHING) {
		}
};



#endif  // SRC_SEARCH_REDUNDANCY_H_
