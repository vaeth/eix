// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__COMPARE_H__
#define EIX__COMPARE_H__ 1

#include <config.h>
#include <string>

/// eix namespace
namespace eix
{
	/// compare two objects.
	/// @return 0 if equal, 1 if left > right or -1 if left < right.
	template<typename T>
	short default_compare(const T& left, const T& right)
	{
		if (left == right)
			return 0;
		if(left < right)
			return -1;
		return 1;
	}

	/// numeric comparison.
	/// @note empty strings count a "0"
	inline short
	numeric_compare(const std::string& left, const std::string& right) ATTRIBUTE_PURE;
	inline short
	numeric_compare(const std::string& left, const std::string& right)
	{
		// strip leading 0's
		std::string::size_type lstart(left.find_first_not_of('0'));
		std::string::size_type rstart(right.find_first_not_of('0'));
		// Special cases: number is 0 or string is empty
		if (lstart == std::string::npos) {
			if(rstart == std::string::npos)
				return 0;
			return -1;
		}
		if (rstart == std::string::npos)
			return 1;

		// check if one is longer, that one would be bigger
		short size_result(default_compare(left.size() - lstart, right.size() - rstart));
		if (size_result)
			return size_result;
		// both strings have the same length, do string comparison
		return left.compare(lstart, std::string::npos, right, rstart, std::string::npos);
	}
}

#endif /* EIX__COMPARE_H__ */
