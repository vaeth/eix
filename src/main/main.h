// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__MAIN_H__
#define EIX__MAIN_H__ 1

#include <string>
extern std::string program_name;

int run_eix(int argc, char *argv[]);
int run_eix_update(int argc, char *argv[]);
int run_eix_diff(int argc, char *argv[]);
int run_eix_drop_permissions(int argc, char *argv[]);
int run_versionsort(int argc, char *argv[]);

#endif /* EIX__MAIN_H__ */
