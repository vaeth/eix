// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__CDB_H__)
#define EIX__CDB_H__

#include <cache/base.h>

class CdbCache : public BasicCache {

	public:
		bool readCategory(Category &vec) throw(ExBasic);

		const char *getType() const
		{ return "cdb"; }
};

#endif /* EIX__CDB_H__ */
