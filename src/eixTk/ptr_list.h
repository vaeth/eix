// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __PTR_ITERATOR_H__
#define __PTR_ITERATOR_H__

#include <eixTk/iterator.h>

#include <list>

namespace eix {

	template<typename _Iterator>
	void delete_all(_Iterator b, _Iterator e)
	{
		for(; b != e; ++b)
			delete *b;
	}

	/// A list that only stores pointers to type.
	template<typename type>
	class ptr_list
		: virtual public std::list<type*>
	{
		public:
			using std::list<type*>::begin;
			using std::list<type*>::end;
			using std::list<type*>::clear;

			/// Normal access iterator.
			typedef ptr_iterator<
				typename std::list<type*>::iterator
			> iterator;

			/// Constant access iterator.
			typedef ptr_iterator<
				typename std::list<type*>::const_iterator
			> const_iterator;

			/// Reverse access iterator.
			typedef ptr_iterator<
				typename std::list<type*>::reverse_iterator
			> reverse_iterator;

			/// Constant reverse access iterator.
			typedef ptr_iterator<
				typename std::list<type*>::const_reverse_iterator
			> const_reverse_iterator;

			void delete_and_clear()
			{
				delete_all(begin(), end());
				clear();
			}
	};

}

#endif /* __PTR_ITERATOR_H__ */
