// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef __TEST_H__
#define __TEST_H__

#include <iostream>

#define TEST_ASSERT(x) do { \
	if(!(x)) { \
		std::cout << #x << " failed." << std::endl; \
		exit(1); \
	} \
} while(0)

#endif /* __TEST_H__ */
