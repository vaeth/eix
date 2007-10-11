// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __FLATREADER_H__
#define __FLATREADER_H__

#include <eixTk/exceptions.h>
#include <portage/keywords.h>
#include <cache/base.h>
#include <string>

class Package;

void flat_get_keywords_slot_iuse_restrict(const std::string &filename, std::string &keywords, std::string &slot, std::string &iuse, std::string &restr, BasicCache::ErrorCallback error_callback);
void flat_read_file(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback);

#endif /* __FLATREADER_H__ */
