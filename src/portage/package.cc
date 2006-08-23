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

#include "package.h"

#include <portage/version.h>
#include <portage/vardbpkg.h>

using namespace std;

Package::~Package()
{
	delete_and_clear();
}

const Package::Duplicates
	Package::DUP_NONE     = 0x00,
	Package::DUP_SOME     = 0x01,
	Package::DUP_OVERLAYS = 0x03;

Version *
VersionList::best() const
{
	for(const_reverse_iterator ri = rbegin();
		ri != rend(); ++ri)
	{
		if((*ri)->isStable() && (!(*ri)->isHardMasked()))
			return *ri;
	}
	return NULL;
}

void
SlotList::push_back_largest(Version *version)
{
	const char *name = (version->slot).c_str();
	for(iterator it = begin(); it != end(); ++it)
	{
		if(strcmp(name, it->slot()) == 0)
		{
			(it->version_list()).push_back(version);
			return;
		}
	}
	push_back(SlotVersions(name, version));
}

const VersionList *
SlotList::operator [] (const char *s) const
{
	for(const_iterator it = begin(); it != end(); ++it)
	{
		if(strcmp(s, it->slot()) == 0)
			return &(it->const_version_list());
	}
	return NULL;
}


/** Check if a package has duplicated versions. */
void Package::checkDuplicates(Version *version)
{
	if(have_duplicate_versions == DUP_OVERLAYS)
		return;
	bool no_overlay = !(version->overlay_key);
	if(no_overlay && (have_duplicate_versions == DUP_SOME))
		return;
	for(iterator i = begin(); i != end(); ++i)
	{
		if(dynamic_cast<BasicVersion&>(**i) == dynamic_cast<BasicVersion&>(*version))
		{
			if(no_overlay)
			{
				have_duplicate_versions = DUP_SOME;
				return;
			}
			if(i->overlay_key)
			{
				have_duplicate_versions = DUP_OVERLAYS;
				return;
			}
			have_duplicate_versions = DUP_SOME;
		}
	}
}

void
Package::sortedPushBack(Version *v)
{
	for(iterator i = begin(); i != end(); ++i)
	{
		if(*v < **i)
		{
			insert(i, v);
			return;
		}
	}
	push_back(v);
}

/** Finishes addVersionStart() after the remaining data
    have been filled */
void Package::addVersionFinalize(Version *version)
{
	Version::Overlay key = version->overlay_key;

	if(version->slot == "0")
		version->slot = "";

	/* This guarantees that we pushed our first version */
	if(size() != 1) {
		if(largest_overlay != key)
		{
			have_same_overlay_key = false;
			if(largest_overlay && key)
				at_least_two_overlays = true;
			if(largest_overlay < key)
				largest_overlay = key;
		}
		if(is_system_package) {
			is_system_package = version->isSystem();
		}
	}
	else {
		largest_overlay       = key;
		have_nontrivial_slots = false;
		is_system_package     = version->isSystem();
	}
	if((version->slot).length())
		have_nontrivial_slots = true;
	// We must recalculate the complete slotlist after each modification.
	// The reason is that the pointers might go into nirvana, because
	// a push_back might move the whole list.
	calculate_slotlist();
}

Version *
Package::best() const
{
	Version *ret = NULL;
	for(const_reverse_iterator ri = rbegin();
		ri != rend();
		++ri)
	{
		if(ri->isStable() && !ri->isHardMasked())
		{
			ret = *ri;
			break;
		}
	}
	return ret;
}

void
Package::calculate_slotlist()
{
	slotlist.clear();
	for(iterator it = begin(); it != end(); ++it)
		slotlist.push_back_largest(*it);
}


Version *
Package::best_slot(const char *slot_name) const
{
	const VersionList *vl = slotlist[slot_name];
	if(!vl)
		return NULL;
	return vl->best();
}

void
Package::best_slots(vector<Version*> &l) const
{
	l.clear();
	for(SlotList::const_iterator sit = slotlist.begin();
		sit != slotlist.end(); ++sit)
	{
		Version *p = (sit->const_version_list()).best();
		if(p)
			l.push_back(p);
	}
}

const char *
Package::slotname(const BasicVersion &v) const
{
	for(const_iterator i = begin(); i != end(); i++)
	{
		if(**i == v)
			return (i->slot).c_str();
	}
	return NULL;
}

/** Compare best_slots() versions of p and q.
    return value:
	 1: p has a bigger value than q
	-2: p has no bigger value than q, but overlays are different
	 0: else */
inline
int compare_slots_sub(const Package &p, const Package &q)
{
	int ret = 0;
	for(SlotList::const_iterator it = p.slotlist.begin();
		it != p.slotlist.end(); ++it)
	{
		Version *p_best = (it->const_version_list()).best();
		if(!p_best)
			continue;
		Version *q_best = q.best_slot(it->slot());
		if(!q_best)
			return 1;
		if(*p_best > *q_best)
			return 1;
		if(*p_best < *q_best)
			return -1;
		if(p_best->overlay_key != q_best->overlay_key)
			ret = -2;
	}
	return ret;
}

/** Compare best_slot() versions with that of p.
    return value:
	 0: Everything matches
	 1: p has a smaller value and no larger
	-1: p has a larger  value and no smaller
	 2: p has a smaller and a larger value
	-2: Everything matches, but overlays are different */
int
Package::compare_slots(const Package &p) const
{
	int first  = compare_slots_sub(*this, p);
	int second = compare_slots_sub(p, *this);
	if(first > 0)
	{
		if(second > 0)
			return 2;
		return 1;
	}
	if(second > 0)
		return -1;
	if(first || second)
		return -2;
	return 0;
}

/** Compare best_slots() versions with that installed in v.
    if v is NULL, it is assumed that none is installed.
    return value:
	 0: All installed versions are best and
	    (unless only_installed) one is installed
	 1: upgrade   necessary but no downgrade
	-1: downgrade necessary but no upgrade
	 2: upgrade and downgrade necessary
	-2: (if only_installed) nothing is installed,
	    and nothing can be installed
	 3: (if only_installed) nothing is installed,
	    but one can be installed */
int
Package::compare_slots(VarDbPkg *v, bool only_installed) const
{
	vector<BasicVersion> *ins = NULL;
	if(v)
		ins = v->getInstalledVector(*this);
	if(ins)
		if(!ins->size())
			ins = NULL;
	if(!ins)
	{
		if(only_installed)
			return 0;
		if(best())
			return 3;
		return -2;
	}
	bool downgrade = false;
	bool upgrade = false;
	for(vector<BasicVersion>::const_iterator it = ins->begin();
		it != ins->end() ; ++it)
	{
		const char *name;
		if(v->have_slots())
			name = (it->slot).c_str();
		else
		{
			name = slotname(*it);
			if(!name)
			{
				downgrade = true;
				continue;
			}
		}
		Version *t_best_slot = best_slot(name);
		if(!t_best_slot)
		{
			downgrade = true;
			continue;
		}
		if(*t_best_slot < *it)
		{
			downgrade = true;
			continue;
		}
		if(*t_best_slot != *it)
		{
			upgrade = true;
			continue;
		}
	}
	if(upgrade && downgrade)
		return 2;
	if(upgrade)
		return 1;
	if(downgrade)
		return -1;
	return 0;
}

/** Compare best() version with that of p.
    return value:
	 0: same
	 1: p is smaller
	-1: p is larger
	-2: same, but overlays are different */
int
Package::compare(const Package &p) const
{
	Version *t_best = best();
	Version *p_best = p.best();
	if(t_best && p_best)
	{
		if(*t_best == *p_best)
			return 0;
		if(*t_best > *p_best)
			return 1;
		if(*t_best < *p_best)
			return -1;
		return -2;
	}
	if(t_best)
		return 1;
	if(p_best)
		return -1;
	return 0;
}

/** Compare best() version with that installed in v.
    if v is NULL, it is assumed that none is installed.
    return value:
	 0: All installed versions are best and
	    (unless only_installed) one is installed
	 1: upgrade necessary
	-1: downgrade necessary
	-2: (if only_installed) nothing is installed,
	    and nothing can be installed
	 3: (if only_installed) nothing is installed,
	    but one can be installed */
int
Package::compare(VarDbPkg *v, bool only_installed) const
{
	BasicVersion *t_best = best();
	vector<BasicVersion> *ins = NULL;
	if(v)
		ins = v->getInstalledVector(*this);
	if(ins)
		if(!ins->size())
			ins = NULL;
	if(ins)
	{
		if(!t_best)
			return -1;
		for(vector<BasicVersion>::const_iterator it = ins->begin();
			it != ins->end(); ++it)
		{
			if(*t_best > *it)
				continue;
			if(*t_best < *it)
				return -1;
			return 0;
		}
		return 1;
	}
	if(only_installed)
		return 0;
	if(t_best)
		return 3;
	return -2;
}

void Package::deepcopy(const Package &p)
{
	*this=p;
	for(Package::iterator it=begin(); it != end(); ++it)
	{
		Version *v=new Version;
		*v=(**it);
		*it = v;
	}
	// The pointers in slotlist should point to the clone.
	calculate_slotlist();
}
