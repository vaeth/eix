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
