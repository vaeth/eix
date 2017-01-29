// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_PTR_ITERATOR_H_
#define SRC_EIXTK_PTR_ITERATOR_H_ 1

#include <config.h>

#include <iterator>

namespace eix {

template<typename m_Iterator> inline static void delete_all(m_Iterator b, m_Iterator e) {
	for(; b != e; ++b) {
		delete *b;
	}
}

/**
An iterator type to iterate through a container containing pointers of the
given data type. The special thing is the operator-> returns the same as the operator*.
Taken from the obby-project (http://darcs.0x539.de/libobby) and extended.
**/
template<typename base_iterator> class ptr_iterator : public base_iterator {
	public:
		ptr_iterator() : base_iterator() {
		}

		explicit ptr_iterator(const base_iterator& iter) : base_iterator(iter) {
		}

		ptr_iterator& operator=(const base_iterator& iter) {
			return static_cast<ptr_iterator&>( base_iterator::operator=(iter));
		}

		typename base_iterator::reference operator->() {
			return *base_iterator::operator->();
		}
};

}  // namespace eix

#endif  // SRC_EIXTK_PTR_ITERATOR_H_
