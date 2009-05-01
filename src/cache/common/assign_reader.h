// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__ASSIGNREADER_H__
#define EIX__ASSIGNREADER_H__ 1

#include <portage/keywords.h>
#include <cache/base.h>

class Package;

void assign_get_keywords_slot_iuse_restrict(const std::string &filename, std::string &keywords, std::string &slotname, std::string &iuse, std::string &restr, std::string &props, BasicCache::ErrorCallback error_callback);
void assign_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback);

#endif /* EIX__ASSIGNREADER_H__ */
