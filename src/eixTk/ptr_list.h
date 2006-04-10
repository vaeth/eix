/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __PTR_ITERATOR_H__
#define __PTR_ITERATOR_H__

#include <list>

namespace eix {

	template<typename _Iterator>
	void delete_all(_Iterator b, _Iterator e)
	{
		for(; b != e; ++b)
			delete *b;
	}

	/// An iterator type to iterate through a container containing pointers of the
	// given data type. The special thing is the operator-> returns the same as the operator*.
	// Taken from the obby-project (http://darcs.0x539.de/libobby) and extended.
	template<typename type, typename container, typename base_iterator>
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

			type* operator->()
			{ return *base_iterator::operator->(); }

			const type* operator->() const
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
				type,
				std::list<type*>,
				typename std::list<type*>::iterator
			> iterator;

			/// Constant access iterator.
			typedef ptr_iterator<
				type,
				std::list<type*>,
				typename std::list<type*>::const_iterator
			> const_iterator;

			/// Reverse access iterator.
			typedef ptr_iterator<
				type,
				std::list<type*>,
				typename std::list<type*>::reverse_iterator
			> reverse_iterator;

			/// Constant reverse access iterator.
			typedef ptr_iterator<
				type,
				std::list<type*>,
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
