// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <grp.h>
#include <pwd.h>
// unistd.h is needed on Solaris for including stropts.h, see below
// _exit; make check_includes happy
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_STROPTS_H
// Needed on Solaris for ioctl() https://bugs.gentoo.org/show_bug.cgi?id=510120
#include <stropts.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#ifdef HAVE_SYS_PTEM_H
#include <sys/ptem.h>
#endif
#ifdef HAVE_SYS_TTY_H
#include <sys/tty.h>
#endif
#ifdef HAVE_SYS_PTY_H
#include <sys/pty.h>
#endif

#include <clocale>
#include <ctime>

#include <string>

#include "eixTk/constexpr.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/sysutils.h"
#include "eixTk/unused.h"

using std::string;

/**
Get uid of a user.
@param u pointer to uid_t .. uid is stored there.
@param name name of user
@return true if user exists
**/
bool get_uid_of(const char *name, uid_t *u) {
	struct passwd ps, *pwd;
	char c[5000];
	if((getpwnam_r(name, &ps, c, 4999, &pwd) != 0) || (pwd == NULLPTR)) {
		return false;
	}
	*u = pwd->pw_uid;
	return true;
}

/**
Get gid of a group.
@param g pointer to gid_t .. gid is stored there.
@param name name of group
@return true if group exists
**/
bool get_gid_of(const char *name, gid_t *g) {
	struct group gs, *grp;
	char c[5000];
	if((getgrnam_r(name, &gs, c, 4999, &grp) != 0) || (grp == NULLPTR)) {
		return false;
	}
	*g = grp->gr_gid;
	return true;
}

/**
@return true if file is a directory or a symlink to some.
**/
bool is_dir(const char *file) {
	struct stat stat_buf;
	if(unlikely(stat(file, &stat_buf) != 0))
		return false;
	return S_ISDIR(stat_buf.st_mode);
}

/**
@return true if file is a plain file or a symlink to some.
**/
bool is_file(const char *file) {
	struct stat stat_buf;
	if(unlikely(stat(file, &stat_buf) != 0))
		return false;
	return S_ISREG(stat_buf.st_mode);
}

/**
@return true if file is a plain file (and not a symlink).
**/
bool is_pure_file(const char *file) {
	struct stat stat_buf;
	if(unlikely(lstat(file, &stat_buf) != 0))
		return false;
	return S_ISREG(stat_buf.st_mode);
}

/**
@return mtime of file.
**/
bool get_mtime(time_t *t, const char *file) {
	struct stat stat_b;
	if(unlikely(stat(file, &stat_b))) {
		return false;
	}
	*t = stat_b.st_mtime;
	return true;
}

/**
@return mydate formatted according to locales and dateFormat
**/
const char *date_conv(const char *dateFormat, time_t mydate) {
	static CONSTEXPR int max_datelen = 256;
	static char buffer[max_datelen];
	string old_lcall = setlocale(LC_ALL, NULLPTR);
	setlocale(LC_ALL, "");
	struct tm *loctime(localtime (&mydate));
	strftime(buffer, max_datelen, dateFormat, loctime);
	setlocale(LC_ALL, old_lcall.c_str());
	return buffer;
}

/**
@return true in case of success
**/
#ifdef TIOCGWINSZ
bool get_geometry(unsigned int *lines, unsigned int *columns) {
	struct winsize win;
	if(ioctl(1, TIOCGWINSZ, &win) == 0) {
		if((win.ws_row >= 0) && (win.ws_col >= 0)) {
			*lines = win.ws_row;
			*columns = win.ws_col;
			return true;
		}
	}
	return false;
}
#else
bool get_geometry(unsigned int *lines ATTRIBUTE_UNUSED, unsigned int *columns ATTRIBUTE_UNUSED) {
	UNUSED(lines);
	UNUSED(columns);
	return false;
}
#endif
