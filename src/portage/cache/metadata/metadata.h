// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __METADATA_H__
#define __METADATA_H__

#include <portage/cache/base.h>

class MetadataCache : public BasicCache {

	public:
		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return true; }

		const char *getType() const
		{ return "metadata"; }
};

#endif /* __METADATA_H__ */
