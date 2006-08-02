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

template<typename _type>
class MaskList
	: public std::map<std::string, std::map<std::string, eix::ptr_list<_type> > >
{
	public:
		typedef typename std::map<std::string, std::map<std::string, eix::ptr_list<_type> > >
			super;

		typedef typename std::map<std::string, std::map<std::string, eix::ptr_list<_type> > >::iterator
			iterator;

		typedef typename std::map<std::string, std::map<std::string, eix::ptr_list<_type> > >::const_iterator
			const_iterator;

		typedef typename std::map<std::string,eix::ptr_list<_type> >::iterator
			cat_iterator;

		typedef typename std::map<std::string,eix::ptr_list<_type> >::const_iterator
			const_cat_iterator;

		typedef typename eix::ptr_list<_type>::iterator
			mask_iterator;

		typedef typename eix::ptr_list<_type>::const_iterator
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


		void add(_type *m)
		{ (*this)[m->getCategory()][m->getName()].push_back(m); }

		void remove(_type *m)
		{
			iterator it = super::find(m->getCategory());
			if(it == super::end())
				return;

			cat_iterator t = it->second.find(m->getName());
			if(t == it->second.end())
				return;

			for(mask_iterator mi = t->second.begin();
				mi != t->second.end();
				++mi)
			{
				if(**mi == *m)
				{
					delete *mi;
					t->second.erase(mi);
					break;
				}
			}
			if(t->second.size() == 0)
			{
				// Now empty
				it->second.erase(t);
			}
			if(it->second.size() == 0)
			{
				// Now empty
				this->erase(it);
			}
		}

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
