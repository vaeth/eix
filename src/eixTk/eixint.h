// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_EIXINT_H_
#define SRC_EIXTK_EIXINT_H_ 1

#include <sys/types.h>

// include "eixTk/eixint.h" make check_includes happy

namespace eix {
	typedef unsigned char UChar;
	typedef signed char TinySigned;

	typedef UChar TinyUnsigned;
	typedef TinySigned SignedBool;

	typedef size_t UNumber;

	typedef UNumber Catsize;
	typedef UNumber Versize;
	typedef UNumber Treesize;

	typedef off_t OffsetType;

	inline static eix::SignedBool
	toSignedBool(int a) {
		if(a == 0) {
			return 0;
		} else {
			return ((a < 0) ? -1 : 1);
		}
	}
}  // namespace eix

#endif  // SRC_EIXTK_EIXINT_H_
