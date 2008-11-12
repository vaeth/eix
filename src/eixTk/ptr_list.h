// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__PTR_ITERATOR_H__)
#define EIX__PTR_ITERATOR_H__

#include <list>

namespace eix {

	template<typename m_Iterator>
	void delete_all(m_Iterator b, m_Iterator e)
	{
		for(; b != e; ++b)
			delete *b;
	}

	/// An iterator type to iterate through a container containing pointers of the
	// given data type. The special thing is the operator-> returns the same as the operator*.
	// Taken from the obby-project (http://darcs.0x539.de/libobby) and extended.
	template<typename base_iterator>
	class ptr_iterator
		: public base_iterator
	{
		public:
			ptr_iterator()
				: base_iterator()
			{ }

			ptr_iterator(const base_iterator& iter)
				: base_iterator(iter)
			{ }

			ptr_iterator& operator=(const base_iterator& iter)
			{ return static_cast<ptr_iterator&>( base_iterator::operator=(iter)); }

			typename base_iterator::reference operator->()
			{ return *base_iterator::operator->(); }
	};

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

#endif /* EIX__PTR_ITERATOR_H__ */
