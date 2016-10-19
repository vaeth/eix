// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_PORTAGE_PACKAGE_H_
#define SRC_PORTAGE_PACKAGE_H_ 1

#include <list>
#include <map>
#include <string>
#include <vector>

#include "eixTk/constexpr.h"
#include "eixTk/eixint.h"
#include "eixTk/inttypes.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/ptr_list.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/instversion.h"
#include "portage/keywords.h"
#include "portage/version.h"

class VarDbPkg;
class PortageSettings;

/**
A sorted list of pointer to Versions
**/
class VersionList : public std::list<Version*> {  // null entries are not allowed
	public:
		explicit VersionList(Version *v) ATTRIBUTE_NONNULL_ : std::list<Version*>(1, v) {
		}

		Version* best(bool allow_unstable) const ATTRIBUTE_PURE;
		Version* best() const ATTRIBUTE_PURE {
			return best(false);
		}
};

class SlotVersions {
	private:
		const char  *m_slotname;
		VersionList  m_version_list;
	public:
		const char *slotname() const {
			return m_slotname;
		}

		const VersionList& const_version_list() const {
			return m_version_list;
		}

		VersionList& version_list() {
			return m_version_list;
		}

		SlotVersions(const char *s, Version *v)  ATTRIBUTE_NONNULL((2, 3)) :
			m_slotname(s), m_version_list(v) {
		}
};

/**
This list is always sorted with respect to the first version for each slot
**/
class SlotList : public std::vector<SlotVersions> {
	public:
		void push_back_largest(Version *version) ATTRIBUTE_NONNULL_;
		const VersionList *operator[](const char *s) const ATTRIBUTE_NONNULL_ ATTRIBUTE_PURE;
};

/**
A class to represent a package in portage It contains various information
about a package, including a sorted(!) list of versions.
**/
class Package : public eix::ptr_list<Version> {
	public:
		typedef std::vector<Version *> VerVec;

		friend class PackageReader;

		/**
		True if duplicated versions are found in for this package.
		That means e.g. that version 0.2 is found in two overlays.
		**/
		typedef uint8_t Duplicates;
		static CONSTEXPR Duplicates
			DUP_NONE     = 0x00U,
			DUP_SOME     = 0x01U,  ///< Duplicate versions are somewhere
			DUP_OVERLAYS = 0x03U;  ///< Duplicate versions are both in overlays

		Duplicates have_duplicate_versions;

		/**
		The largest overlay from which one of the version comes.
		**/
		ExtendedVersion::Overlay largest_overlay;

		typedef uint8_t Versioncollects;
		static CONSTEXPR Versioncollects
			COLLECT_NONE                  = 0x00U,
			COLLECT_HAVE_NONTRIVIAL_SLOTS = 0x01U,
			COLLECT_HAVE_SAME_OVERLAY_KEY = 0x02U,
			COLLECT_AT_LEAST_TWO_OVERLAYS = 0x04U,
			COLLECT_HAVE_MAIN_REPO_KEY    = 0x08U,
			COLLECT_DEFAULT               = COLLECT_HAVE_SAME_OVERLAY_KEY;
		Versioncollects version_collects;

		MaskFlags local_collects;

		std::vector<MaskFlags> saved_collects;

		/**
		Package properties (stored in db)
		**/
		std::string category, name, desc, homepage, licenses;

		IUseSet iuse;

		/**
		Our calc_allow_upgrade_slots(this) cache;
		mutable since it is just a cache.
		**/
		mutable bool allow_upgrade_slots, know_upgrade_slots;
		bool calc_allow_upgrade_slots(const PortageSettings *ps) const ATTRIBUTE_NONNULL_;

		const SlotList& slotlist() const {
			if(!m_has_cached_slotlist) {
				build_slotlist();
				m_has_cached_slotlist = true;
			}
			return m_slotlist;
		}

		/**
		@return true if at least one slot is nonempty (different from "0")
		**/
		bool have_nontrivial_slots() const {
			return (version_collects & COLLECT_HAVE_NONTRIVIAL_SLOTS);
		}

		/**
		@return true if all versions come from one overlay.
		**/
		bool have_same_overlay_key() const {
			return (version_collects & COLLECT_HAVE_SAME_OVERLAY_KEY);
		}

		/**
		@return true if any version comes from main repository.
		**/
		bool have_main_repo_key() const {
			return (version_collects & COLLECT_HAVE_MAIN_REPO_KEY);
		}

		/**
		@return true if all versions come from at least two overlays.
		**/
		bool at_least_two_overlays() const {
			return (version_collects & COLLECT_AT_LEAST_TWO_OVERLAYS);
		}

		/**
		@return true if some version is in @system.
		**/
		bool is_system_package() const {
			return local_collects.isSystem();
		}

		/**
		@return true if some version is in @profile.
		**/
		bool is_profile_package() const {
			return local_collects.isProfile();
		}

		/**
		@return true if some version is in the world file.
		**/
		bool is_world_package() const {
			return local_collects.isWorld();
		}

		/**
		@return true if some version is in the world sets.
		**/
		bool is_world_sets_package() const {
			return local_collects.isWorldSets();
		}

		/**
		Preset with defaults
		**/
		Package() : saved_collects(Version::SAVEMASK_SIZE, MaskFlags(MaskFlags::MASK_NONE)) {
			defaults();
		}

		/**
		Fill in name and category and preset with defaults
		**/
		Package(const std::string& c, const std::string& n) :
			saved_collects(Version::SAVEMASK_SIZE, MaskFlags(MaskFlags::MASK_NONE)), category(c), name(n) {
			defaults();
		}

		/**
		De-constructor, delete content of Version-list.
		**/
		~Package();

		/**
		Adds a version to "the versions" list.
		Only BasicVersion needs to be filled here.
		You must call addVersionFinalize() after filling
		the remaining data
		**/
		void addVersionStart(Version *version) {
			checkDuplicates(version);
			sortedPushBack(version);
		}

		/**
		Finishes addVersion() after the remaining data have been filled
		**/
		void addVersionFinalize(Version *version) ATTRIBUTE_NONNULL_;

		/**
		Add a version to "the versions" list.
		**/
		void addVersion(Version *version) ATTRIBUTE_NONNULL_ {
			addVersionStart(version);
			addVersionFinalize(version);
		}

		/**
		Call this after modifying system or world state of versions.
		**/
		void finalize_masks();

		void save_keyflags(Version::SavedKeyIndex i) {
			for(iterator it(begin()); likely(it != end()); ++it) {
				it->save_keyflags(i);
			}
		}

		void save_maskflags(Version::SavedMaskIndex i) {
			saved_collects[i] = local_collects;
			for(iterator it(begin()); likely(it != end()); ++it) {
				it->save_maskflags(i);
			}
		}

		void save_accepted_effective(Version::SavedEffectiveIndex i) {
			for(iterator it(begin()); likely(it != end()); ++it) {
				it->save_accepted_effective(i);
			}
		}

		bool restore_keyflags(Version::SavedKeyIndex i) {
			local_collects = saved_collects[i];
			for(iterator it(begin()); likely(it != end()); ++it) {
				if(unlikely(!(it->restore_keyflags(i)))) {
					return false;
				}
			}
			return true;
		}

		bool restore_maskflags(Version::SavedMaskIndex i) {
			for(iterator it(begin()); likely(it != end()); ++it) {
				if(unlikely(!(it->restore_maskflags(i)))) {
					return false;
				}
			}
			return true;
		}

		bool restore_accepted_effective(Version::SavedEffectiveIndex i) {
			for(iterator it(begin()); likely(it != end()); ++it) {
				if(unlikely(!(it->restore_accepted_effective(i)))) {
					return false;
				}
			}
			return true;
		}

		Version *best(bool allow_unstable) const ATTRIBUTE_PURE;
		Version *best() const ATTRIBUTE_PURE {
			return best(false);
		}

		Version *best_slot(const char *slot_name, bool allow_unstable) const ATTRIBUTE_NONNULL_;
		Version *best_slot(const char *slot_name) const ATTRIBUTE_NONNULL_ {
			return best_slot(slot_name, false);
		}

		void best_slots(Package::VerVec *l, bool allow_unstable) const ATTRIBUTE_NONNULL_;
		void best_slots(Package::VerVec *l) const ATTRIBUTE_NONNULL_ {
			return best_slots(l, false);
		}

		/**
		Calculate list of uninstalled upgrade candidates
		**/
		void best_slots_upgrade(Package::VerVec *versions, VarDbPkg *v, const PortageSettings *ps, bool allow_unstable) const ATTRIBUTE_NONNULL((2, 4));

		/**
		@return true if version is an (installed or uninstalled) upgrade candidate
		**/
		bool is_best_upgrade(bool check_slots, const Version *version, VarDbPkg *v, const PortageSettings *ps, bool allow_unstable) const ATTRIBUTE_NONNULL((3, 5));

		/**
		Test whether p has a worse best_slot().
		@return
		-  1: p has  a worse/missing best_slot
		-  3: p has no worse/missing best_slot, but an
		      identical from a different overlay
		-  0: else
		**/
		eix::TinySigned worse_best_slots(const Package& p) const;

		/**
		Compare best_slots() versions with that of p.
		@return
		-  0: Everything matches
		-  1: p has a worse/missing best_slot, and *this has not
		- -1: *this has a worse/missing best_slot, and p has not
		-  2: p and *this both have a worse/missing best_slot
		-  3: all matches, but at least one overlay differs
		**/
		eix::TinySigned compare_best_slots(const Package& p) const;

		/**
		Compare best() version with that of p.
		@return
		-  0: same
		-  1: p is smaller
		- -1: p is larger
		-  3: same, but overlays (or slots if test_slot)
		      are different
		**/
		eix::TinySigned compare_best(const Package& p, bool test_slot) const ATTRIBUTE_PURE;

		/**
		@return true if p has a worse/missing best/best_slot/different overlay
		**/
		bool have_worse(const Package& p, bool test_slots) const {
			if(test_slots) {
				return (worse_best_slots(p) > 0);
			}
			return (compare_best(p, false) > 0);
		}

		/**
		@return true if p differs in at least one best/best_slot
		**/
		bool differ(const Package& p, bool test_slots) const {
			if(test_slots) {
				return compare_best_slots(p);
			}
			return compare_best(p, false);
		}

		/**
		Compare best_slots() versions with that installed in v.
		If v is NULLPTR, it is assumed that none is installed.
		@return
		-  0: All installed versions are best and
		      (unless only_installed) one is installed
		      or nothing is installed and nothing can be
		      installed
		-  1: upgrade   necessary but no downgrade
		- -1: downgrade necessary but no upgrade
		-  2: upgrade and downgrade necessary
		-  4: (if only_installed) nothing is installed,
		      but one can be installed
		**/
		eix::TinySigned check_best_slots(VarDbPkg *v, bool only_installed) const;

		/**
		Compare best() version with that installed in v.
		if v is NULLPTR, it is assumed that none is installed.
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
		      but one can be installed
		**/
		eix::TinySigned check_best(VarDbPkg *v, bool only_installed, bool test_slot) const;

		/**
		@return true if we can upgrade v or has v different slots
		**/
		bool can_upgrade(VarDbPkg *v, const PortageSettings *ps, bool only_installed, bool test_slots) const ATTRIBUTE_NONNULL((3)) {
			if(!test_slots) {
				return (check_best(v, only_installed, false) > 0);
			}
			if(calc_allow_upgrade_slots(ps)) {
				if(check_best(v, only_installed, true) > 0) {
					return true;
				}
			}
			return (check_best_slots(v, only_installed) > 0);
		}

		/**
		@return true if we must downgrade v or has v different categories/slots
		**/
		bool must_downgrade(VarDbPkg *v, bool test_slots) const {
			eix::TinySigned c(check_best(v, true, test_slots));
			if((c < 0) || (c == 3)) {
				return true;
			}
			if(!test_slots) {
				return false;
			}
			c = check_best_slots(v, true);
			return ((c < 0) || (c == 2));
		}

		/**
		@return true if we have an upgrade/downgrade recommendation
		**/
		bool recommend(VarDbPkg *v, const PortageSettings *ps, bool only_installed, bool test_slots) const ATTRIBUTE_NONNULL((3)) {
			return can_upgrade(v, ps, only_installed, test_slots) ||
				must_downgrade(v, test_slots);
		}

		bool differ(const Package& p, VarDbPkg *v, const PortageSettings *ps, bool only_installed, bool testvardb, bool test_slots) const {
			if(testvardb) {
				return recommend(v, ps, only_installed, test_slots);
			}
			return differ(p, test_slots);
		}

		/**
		Get the name of a slot of a version.
		returns NULLPTR if not found.
		**/
		const char *slotname(const ExtendedVersion& v) const ATTRIBUTE_PURE;

		/**
		Get the name of a slot of an installed version,
		possibly reading it from disk.
		@return true if a reasonable choice seems to be found
		(v.know_slot determines whether we had full success).
		**/
		bool guess_slotname(InstVersion *v, const VarDbPkg *vardbpkg, const char *force) const ATTRIBUTE_NONNULL((2));
		bool guess_slotname(InstVersion *v, const VarDbPkg *vardbpkg) const ATTRIBUTE_NONNULL((2)) {
			return guess_slotname(v, vardbpkg, NULLPTR);
		}

		Version *latest() const {
			return *rbegin();
		}

		eix::SignedBool compare(const Package& c) const {
			int i(category.compare(c.category));
			if(likely(i == 0)) {
				return name.compare(c.name);
			}
			return i;
		}

	protected:
		/**
		\c slotlist is always sorted with respect to the first version.
		Moreover, this list is complete, i.e. it contains also the
		trivial slot (-> each version is contained exactly in one slot)
		**/
		mutable SlotList m_slotlist;
		mutable bool m_has_cached_slotlist;

		/**
		This is for caching in guess_slotname
		**/
		mutable bool m_has_cached_subslots, m_unique_subslot;
		mutable std::string m_subslot;

		/**
		Create new slotlist. Const because we operate on mutable cache types.
		**/
		void build_slotlist() const;

		/**
		This is called by addVersionFinalize() to calculate
		collected iuse and to save memory by freeing version iuse
		**/
		void collect_iuse(Version *version) ATTRIBUTE_NONNULL_;

		/**
		Check if a package has duplicated versions.
		**/
		void checkDuplicates(const Version *version) ATTRIBUTE_NONNULL_;

		void sortedPushBack(Version *version) ATTRIBUTE_NONNULL_;

		void defaults() {
			know_upgrade_slots = m_has_cached_slotlist =
				m_has_cached_subslots = false;
			have_duplicate_versions = DUP_NONE;
			version_collects = COLLECT_DEFAULT;
			local_collects.set(MaskFlags::MASK_NONE);
		}
};

// These tests are most efficient if the compare "close" elements:
// They are supposed to be used in eix-diff to compare sorted lists
inline static bool operator<(const Package& a, const Package& b) {
	int i(a.category.compare(b.category));
	if(likely(i == 0)) {
		return (a.name < b.name);
	}
	return (i < 0);
}

inline static bool operator<=(const Package& a, const Package& b) {
	int i(a.category.compare(b.category));
	if(likely(i == 0)) {
		return (a.name <= b.name);
	}
	return (i < 0);
}

inline static bool operator==(const Package& a, const Package& b) {
	return (likely(a.category == b.category) && (a.name == b.name));
}

inline static bool operator>(const Package& a, const Package& b) {
	return (b < a);
}

inline static bool operator>=(const Package& a, const Package& b) {
	return (b <= a);
}

class PackageSave {
		typedef std::map<const Version*, KeywordSave> DataType;
		DataType data;

	public:
		explicit PackageSave(const Package *p) {
			store(p);
		}

		void store(const Package *p);

		void restore(Package *p) const ATTRIBUTE_NONNULL_;
};

class PackagePtr {
	private:
		Package *ptr;

	public:
		PackagePtr() : ptr(NULLPTR) {
		}

		explicit PackagePtr(Package *p) : ptr(p) {
		}

		operator Package *() const {
			return ptr;
		}

		Package& operator*() const {
			return *ptr;
		}

		Package *operator->() const {
			return ptr;
		}

		PackagePtr& operator=(Package *p) {
			ptr = p;
			return *this;
		}
};

// Comparison is assumed to happen only in PackageTree where we know
// that categories do match
inline static bool operator<(const PackagePtr& a, const PackagePtr& b) {
	return (a->name < b->name);
}

inline static bool operator==(const PackagePtr& a, const PackagePtr& b) {
	return (a->name == b->name);
}

inline static bool operator>(const PackagePtr& a, const PackagePtr& b) {
	return (a->name > b->name);
}

inline static bool operator<=(const PackagePtr& a, const PackagePtr& b) {
	return (a->name <= b->name);
}

inline static bool operator>=(const PackagePtr& a, const PackagePtr& b) {
	return (a->name >= b->name);
}

#endif  // SRC_PORTAGE_PACKAGE_H_
