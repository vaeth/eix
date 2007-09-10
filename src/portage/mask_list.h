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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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

#ifndef __MASK_LIST_H__
#define __MASK_LIST_H__

#include <map>
#include <string>

#include <eixTk/ptr_list.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/mask.h>

template<typename _type>
class MaskList
	: public std::map<std::string, std::map<std::string, eix::ptr_list<_type> > >
{
	public:
		typedef typename eix::ptr_list<_type>
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
			for(iterator it = super::begin(); it != super::end(); ++it)
			{
				for(cat_iterator t = it->second.begin();
					t != it->second.end();
					++t)
				{
					t->second.delete_and_clear();
				}
			}
		}

		MaskList()
		{ }

		MaskList(const MaskList<_type> &ori) : super()
		{ add(ori); }

		void add(const MaskList<_type> &ori)
		{
			for(const_iterator it = ori.begin(); it != ori.end(); ++it)
			{
				CatList &cl = (*this)[it->first];
				for(const_cat_iterator t = it->second.begin();
					t != it->second.end();
					++t)
				{
					MskList &msk = cl[t->first];
					for(const_mask_iterator m = t->second.begin();
						m != t->second.end(); ++m) {
						msk.push_back(new _type(**m));
					}
				}
			}
		}

		/** @return true if actually something was added.
		    For speed reasons, we currently return always true
		    and add even unnecessarily. Note that this means that
		    we must remove more carefully. */

		bool add(_type *m)
		{
			(*this)[m->getCategory()][m->getName()].push_back(m);
			return true;
		}

		/** @return true if actually something was removed */
		bool remove(_type *m)
		{
			iterator it = super::find(m->getCategory());
			if(it == super::end())
				return false;

			cat_iterator t = it->second.find(m->getName());
			if(t == it->second.end())
				return false;

			bool deleted = false;
			mask_iterator mi = t->second.begin();
			while(mi != t->second.end())
			{
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

		void remove(const MaskList<_type> &l)
		{
			for(const_iterator it = l.begin(); it != l.end(); ++it)
			{
				for(const_cat_iterator t = it->second.begin();
					t != it->second.end();
					++t)
				{
					for(const_mask_iterator m = t->second.begin();
						m != t->second.end(); ++m) {
							remove(*m);
					}
				}
			}
		}

#if 0
		void print() const
		{
			for(const_iterator it = super::begin(); it != super::end(); ++it)
			{
				for(const_cat_iterator t = it->second.begin();
					t != it->second.end();
					++t)
				{
					for(const_mask_iterator m = t->second.begin();
						m != t->second.end(); ++m) {
							m->print();
					}
				}
			}
		}
#endif

		const eix::ptr_list<_type> *get(const Package *p) const
		{
			const_iterator it = super::find(p->category);
			if(it == super::end())
				return NULL;

			const_cat_iterator t = it->second.find(p->name);
			if(t == it->second.end())
				return NULL;

			return &(t->second);
		}

		// return true if some masks applied
		bool applyMasks(Package *p, Keywords::Redundant check = Keywords::RED_NOTHING) const
		{
			const eix::ptr_list<_type> *l = get(p);
			if(l == NULL)
				return false;

			bool rvalue = false;
			bool had_mask = false;
			bool had_unmask = false;
			for(const_mask_iterator m = l->begin();
				m != l->end();
				++m)
			{
				rvalue = 1;
				m->checkMask(*p, false, false, check);
				switch(m->get_type())
				{
					case Mask::maskMask:
						had_mask=true;
						break;
					case Mask::maskUnmask:
						had_unmask=true;
						break;
					default:
						break;
				}
			}
			if(!(check & Keywords::RED_MASK))
				had_mask = false;
			if(!(check & Keywords::RED_UNMASK))
				had_unmask = false;
			if(had_mask || had_unmask)
			{
				for(Package::iterator i = p->begin();
					i != p->end(); ++i)
				{
					if(had_mask)
					{
						if(!i->was_masked())
							i->set_redundant(Keywords::RED_MASK);
					}
					if(had_unmask)
					{
						if(!i->was_unmasked())
							i->set_redundant(Keywords::RED_UNMASK);
					}
				}
			}
			return rvalue;
		}
};

#endif /* __MASK_LIST_H__ */
