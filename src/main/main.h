// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_MAIN_MAIN_H_
#define SRC_MAIN_MAIN_H_ 1

#include <config.h>  // IWYU pragma: keep

extern const char *program_name;

int run_eix(int argc, char *argv[]);
int run_eix_update(int argc, char *argv[]);
int run_eix_diff(int argc, char *argv[]);
int run_eix_header(int argc, char *argv[]);
int run_eix_drop_permissions(int argc, char *argv[]);
int run_masked_packages(int argc, char *argv[]);
int run_versionsort(int argc, char *argv[]);

#endif  // SRC_MAIN_MAIN_H_
