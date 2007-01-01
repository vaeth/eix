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

bool Package::upgrade_to_best;

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

void Package::add_coll_iuse(const string &s)
{
	vector<string> iuse = split_string(coll_iuse + " " + s);
	std::sort(iuse.begin(), iuse.end());
	iuse.erase(std::unique(iuse.begin(), iuse.end()), iuse.end());
	coll_iuse = join_vector(iuse);
}

void Package::collect_iuse()
{
	vector<string> iuse = split_string(coll_iuse);
	for(iterator it = begin(); it != end(); ++it) {
		iuse.insert(iuse.end(), (it->iuse).begin(), (it->iuse).end());
#ifdef NOT_FULL_USE
		// Clear iuse to save memory:
		(it->iuse).clear();
#endif
	}
	std::sort(iuse.begin(), iuse.end());
	iuse.erase(std::unique(iuse.begin(), iuse.end()), iuse.end());
	coll_iuse = join_vector(iuse);
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
	collect_iuse();
	// We must recalculate the complete slotlist after each modification.
	// The reason is that the pointers might go into nirvana, because
	// a push_back might move the whole list.
	calculate_slotlist();
}

void Package::save_maskstuff()
{
	for(iterator i = begin(); i != end(); ++i)
		i->save_maskstuff();
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
bool Package::guess_slotname(InstVersion &v, const VarDbPkg *vardbpkg) const
{
	if(vardbpkg->care_slots())
		return vardbpkg->readSlot(*this, v);
	if(v.know_slot)
		return true;
	const char *s = slotname(v);
	if(s)
	{
		v.slot = s;
		v.know_slot = true;
	}
	if(vardbpkg->readSlot(*this, v))
		return true;
	if(slotlist.size() == 1)
	{
		// There is only one slot, so the choice seems clear.
		// However, perhaps our package is from an old database
		// (e.g. in diff-eix) and so there might be new slots elsewhere
		// Therefore we better don't modify v.know_slot.
		v.slot = slotlist.begin()->slot();
		return true;
	}
	return false;
}

/** Test whether p has a worse best_slot()
    @return
	-  1: p has  a worse best_slot
	-  3: p has no worse best_slot, but an identical
	      from a different overlay
	-  0: else */
int Package::worse_best_slots(const Package &p) const
{
	int ret = 0;
	for(SlotList::const_iterator it = slotlist.begin();
		it != slotlist.end(); ++it)
	{
		Version *t_best = (it->const_version_list()).best();
		if(!t_best)
			continue;
		Version *p_best = p.best_slot(it->slot());
		if(!p_best)
			return 1;
		if(*t_best > *p_best)
			return 1;
		if(*t_best < *p_best)
			continue;
		if(t_best->overlay_key != p_best->overlay_key)
			ret = 3;
	}
	return ret;
}

/** Compare best_slots() versions with that of p.
    @return
	-  0: Everything matches
	-  1: p has a worse/missing best_slot, and *this has not
	- -1: *this has a worse/missing best_slot, and p has not
	-  2: p and *this both have a worse/missing best_slot
	-  3: all matches, but at least one overlay differs */
int
Package::compare_best_slots(const Package &p) const
{
	int worse  = worse_best_slots(p);
	int better = p.worse_best_slots(*this);
	if(worse == 1)
	{
		if(better == 1)
			return 2;
		return 1;
	}
	if(better == 1)
		return -1;
	if(worse || better)
		return 3;
	return 0;
}

/** Compare best() version with that of p.
    @return
	-  0: same
	-  1: p is smaller
	- -1: p is larger
	-  3: same, but overlays (or slots if test_slot)
	      are different */
int
Package::compare_best(const Package &p, bool test_slot) const
{
	Version *t_best = best();
	Version *p_best = p.best();
	if(t_best && p_best)
	{
		if(*t_best > *p_best)
			return 1;
		if(*t_best < *p_best)
			return -1;
		if(t_best->overlay_key != p_best->overlay_key)
			return 3;
		if(test_slot && (t_best->slot != p_best->slot))
			return 3;
		return 0;
	}
	if(t_best)
		return 1;
	if(p_best)
		return -1;
	return 0;
}

/** Compare best_slots() versions with that installed in v.
    if v is NULL, it is assumed that none is installed.
    @return
	-  0: All installed versions are best and
	      (unless only_installed) one is installed
	      or nothing is installed and nothing can be
	      installed
	-  1: upgrade   necessary but no downgrade
	- -1: downgrade necessary but no upgrade
	-  2: upgrade and downgrade necessary
	-  4: (if only_installed) nothing is installed,
	      but one can be installed */
int
Package::check_best_slots(VarDbPkg *v, bool only_installed) const
{
	vector<InstVersion> *ins = NULL;
	if(v)
		ins = v->getInstalledVector(*this);
	if(ins)
		if(!ins->size())
			ins = NULL;
	if(!ins)
	{
		if(!only_installed)
		{
			if(best())
				return 4;
		}
		return 0;
	}
	bool downgrade = false;
	bool upgrade = false;
	for(vector<InstVersion>::iterator it = ins->begin();
		it != ins->end() ; ++it)
	{
		if(!guess_slotname(*it, v))
		{
			// Perhaps the slot was removed:
			downgrade = true;
			Version *t_best = best();
			if(t_best)
			{
				if(*t_best > *it)
					upgrade = true;
			}
			continue;
		}
		Version *t_best_slot = best_slot((it->slot).c_str());
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

/** Compare best() version with that installed in v.
    if v is NULL, it is assumed that none is installed.
    @return
	-  0: All installed versions are best and
	      (unless only_installed) one is installed
	      or nothing is installed and nothing can be
	      installed
	-  1: upgrade necessary
	- -1: downgrade necessary
	-  3: (if test_slot) everything matches,
	      but slots are different.
	-  4: (if only_installed) nothing is installed,
	      but one can be installed */
int
Package::check_best(VarDbPkg *v, bool only_installed, bool test_slot) const
{
	BasicVersion *t_best = best();
	vector<InstVersion> *ins = NULL;
	if(v)
		ins = v->getInstalledVector(*this);
	if(ins)
		if(!ins->size())
			ins = NULL;
	if(ins)
	{
		if(!t_best)
			return -1;
		for(vector<InstVersion>::iterator it = ins->begin();
			it != ins->end(); ++it)
		{
			if(*t_best > *it)
				continue;
			if(*t_best < *it)
				return -1;
			if(!test_slot)
				return 0;
			if(guess_slotname(*it, v))
			{
				if(t_best->slot == it->slot)
					return 0;
			}
			return 3;
		}
		return 1;
	}
	if((!only_installed) && t_best)
		return 4;
	return 0;
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
