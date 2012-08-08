// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_SEARCH_NOWARN_H_
#define SRC_SEARCH_NOWARN_H_ 1

#include <string>
#include <vector>

#include "eixTk/null.h"
#include "portage/keywords.h"
#include "portage/mask.h"
#include "portage/mask_list.h"
#include "search/packagetest.h"

class NowarnKeywords;
class Package;
class PortageSettings;

class NowarnFlags
{
	Keywords::Redundant red;
	PackageTest::TestInstalled ins;
public:
	NowarnFlags(Keywords::Redundant r = Keywords::RED_NOTHING, PackageTest::TestInstalled i = PackageTest::INS_NONE) :
	red(r), ins(i)
	{ }

	void apply_red(Keywords::Redundant *r) const
	{ *r &= ~red; }

	void apply_ins(PackageTest::TestInstalled *i) const
	{ *i &= ~ins; }

	void clear()
	{
		red = Keywords::RED_NOTHING;
		ins = PackageTest::INS_NONE;
	}

	void setbits(const NowarnFlags &s)
	{
		red |= s.red;
		ins |= s.ins;
	}

	void clearbits(const NowarnFlags &s)
	{
		red &= ~s.red;
		ins &= ~s.ins;
	}
};


class NowarnMask : public Mask
{
		friend class NowarnMaskList;
	public:
		NowarnFlags set_flags, clear_flags;

		NowarnMask() : Mask(maskTypeNone)
		{ }

		void init_nowarn(const std::vector<std::string> &flagstrings);

		static void init_static();
	private:
		static NowarnKeywords *nowarn_keywords;
};

class NowarnMaskList : public MaskList<NowarnMask>
{
		typedef MaskList<NowarnMask> super;
	public:
		void apply(Package *p, Keywords::Redundant *r, PackageTest::TestInstalled *i, PortageSettings *portagesettings) const;
};

class NowarnPreList : public PreList
{
		typedef PreList super;
	public:
		NowarnPreList() : super()
		{ }

		NowarnPreList(const std::vector<std::string> &lines, const std::string &filename, bool only_add) : super(lines, filename, NULLPTR, only_add)
		{ }

		void initialize(NowarnMaskList *l);
};

#endif  // SRC_SEARCH_NOWARN_H_
