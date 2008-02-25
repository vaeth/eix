// vim:set et cinoptions=g0,t0,^-2,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __GUARD__COMPARE_H__
#define __GUARD__COMPARE_H__

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
}

#endif /* __GUARD__COMPARE_H__ */
