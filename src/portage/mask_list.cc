// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "mask_list.h"
#include <eixTk/likely.h>
#include <eixTk/ptr_list.h>
#include <eixTk/stringutils.h>
#include <portage/keywords.h>
#include <portage/mask.h>
#include <portage/package.h>

#include <string>
#include <vector>

#include <cstddef>

class Version;

using namespace std;

// return true if some masks applied
template <>
bool
MaskList<Mask>::applyMasks(Package *p, Keywords::Redundant check) const
{
	const eix::ptr_list<Mask> *l(get(p));
	if(l == NULL)
		return false;

	bool rvalue(false);
	bool had_mask(false);
	bool had_unmask(false);
	for(const_mask_iterator m(l->begin()); likely(m != l->end()); ++m) {
		rvalue = 1;
		m->checkMask(*p, check);
		switch(m->get_type())
		{
			case Mask::maskMask:
				had_mask = true;
				break;
			case Mask::maskUnmask:
				had_unmask = true;
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
		for(Package::iterator i(p->begin());
			likely(i != p->end()); ++i) {
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

template<>
void
MaskList<Mask>::applySetMasks(Version *v, const string &set_name) const
{
	const eix::ptr_list<Mask> *l(get(set_name));
	if(l == NULL) {
		return;
	}
	for(const_mask_iterator m(l->begin());
		likely(m != l->end()); ++m) {
		m->apply(v, false, false, Keywords::RED_NOTHING);
	}
}

template<>
void
MaskList<Mask>::applyVirtualMasks(Package *p) const
{
	if(p->provide.empty())
		return;
	vector<string> provide;
	split_string(provide, p->provide);
	for(vector<string>::const_iterator v(provide.begin());
		likely(v != provide.end()); ++v) {
		const eix::ptr_list<Mask> *l(get_split(*v));
		if(l == NULL) {
			continue;
		}
		for(const_mask_iterator m(l->begin());
			likely(m != l->end()); ++m) {
			m->applyVirtual(*p);
		}
	}
}
