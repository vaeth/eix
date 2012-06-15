// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "package.h"
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <portage/basicversion.h>
#include <portage/conf/portagesettings.h>
#include <portage/extendedversion.h>
#include <portage/instversion.h>
#include <portage/keywords.h>
#include <portage/vardbpkg.h>
#include <portage/version.h>

#include <set>
#include <vector>

using namespace std;

bool
Package::calc_allow_upgrade_slots(const PortageSettings *ps) const
{
	if(likely(know_upgrade_slots))
		return allow_upgrade_slots;
	know_upgrade_slots = true;
	return (allow_upgrade_slots = ps->calc_allow_upgrade_slots(this));
}

Version *
Package::best(bool allow_unstable) const
{
	for(const_reverse_iterator ri(rbegin()); likely(ri != rend()); ++ri) {
		if(ri->maskflags.isHardMasked())
			continue;
		if(ri->keyflags.isStable() ||
			(allow_unstable && ri->keyflags.isUnstable()))
			return *ri;
	}
	return NULLPTR;
}

void
Package::build_slotslit() const
{
	m_slotlist.clear();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		m_slotlist.push_back_largest(*it);
	}
}


Version *
Package::best_slot(const char *slot_name, bool allow_unstable) const
{
	const VersionList *vl(slotlist()[slot_name]);
	if(vl == NULLPTR)
		return NULLPTR;
	return vl->best(allow_unstable);
}

void
Package::best_slots(vector<Version*> &l, bool allow_unstable) const
{
	l.clear();
	for(SlotList::const_iterator sit(slotlist().begin());
		likely(sit != slotlist().end()); ++sit) {
		Version *p((sit->const_version_list()).best(allow_unstable));
		if(p != NULLPTR) {
			l.push_back(p);
		}
	}
}

void
Package::best_slots_upgrade(vector<Version*> &versions, VarDbPkg *v, const PortageSettings *ps, bool allow_unstable) const
{
	versions.clear();
	if(unlikely(v == NULLPTR))
		return;
	vector<InstVersion> *ins(v->getInstalledVector(*this));
	if((ins == NULLPTR) || (ins->empty()))
		return;
	bool need_best(false);
	set<Version*> versionset;
	for(vector<InstVersion>::iterator it(ins->begin());
		it != ins->end() ; ++it) {
		if(guess_slotname(*it, v)) {
			Version *bv(best_slot((it->slotname).c_str(), allow_unstable));
			if((bv != NULLPTR) && (*bv != *it))
				versionset.insert(bv);
		}
		else // Perhaps the slot was removed:
			need_best=true;
	}
	if(!need_best) {
		if(calc_allow_upgrade_slots(ps))
			need_best = true;
	}
	if(need_best) {
		Version *bv(best(allow_unstable));
		if(bv != NULLPTR)
			versionset.insert(bv);
	}
	if(versionset.empty())
		return;
	// Return only uninstalled versions:
	for(set<Version*>::const_iterator it(versionset.begin());
		likely(it != versionset.end()); ++it) {
		bool found(false);
		for(vector<InstVersion>::const_iterator insit(ins->begin());
			likely(insit != ins->end()); ++insit) {
			if(*insit == **it) {
				found = true;
				break;
			}
		}
		if(!found)
			versions.push_back(*it);
	}
}

bool
Package::is_best_upgrade(bool check_slots, const Version* version, VarDbPkg *v, const PortageSettings *ps, bool allow_unstable) const
{
	if(unlikely(v == NULLPTR))
		return false;
	vector<InstVersion> *ins(v->getInstalledVector(*this));
	if(ins == NULLPTR)
		return false;
	bool need_best(!check_slots);
	if(check_slots) {
		for(vector<InstVersion>::iterator it(ins->begin());
			likely(it != ins->end()); ++it) {
			if(guess_slotname(*it, v)) {
				if(version == best_slot((it->slotname).c_str(), allow_unstable))
					return true;
			}
			else // Perhaps the slot was removed:
				need_best = true;
		}
		if(!need_best) {
			if(calc_allow_upgrade_slots(ps))
				need_best = true;
		}
	}
	if(need_best) {
		if(version == best(allow_unstable))
			return true;
	}
	return false;
}

const char *
Package::slotname(const ExtendedVersion &v) const
{
	for(const_iterator i(begin()); likely(i != end()); ++i) {
		if(**i == v) {
			return (i->slotname).c_str();
		}
	}
	return NULLPTR;
}

bool
Package::guess_slotname(InstVersion &v, const VarDbPkg *vardbpkg) const
{
	if(vardbpkg->care_slots())
		return vardbpkg->readSlot(*this, v);
	if(likely(v.know_slot))
		return true;
	const char *s(slotname(v));
	if(s != NULLPTR) {
		v.slotname = s;
		v.know_slot = true;
	}
	if(vardbpkg->readSlot(*this, v))
		return true;
	if(slotlist().size() == 1)
	{
		// There is only one slot, so the choice seems clear.
		// However, perhaps our package is from an old database
		// (e.g. in eix-diff) and so there might be new slots elsewhere
		// Therefore we better don't modify v.know_slot.
		v.slotname = slotlist().begin()->slotname();
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
int
Package::worse_best_slots(const Package &p) const
{
	int ret(0);
	for(SlotList::const_iterator it(slotlist().begin());
		it != slotlist().end(); ++it) {
		Version *t_best((it->const_version_list()).best());
		if(!t_best)
			continue;
		Version *p_best(p.best_slot(it->slotname()));
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
	int worse(worse_best_slots(p));
	int better(p.worse_best_slots(*this));
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
	Version *t_best(best());
	Version *p_best(p.best());
	if((t_best != NULLPTR) && (p_best != NULLPTR)) {
		if(*t_best > *p_best)
			return 1;
		if(*t_best < *p_best)
			return -1;
		if(t_best->overlay_key != p_best->overlay_key)
			return 3;
		if(test_slot && (t_best->slotname != p_best->slotname))
			return 3;
		return 0;
	}
	if(t_best != NULLPTR)
		return 1;
	if(p_best != NULLPTR)
		return -1;
	return 0;
}

/** Compare best_slots() versions with that installed in v.
    if v is NULLPTR, it is assumed that none is installed.
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
	vector<InstVersion> *ins(NULLPTR);
	if(likely(v != NULLPTR))
		ins = v->getInstalledVector(*this);
	if((ins == NULLPTR) || ins->empty()) {
		if(!only_installed) {
			if(best())
				return 4;
		}
		return 0;
	}
	bool downgrade(false);
	bool upgrade(false);
	for(vector<InstVersion>::iterator it(ins->begin());
		likely(it != ins->end()); ++it) {
		if(!guess_slotname(*it, v)) {
			// Perhaps the slot was removed:
			downgrade = true;
			Version *t_best(best());
			if(t_best) {
				if(*t_best > *it)
					upgrade = true;
			}
			continue;
		}
		Version *t_best_slot(best_slot((it->slotname).c_str()));
		if(!t_best_slot) {
			downgrade = true;
			continue;
		}
		if(*t_best_slot < *it) {
			downgrade = true;
			continue;
		}
		if(*t_best_slot != *it) {
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
	      but one can be installed */
int
Package::check_best(VarDbPkg *v, bool only_installed, bool test_slot) const
{
	ExtendedVersion *t_best(best());
	vector<InstVersion> *ins(NULLPTR);
	if(likely(v != NULLPTR))
		ins = v->getInstalledVector(*this);
	if((ins != NULLPTR) && !ins->empty()) {
		if(!t_best)
			return -1;
		for(vector<InstVersion>::iterator it(ins->begin());
			likely(it != ins->end()); ++it) {
			short vgl(BasicVersion::compare(*t_best, *it));
			if(vgl > 0)
				continue;
			if(vgl < 0)
				return -1;
			if(!test_slot)
				return 0;
			if(guess_slotname(*it, v))
			{
				if(t_best->slotname == it->slotname)
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

void
PackageSave::store(const Package *p)
{
	data.clear();
	if(p == NULLPTR)
		return;
	for(Package::const_iterator it(p->begin());
		likely(it != p->end()); ++it)
		data[*it] = KeywordSave(*it);
}

void
PackageSave::restore(Package *p) const
{
	if(unlikely(data.empty()))
		return;
	for(Package::iterator it(p->begin());
		likely(it != p->end()); ++it) {
		DataType::const_iterator d(data.find(*it));
		if(d == data.end())
			continue;
		d->second.restore(*it);
	}
}
