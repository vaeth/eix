// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef __PORT2_1_0_H__
#define __PORT2_1_0_H__

#include <portage/cache/base.h>

class Port2_1_0_Cache : public BasicCache {

	public:
		bool readCategory(Category &vec) throw(ExBasic);

		const char *getType() const
		{ return "portage-2.1"; }
};

#endif /* __PORT2_1_0_H__ */
