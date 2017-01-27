// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_PTR_CONTAINER_H_
#define SRC_EIXTK_PTR_CONTAINER_H_ 1

#include "eixTk/ptr_iterator.h"

// Make check_includes.sh happy: include "eixTk/ptr_container.h"

namespace eix {
/**
A set that only stores pointers to type
**/
template<class type> class ptr_container : public type {
	public:
		using type::begin;
		using type::end;
		using type::clear;

		/**
		Normal access iterator
		**/
		typedef ptr_iterator<typename type::iterator> iterator;

		/**
		Constant access iterator
		**/
		typedef ptr_iterator<typename type::const_iterator> const_iterator;

		/**
		Reverse access iterator
		**/
		typedef ptr_iterator<typename type::reverse_iterator> reverse_iterator;

		/**
		Constant reverse access iterator
		**/
		typedef ptr_iterator<typename type::const_reverse_iterator> const_reverse_iterator;

		void delete_and_clear() {
			delete_all(begin(), end());
			clear();
		}
};

}  // namespace eix

#endif  // SRC_EIXTK_PTR_CONTAINER_H_
