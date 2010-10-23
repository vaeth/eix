// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "nowarn.h"
#include <eixTk/exceptions.h>
#include <eixTk/likely.h>
#include <portage/conf/portagesettings.h>
#include <portage/keywords.h>
#include <portage/mask.h>
#include <portage/mask_list.h>
#include <portage/packagesets.h>
#include <search/packagetest.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include <cstddef>

using namespace std;

class NowarnKeywords
{
	typedef map<string, NowarnFlags> NameMap;
	NameMap name_map;
public:
	void init_red(const char *s, Keywords::Redundant red)
	{ name_map.insert(pair<string, NowarnFlags>(s, NowarnFlags(red))); }

	void init_ins(const std::string &s, PackageTest::TestInstalled ins)
	{ name_map.insert(pair<string, NowarnFlags>(s, NowarnFlags(Keywords::RED_NOTHING, ins))); }

	void apply(const std::string &s, NowarnFlags &f) const
	{
		NameMap::const_iterator it(name_map.find(s));
		if(it != name_map.end()) {
			f.setbits(it->second);
		}
	}

	NowarnKeywords()
	{
		init_red("in_keywords",      Keywords::RED_IN_KEYWORDS);
		init_red("no_change",        Keywords::RED_NO_CHANGE);
		init_red("double",           Keywords::RED_DOUBLE);
		init_red("mixed",            Keywords::RED_MIXED);
		init_red("weaker",           Keywords::RED_WEAKER);
		init_red("strange",          Keywords::RED_STRANGE);
		init_red("minusasterisk",    Keywords::RED_MINUSASTERISK);
		init_red("double_line",      Keywords::RED_DOUBLE_LINE);
		init_red("in_mask",          Keywords::RED_IN_MASK);
		init_red("mask_no_change",   Keywords::RED_MASK);
		init_red("double_masked",    Keywords::RED_DOUBLE_MASK);
		init_red("in_unmask",        Keywords::RED_IN_UNMASK);
		init_red("unmask_no_change", Keywords::RED_UNMASK);
		init_red("double_unmasked",  Keywords::RED_DOUBLE_UNMASK);
		init_red("in_use",           Keywords::RED_IN_USE);
		init_red("double_use",       Keywords::RED_DOUBLE_USE);
		init_red("in_env",           Keywords::RED_IN_ENV);
		init_red("double_env",       Keywords::RED_DOUBLE_ENV);
		init_red("in_cflags",        Keywords::RED_IN_CFLAGS);
		init_red("double_cflags",    Keywords::RED_DOUBLE_CFLAGS);
		init_ins("nonexistent",      PackageTest::INS_NONEXISTENT);
		init_ins("masked",           PackageTest::INS_MASKED);
		init_ins("other_overlay",    PackageTest::INS_OVERLAY);
	}
};
static NowarnKeywords nowarn_keywords;

void
NowarnMask::init_nowarn(const vector<string> &flagstrings)
{
	set_flags.clear();
	clear_flags.clear();
	for(vector<string>::const_iterator it(flagstrings.begin());
		likely(it != flagstrings.end()); ++it) {
		if(it->empty()) {
			continue;
		}
		if((*it)[0] == '-') {
			nowarn_keywords.apply(it->substr(1), clear_flags);
		}
		nowarn_keywords.apply(*it, set_flags);
	}
}

void
NowarnMaskList::apply(Package *p, Keywords::Redundant &r, PackageTest::TestInstalled &i, PortageSettings *portagesettings) const
{
	super::Get *masks(get(p));
	if(masks == NULL) {
		return;
	}
	bool found(false);
	NowarnFlags set_flags, clear_flags;
	for(super::Get::const_iterator it(masks->begin());
		likely(it != masks->end()); ++it) {
		if(!it->have_match(*p)) {
			continue;
		}
		found = true;
		set_flags.setbits(it->set_flags);
		clear_flags.setbits(it->clear_flags);
	}
	delete masks;
	// Now we also apply the set-items...
	set<SetsIndex> my_sets;
	for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
		if(likely(v->sets_indizes.empty())) {
			// Shortcut for the most frequent case
			continue;
		}
		for(vector<SetsIndex>::const_iterator sit(v->sets_indizes.begin());
			unlikely(sit != v->sets_indizes.end()); ++sit) {
			my_sets.insert(*sit);
		}
	}
	if(unlikely(!my_sets.empty())) {
		for(set<SetsIndex>::const_iterator sit(my_sets.begin());
			unlikely(sit != my_sets.end()); ++sit) {
			super::Get *setmasks(get_setname(portagesettings->set_names[*sit]));
			if(setmasks == NULL) {
				continue;
			}
			for(super::Get::const_iterator m(setmasks->begin());
				likely(m != setmasks->end()); ++m) {
				found = true;
				set_flags.setbits(m->set_flags);
				clear_flags.setbits(m->clear_flags);
			}
			delete setmasks;
		}
	}
	if(!found) {
		return;
	}
	set_flags.clearbits(clear_flags);
	set_flags.apply_red(r);
	set_flags.apply_ins(i);
}

void
NowarnPreList::initialize(NowarnMaskList &l)
{
	finalize();
	for(const_iterator it(begin()); likely(it != end()); ++it) {
		try {
			NowarnMask m(it->name.c_str());
			m.init_nowarn(it->args);
			l.add(m);
		}
		catch(const ExBasic &e) {
			cerr << e;
		}
	}
	l.finalize();
	clear();
}

