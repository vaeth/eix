// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_COMPARE_H_
#define SRC_EIXTK_COMPARE_H_ 1

#include <string>

#include "eixTk/eixint.h"

/// eix namespace
namespace eix {
	/// compare two objects.
	/// @return 0 if equal, 1 if left > right or -1 if left < right.
	template<typename T>
	inline static eix::SignedBool
	default_compare(const T& left, const T& right)
	{
		if(left == right) {
			return 0;
		}
		return ((left < right) ? -1 : 1);
	}

	/// numeric comparison.
	/// @note empty strings count a "0"
	eix::SignedBool numeric_compare(const std::string& left, const std::string& right) ATTRIBUTE_PURE;
}

#endif  // SRC_EIXTK_COMPARE_H_
