// vim:set et cinoptions=g0,t0,^-2,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __GUARD__COMPARE_H__
#define __GUARD__COMPARE_H__

#include <string>

/// eix namespace
namespace eix
{
  /// compare two objects.
  /// @return 0 if equal, 1 if left > right or -1 if left < right.
  template<typename T>
      int default_compare(const T& left, const T& right) 
      {
          if (left == right)
              return 0;
          else if(left < right)
              return -1;
          else
              return 1;
      }

  /// numeric comparison.
  /// @note empty strings count a "0"
  static inline int
  numeric_compare(const std::string& left, const std::string& right)
  {
      // strip leading 0's
      const std::string::size_type lstart = left.find_first_not_of("0");
      const std::string::size_type rstart = right.find_first_not_of("0");

      // check if one is longer, that one would be bigger
      const int size_result = default_compare(left.size() - (lstart == std::string::npos ? 0 : lstart),
                                              right.size() - (rstart == std::string::npos ? 0 : rstart));
      if (size_result)
          return size_result;
      // both strings have the same length, do string comparison
      return left.compare(lstart, std::string::npos, right, rstart, std::string::npos);
  }
}

#endif /* __GUARD__COMPARE_H__ */
