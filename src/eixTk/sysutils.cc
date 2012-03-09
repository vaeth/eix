// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "sysutils.h"
#include <eixTk/likely.h>

#include <string>

#include <clocale>
#include <cstddef>
#include <ctime>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

/** Get uid of a user.
 * @param u pointer to uid_t .. uid is stored there.
 * @param name name of user
 * @return true if user exists */
bool
get_uid_of(const char *name, uid_t *u)
{
	struct passwd *pwd(getpwnam(name));
	if(unlikely(pwd == NULL))
		return false;
	*u = pwd->pw_uid;
	return true;
}

/** Get gid of a group.
 * @param g pointer to gid_t .. gid is stored there.
 * @param name name of group
 * @return true if group exists */
bool
get_gid_of(const char *name, gid_t *g)
{
	struct group *grp(getgrnam(name));
	if(unlikely(grp == NULL))
		return false;
	*g = grp->gr_gid;
	return true;
}

/** @return true if file is a directory or a symlink to some. */
bool
is_dir(const char *file)
{
	struct stat stat_buf;
	if(unlikely(stat(file, &stat_buf) != 0))
		return false;
	return S_ISDIR(stat_buf.st_mode);
}

/** @return true if file is a plain file or a symlink to some. */
bool
is_file(const char *file)
{
	struct stat stat_buf;
	if(unlikely(stat(file, &stat_buf) != 0))
		return false;
	return S_ISREG(stat_buf.st_mode);
}

/** @return true if file is a plain file (and not a symlink). */
bool
is_pure_file(const char *file)
{
	struct stat stat_buf;
	if(unlikely(lstat(file, &stat_buf) != 0))
		return false;
	return S_ISREG(stat_buf.st_mode);
}

/** @return mtime of file. */
time_t
get_mtime(const char *file)
{
	struct stat stat_b;
	if(unlikely(stat(file, &stat_b)))
		return 0;
	return stat_b.st_mtime;
}

/** @return mydate formatted according to locales and dateFormat */
const char *
date_conv(const char *dateFormat, time_t mydate)
{
	const int max_datelen = 256;
	static char buffer[max_datelen];
	string old_lcall = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "");
	struct tm *loctime(localtime (&mydate));
	strftime(buffer, max_datelen, dateFormat, loctime);
	setlocale(LC_ALL, old_lcall.c_str());
	return buffer;
}
