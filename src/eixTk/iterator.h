// vim:set et cinoptions=g0,t0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __GUARD__ITERATOR_H__
#define __GUARD__ITERATOR_H__

#include <iterator>

namespace eix
{
    /** An iterator type to iterate through a container containing pointers of
     * the given data type. The special thing is the operator-> returns the same
     * as the operator*.
     * Taken from the obby-project (http://darcs.0x539.de/libobby) and extended.
     */
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
        { return static_cast<ptr_iterator&>(base_iterator::operator=(iter)); }

        typename base_iterator::reference operator->()
        { return *base_iterator::operator->(); }
    };


    /** Wrapper for map iterators that simulates a normal iterator over the
     * "second" attribute.
     */
    template<typename base_iterator, typename T>
    class second_ptr_iter 
    : public base_iterator
    {
    public:
        second_ptr_iter()
            : base_iterator()
        { }

        second_ptr_iter(const base_iterator& iter)
            : base_iterator(iter)
        { }

        second_ptr_iter& operator=(const base_iterator& iter)
        { return static_cast<second_ptr_iter&>( base_iterator::operator=(iter)); }

        T* operator*()
        { return base_iterator::operator->()->second; }

        T* operator->()
        { return base_iterator::operator->()->second; }
    };
}

#endif /* __GUARD__ITERATOR_H__ */
