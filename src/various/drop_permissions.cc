// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "drop_permissions.h"
#include <eixTk/null.h>
#include <eixTk/sysutils.h>
#include <eixrc/eixrc.h>

#include <string>

#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_INTERIX_SECURITY_H
#include <interix/security.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

using namespace std;

void
drop_permissions(EixRc &eix)
{
	bool set_uid(true);
	bool valid_user(true);
	uid_t uid;
	const string &user(eix["EIX_USER"]);
	if(user.empty() || !get_uid_of(user.c_str(), &uid)) {
		valid_user = false;
		uid_t i(eix.getInteger("EIX_UID"));
		if(i > 0) {
			uid = i;
		}
		else {
			set_uid = false;
		}
	}
	bool set_gid(true);
	gid_t gid;
	const string &group(eix["EIX_GROUP"]);
	if(group.empty() || (get_uid_of(group.c_str(), &gid) == 0)) {
		gid_t i(eix.getInteger("EIX_GID"));
		if(i > 0) {
			gid = i;
		}
		else {
			set_gid = false;
		}
	}
#ifdef HAVE_SETUSER
	if(valid_user) {
		setuser(user.c_str(), NULLPTR, SU_COMPLETE);
	}
#endif
	if(set_gid) {
#ifdef HAVE_SETGID
		setgid(gid);
#endif
#ifdef HAVE_SETEGID
		setegid(gid);
#endif
#ifdef HAVE_SETGROUPS
		setgroups(1, &gid);
#endif
#ifdef HAVE_INITGROUPS
		if(valid_user) {
			initgroups(user.c_str(), gid);
		}
#endif
	}
	if(set_uid) {
#ifdef HAVE_SETUID
		setuid(uid);
#endif
#ifdef HAVE_SETEUID
		seteuid(uid);
#endif
	}
}

