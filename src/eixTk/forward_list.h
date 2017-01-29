// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_FORWARD_LIST_H_
#define SRC_EIXTK_FORWARD_LIST_H_ 1

#include <config.h>

// check_includes: include "eixTk/forward_list.h"

#ifdef HAVE_FORWARD_LIST
#include <forward_list>
namespace eix {
	using std::forward_list;
}
#else
#include <list>
namespace eix {
template<typename T> class forward_list : public std::list<T> {
};
}
#endif

#endif  // SRC_EIXTK_FORWARD_LIST_H_
