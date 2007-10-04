// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <eixTk/exceptions.h>

#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <cstdlib>

using namespace std;

static bool is_on_list (char *const *list, const char *member)
{
	while (*list) {
		if (strcmp (*list, member) == 0)
			return true;
		list++;
	}
	return false;
}

static bool check_user_in_grp_struct(struct group *grp)
{
	struct passwd *pwd;
	if((pwd = getpwuid(getuid())) == 0)
		throw(ExBasic("getpwuid() tells me that my uid is not known to this system."));
	if(is_on_list(grp->gr_mem, pwd->pw_name))
		return true;
	return false;
}

bool user_in_group(const char *group_name)
{
	errno = 0;
	struct group *grp = getgrnam(group_name);
	if(grp == NULL)
		throw(ExBasic("getgrnam(%s) failed: %s", group_name, strerror(errno)));
	return check_user_in_grp_struct(grp);
}

bool get_uid_of(const char *name, uid_t *u)
{
	struct passwd *pwd;
	if((pwd = getpwnam(name)) == NULL)
		return false;
	*u = pwd->pw_uid;
	return true;
}

bool get_gid_of(const char *name, gid_t *g)
{
	struct group *grp = getgrnam(name);
	if(grp == NULL)
		return false;
	*g = grp->gr_gid;
	return true;
}


bool is_writable(const char *file) //throw(ExBasic)
{
	struct stat stat_buf;
	if(stat(file, &stat_buf) != 0) {
		//throw(ExBasic("file %s not found. ", file));
		return false;
	}
	gid_t g;
	return (get_gid_of("portage", &g)
			&& (stat_buf.st_mode & (S_IWGRP|S_IRGRP)) == (S_IWGRP|S_IRGRP)
			&& stat_buf.st_gid == g );
}

bool is_dir(const char *file)
{
	struct stat stat_buf;
	if(stat(file, &stat_buf) != 0)
		return false;
	return S_ISDIR(stat_buf.st_mode);
}

/** Return mtime of file. */
time_t get_mtime(const char *file)
{
	struct stat stat_b;
	if(stat(file, &stat_b))
		return 0;
	return stat_b.st_mtime;
}

/** @return mydate formatted according to locales and dateFormat */
const char *date_conv(const char *dateFormat, time_t mydate)
{
	const int max_datelen=256;
	static char buffer[max_datelen];
	string old_lcall = setlocale(LC_ALL, NULL);
	setlocale(LC_ALL, "");
	struct tm *loctime = localtime (&mydate);
	strftime(buffer, max_datelen, dateFormat, loctime);
	setlocale(LC_ALL, old_lcall.c_str());
	return buffer;
}
