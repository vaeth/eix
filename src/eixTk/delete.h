// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_DELETE_H_
#define SRC_EIXTK_DELETE_H_ 1

// include "eixTk/unused.h" This comment satisfies check_include script

#ifdef HAVE_DELETE
#define ASSIGN_DELETE = delete

#else /* ifndef HAVE_DELETE */

#define ASSIGN_DELETE

#endif

#endif  // SRC_EIXTK_DELETE_H_
