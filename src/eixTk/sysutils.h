// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_SYSUTILS_H_
#define SRC_EIXTK_SYSUTILS_H_ 1

#include <config.h>

#include <sys/types.h>

#include <ctime>

/** Get uid of a user.
 * @param u pointer to uid_t .. uid is stored there.
 * @param name name of user
 * @return true if user exists */
bool get_uid_of(const char *name, uid_t *u);

/** Get gid of a group.
 * @param g pointer to gid_t .. gid is stored there.
 * @param name name of group
 * @return true if group exists */
bool get_gid_of(const char *name, gid_t *g);

/** @return true if file is a directory or a symlink to some. */
bool is_dir(const char *file);

/** @return true if file is a plain file or a symlink to some. */
bool is_file(const char *file);

/** @return true if file is a plain file (and not a symlink). */
bool is_pure_file(const char *file);

/** @return mtime of file. */
time_t get_mtime(const char *file);

/** @return mydate formatted according to locales and dateFormat */
const char *date_conv(const char *dateFormat, time_t mydate);

#endif  // SRC_EIXTK_SYSUTILS_H_
