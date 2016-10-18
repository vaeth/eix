// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_PTR_SET_H_
#define SRC_EIXTK_PTR_SET_H_ 1

#include <set>

#include "eixTk/ptr_iterator.h"

namespace eix {
/**
A set that only stores pointers to type
**/
template<typename type> class ptr_set : public std::set<type> {
	public:
		using std::set<type>::begin;
		using std::set<type>::end;
		using std::set<type>::clear;

		/**
		Normal access iterator
		**/
		typedef ptr_iterator<typename std::set<type>::iterator> iterator;

		/**
		Constant access iterator
		**/
		typedef ptr_iterator<typename std::set<type>::const_iterator> const_iterator;

		/**
		Reverse access iterator
		**/
		typedef ptr_iterator<typename std::set<type>::reverse_iterator> reverse_iterator;

		/**
		Constant reverse access iterator
		**/
		typedef ptr_iterator<typename std::set<type>::const_reverse_iterator> const_reverse_iterator;

		void delete_and_clear() {
			delete_all(begin(), end());
			clear();
		}
};

}  // namespace eix

#endif  // SRC_EIXTK_PTR_SET_H_
