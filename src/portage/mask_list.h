// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__MASK_LIST_H__
#define EIX__MASK_LIST_H__ 1

#include <eixTk/likely.h>
#include <eixTk/ptr_list.h>
#include <portage/keywords.h>
#include <portage/mask.h>
#include <portage/package.h>

#include <map>
#include <string>

#include <cstddef>

class Version;

template<typename m_Type>
class MaskList
	: public std::map<std::string, std::map<std::string, eix::ptr_list<m_Type> > >
{
	public:
		typedef typename eix::ptr_list<m_Type>
			MskList;

		typedef typename std::map<std::string, MskList>
			CatList;

		typedef typename std::map<std::string, CatList>
			super;

		typedef typename super::iterator
			iterator;

		typedef typename super::const_iterator
			const_iterator;

		typedef typename CatList::iterator
			cat_iterator;

		typedef typename CatList::const_iterator
			const_cat_iterator;

		typedef typename MskList::iterator
			mask_iterator;

		typedef typename MskList::const_iterator
			const_mask_iterator;

		~MaskList()
		{
			for(iterator it(super::begin());
				likely(it != super::end()); ++it) {
				for(cat_iterator t(it->second.begin());
					likely(t != it->second.end()); ++t) {
					t->second.delete_and_clear();
				}
			}
		}

		MaskList()
		{ }

		MaskList(const MaskList<m_Type> &ori) : super()
		{ add(ori); }

		void add(const MaskList<m_Type> &ori)
		{
			for(const_iterator it(ori.begin());
				likely(it != ori.end()); ++it) {
				CatList &cl((*this)[it->first]);
				for(const_cat_iterator t(it->second.begin());
					likely(t != it->second.end()); ++t) {
					MskList &msk(cl[t->first]);
					for(const_mask_iterator m(t->second.begin());
						likely(m != t->second.end()); ++m) {
						msk.push_back(new m_Type(**m));
					}
				}
			}
		}

		void raise_empty(const std::string &s)
		{
			for(iterator it(super::begin());
				likely(it != super::end()); ++it) {
				for(cat_iterator t(it->second.begin());
					likely(t != it->second.end()); ++t) {
					for(mask_iterator m(t->second.begin());
						likely(m != t->second.end()); ++m) {
						m->raise_empty(s);
					}
				}
			}
		}

		/** @return true if actually something was added.
		    For speed reasons, we currently return always true
		    and add even unnecessarily. Note that this means that
		    we must remove more carefully. */

		bool add(m_Type *m)
		{
			(*this)[m->getCategory()][m->getName()].push_back(m);
			return true;
		}

		/** @return true if actually something was removed */
		bool remove(m_Type *m)
		{
			iterator it(super::find(m->getCategory()));
			if(it == super::end())
				return false;

			cat_iterator t(it->second.find(m->getName()));
			if(t == it->second.end())
				return false;

			bool deleted(false);
			mask_iterator mi(t->second.begin());
			while(mi != t->second.end()) {
				if(**mi == *m)
				{
					deleted = true;
					delete *mi;
					t->second.erase(mi);
					// mi is invalidated
					mi = t->second.begin();
				}
				else
					++mi;
			}
			if(t->second.empty())
			{
				// Now empty
				it->second.erase(t);
			}
			if(it->second.empty())
			{
				// Now empty
				this->erase(it);
			}
			return deleted;
		}

		void remove(const MaskList<m_Type> &l)
		{
			for(const_iterator it(l.begin()); likely(it != l.end()); ++it)
			{
				for(const_cat_iterator t(it->second.begin());
					likely(t != it->second.end()); ++t) {
					for(const_mask_iterator m(t->second.begin());
						likely(m != t->second.end()); ++m) {
							remove(*m);
					}
				}
			}
		}

#if 0
		void print() const
		{
			for(const_iterator it(super::begin());
				likely(it != super::end()); ++it) {
				for(const_cat_iterator t(it->second.begin());
					likely(t != it->second.end()); ++t) {
					std::cerr << it->first << "/" << t->first << std::endl;
					for(const_mask_iterator m(t->second.begin());
						likely(m != t->second.end()); ++m) {
//							m->print();
					}
				}
			}
		}
#endif
		const eix::ptr_list<m_Type> *get(const std::string &name, const std::string &category = SET_CATEGORY) const
		{
			const_iterator it(super::find(category));
			if(it == super::end())
				return NULL;

			const_cat_iterator t(it->second.find(name));
			if(t == it->second.end())
				return NULL;

			return &(t->second);
		}

		const eix::ptr_list<m_Type> *get(const Package *p) const
		{ return get(p->name, p->category); }

		const eix::ptr_list<m_Type> *get_split(const std::string &cat_name) const
		{
			std::string::size_type slash(cat_name.find("/"));
			if(slash == std::string::npos) {
				return NULL;
			}
			return get(cat_name.substr(slash + 1), cat_name.substr(0, slash));
		}

		void applyListItems(Package *p) const
		{
			const eix::ptr_list<m_Type> *l(get(p));
			if(l == NULL) {
				return;
			}
			for(const_mask_iterator m(l->begin());
				likely(m != l->end()); ++m) {
				m->applyItem(*p);
			}
		}

		void applyListSetItems(Version *v, const std::string &set_name) const
		{
			const eix::ptr_list<m_Type> *l(get(set_name));
			if(l == NULL) {
				return;
			}
			for(const_mask_iterator m(l->begin());
				likely(m != l->end()); ++m) {
				m->applyItem(v);
			}
		}

		// return true if some masks applied
		bool applyMasks(Package *p, Keywords::Redundant check = Keywords::RED_NOTHING) const;

		void applySetMasks(Version *v, const std::string &set_name) const;

		void applyVirtualMasks(Package *p) const;
};

#endif /* EIX__MASK_LIST_H__ */
