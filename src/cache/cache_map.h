// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>

#ifndef EIX__GET_H__
#define EIX__GET_H__ 1

#include <string>

class BasicCache;

BasicCache *get_cache(const std::string &name, const std::string &appending);

#endif /* EIX__GET_H__ */
