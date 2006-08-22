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

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <eixTk/ptr_list.h>

#include <list>
#include <string>

#include <portage/version.h>

/** A sorted list of pointer to Versions */

class VersionList : public std::list<Version*>
{
	public:
		VersionList(Version *v) : std::list<Version*>(1, v)
		{ }

		Version* best() const;
};

class SlotVersions
{
	private:
		const char  *m_slot;
		VersionList  m_version_list;

	public:
		const char *slot() const
		{ return m_slot; }

		const VersionList &const_version_list() const
		{ return m_version_list; }

		VersionList &version_list()
		{ return m_version_list; }

		SlotVersions(const char *s, Version *v) :
			m_slot(s), m_version_list(v)
		{ }
};

/** This list is always sorted with respect to the first version for each slot */
class SlotList : public std::list<SlotVersions>
{
	public:
		void push_back_largest(Version *version);
		const VersionList *operator [] (const char *s) const;
};

/** A class to represent a package in portage It contains various information
 * about a package, including a sorted(!) list of versions. */
class Package
	: public eix::ptr_list<Version>
{
	public:
		/** True if duplicated versions are found in for this package.
		 * That means e.g. that version 0.2 is found in two overlays. */
		typedef char Duplicates;
		static const Duplicates
			DUP_NONE,
			DUP_SOME,    /* Duplicate versions are somewhere */
			DUP_OVERLAYS;/* Duplicate versions are both in overlays */

		Duplicates have_duplicate_versions;

		/** The following list is always sorted with respect to the
		    first version. Moreover, this list is complete, i.e. it
		    contains also the trivial slot (-> each version is
		    contained exactly in one slot) */
		SlotList slotlist;

		/* True if at least one slot is nonempty (different from "0") */
		bool have_nontrivial_slots;

		/** True if all versions come from one overlay. */
		bool have_same_overlay_key;

		/** True if all versions come from at most one overlay. */
		bool at_least_two_overlays;

		/** The largest overlay from which one of the version comes. */
		Version::Overlay largest_overlay;

		/** True if every version is in the system-profile. */
		bool is_system_package;

		/** Package properties (stored in db) */
		std::string category, name, desc, homepage, licenses, installed_versions, provide;

		/// Preset with defaults
		Package()
		{ defaults(); }

		/// Fill in name and category and preset with defaults
		Package(std::string c, std::string n)
			: category(c), name(n)
		{ defaults(); }

		/** De-constructor, delete content of Version-list. */
		~Package();

		/** Adds a version to "the versions" list.
		    Only BasicVersion needs to be filled here.
		    You must call addVersionFinalize() after filling
		    the remaining data */
		void addVersionStart(Version *version)
		{ checkDuplicates(version); sortedPushBack(version); }

		/** Finishes addVersion() after the remaining data
		    have been filled */
		void addVersionFinalize(Version *version);

		/** Adds a version to "the versions" list. */
		void addVersion(Version *version)
		{ addVersionStart(version); addVersionFinalize(version); }

		void calculate_slotlist();

		Version *best() const;

		Version *best_slot(const char *slot_name) const;

		void best_slots(std::vector<Version*> &l) const;

		const char *slotname(const BasicVersion &v) const;

		Version *latest() const
		{ return *rbegin(); }

		void deepcopy(const Package &p);

	protected:
		/** Check if a package has duplicated vsions. */
		void checkDuplicates(Version *version);

		void sortedPushBack(Version *version);

		void defaults()
		{
			is_system_package = false;
			have_same_overlay_key = true;
			at_least_two_overlays = false;
			have_duplicate_versions = DUP_NONE;
			have_nontrivial_slots = false;
		}
};

#endif /* __PACKAGE_H__ */
