// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_FORWARD_LIST_H_
#define SRC_EIXTK_FORWARD_LIST_H_ 1

#include <config.h>

// check_includes: include "eixTk/forward_list.h"

#ifdef HAVE_FORWARD_LIST
#include <forward_list>
namespace eix {
	using std::forward_list;
}  // namespace eix
#else

// This is not a full implementation of forward_list; only some things we use

#include "eixTk/likely.h"
#include "eixTk/null.h"
namespace eix {
template<class T> class forward_list_node {
	public:
		T value;
		forward_list_node *node;

		explicit forward_list_node(const T& val)
			: value(val) {
		}
};

template<class T> class forward_list_iterator {
	public:
		forward_list_node<T> *m_node;

		typedef forward_list_iterator<T> self;
		typedef T value_type;
		typedef T& reference;
		typedef T *pointer;

		forward_list_iterator() {
		}

		explicit forward_list_iterator(forward_list_node<T> *node)
			: m_node(node) {
		}

		reference operator*() const {
			return m_node->value;
		}

		pointer operator->() const {
			return &(m_node->value);
		}

		self operator++() {
			m_node = m_node->node;
			return *this;
		}

		self operator++(int) {
			self tmp(*this);
			m_node = m_node->node;
			return tmp;
		}

		bool operator==(const self& x) {
			return (m_node == x.m_node);
		}

		bool operator!=(const self& x) {
			return (m_node != x.m_node);
		}
};

template<class T> class forward_list_const_iterator {
	public:
		const forward_list_node<T> *m_node;

		typedef forward_list_const_iterator<T> self;
		typedef T value_type;
		typedef const T& const_reference;
		typedef const T *const_pointer;

		forward_list_const_iterator() {
		}

		explicit forward_list_const_iterator(forward_list_node<T> *node)
			: m_node(node) {
		}

		const_reference operator*() const {
			return m_node->value;
		}

		const_pointer operator->() const {
			return &(m_node->value);
		}

		self operator++() {
			m_node = m_node->node;
			return *this;
		}

		self operator++(int) {
			self tmp(*this);
			m_node = m_node->node;
			return tmp;
		}

		bool operator==(const self& x) {
			return (m_node == x.m_node);
		}

		bool operator!=(const self& x) {
			return (m_node != x.m_node);
		}
};

template<class T> class forward_list {
	private:
		typedef forward_list_node<T> node_type;
		typedef forward_list_node<T> *node_ptr;
		node_ptr head;

	public:
		typedef T value_type;
		typedef forward_list_iterator<T> iterator;
		typedef forward_list_const_iterator<T> const_iterator;

		forward_list()
			: head(NULLPTR) {
		}

		~forward_list() {
			for(node_ptr curr(head); likely(curr != NULLPTR); ) {
				node_ptr next(curr->node);
				delete curr;
				curr = next;
			}
		}

		iterator begin() {
			return iterator(head);
		}

		const_iterator begin() const {
			return const_iterator(head);
		}

		iterator before_begin() {
			return iterator(NULLPTR);
		}

		const_iterator before_begin() const {
			return const_iterator(NULLPTR);
		}

		iterator end() {
			return iterator(NULLPTR);
		}

		const_iterator end() const {
			return const_iterator(NULLPTR);
		}

		void insert_after(iterator iterator, const value_type& value) {
			node_ptr new_node(new node_type(value));
			node_ptr parent(iterator.m_node);
			if(likely(parent != NULLPTR)) {
				new_node->node = parent->node;
				parent->node = new_node;
			} else {
				new_node->node = head;
				head = new_node;
			}
		}
};
}  // namespace eix
#endif

#endif  // SRC_EIXTK_FORWARD_LIST_H_
