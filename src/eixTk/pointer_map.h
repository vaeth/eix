// vim:set et cinoptions=g0,t0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __GUARD__PROP_MAP_H__
#define __GUARD__PROP_MAP_H__

#include <eixTk/iterator.h>

#include <map>

namespace eix
{
    /// std::less for pointers to objects.
    template<typename T>
    struct ptr_less
        : public std::binary_function<const T*, const T*, bool>
    {
        bool operator()(const T* const& arg_1, const T* const& arg_2) const
        { return *arg_1 < *arg_2; }
    };

    /** Container that provides fast access to objects based on a constant key.
     */
    template<typename _Key, typename _Value, typename _Adaptor>
    class pointer_map
    {
    public:
        typedef _Adaptor     adaptor_type;
        typedef _Key         key_type;
        typedef _Value       value_type;
        typedef unsigned int size_type;

        typedef std::map<const key_type*, value_type*, ptr_less<key_type> >
            tree_type;

        typedef second_ptr_iter<typename tree_type::iterator, value_type>
            iterator;

        typedef second_ptr_iter<typename tree_type::const_iterator, const value_type>
            const_iterator;

        pointer_map() { }

        virtual ~pointer_map() { }

        iterator begin()
        { return iterator(m_tree.begin()); }

        const_iterator begin() const
        { return const_iterator(m_tree.begin()); }

        iterator end()
        { return iterator(m_tree.end()); }

        const_iterator end() const
        { return const_iterator(m_tree.end()); }

        bool empty() const
        { return m_tree.empty(); }

        size_type size() const
        { return m_tree.size(); }

        iterator find(const key_type& key)
        { return iterator(m_tree.find(&key)); }

        const_iterator find(const key_type& key) const
        { return const_iterator(m_tree.find(&key)); }

        value_type* insert(value_type* value)
        {
            m_tree.insert(std::make_pair(m_adaptor(value), value));
            return value;
        }

        void erase(const iterator& pos)
        { m_tree.erase(pos); }

        value_type* get(const key_type& key)
        {
            typename tree_type::iterator it = m_tree.find(&key);
            if (it != m_tree.end()) {
                return it->second;
            }
            return 0;
        }

        const value_type* get(const key_type& key) const
        {
            typename tree_type::const_iterator it = m_tree.find(&key);
            if (it != m_tree.end()) {
                return it->second;
            }
            return 0;
        }

        value_type& operator[](const key_type & key)
        {
            typename tree_type::iterator it = m_tree.find(&key);

            if (it != m_tree.end())
                return *it->second;

            return *insert(new value_type(key));
        }

    private:
        /// adaptor that will give us access to the key_types inside the
        /// value_types.
        adaptor_type m_adaptor;

        /// actually used map
        tree_type    m_tree;
    };
}

#endif /* __GUARD__PROP_MAP_H__ */
