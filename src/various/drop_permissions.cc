// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "various/drop_permissions.h"
#include <config.h>  // IWYU pragma: keep

#include <unistd.h>
#include <sys/types.h>  // IWYU pragma: keep

#ifdef HAVE_INTERIX_SECURITY_H
#include <interix/security.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#include <string>

#include "eixTk/eixint.h"
#include "eixTk/i18n.h"
#ifdef HAVE_SETUSER
#include "eixTk/null.h"
#endif
#include "eixTk/sysutils.h"
#include "eixrc/eixrc.h"

using std::string;

#ifdef NEED_SETUID_PROTO
int setuid(uid_t uid);
#endif
#ifdef NEED_SETEUID_PROTO
int seteuid(uid_t uid);
#endif
#ifdef NEED_SETGID_PROTO
int setgid(gid_t gid);
#endif
#ifdef NEED_SETEGID_PROTO
int setegid(gid_t gid);
#endif
#ifdef NEED_GETUID_PROTO
uid_t getuid();
#endif
#ifdef NEED_GETEUID_PROTO
uid_t geteuid();
#endif
#ifdef NEED_GETGID_PROTO
gid_t getgid();
#endif
#ifdef NEED_GETEGID_PROTO
gid_t getegid();
#endif

static bool drop_permissions(EixRc *eix);

bool drop_permissions(EixRc *eix, string *errtext) {
	if(drop_permissions(eix)) {
		errtext->clear();
		return true;
	}
	if(eix->getBool("NODROP_FATAL")) {
		errtext->assign(_("failed to drop permissions"));
		return false;
	}
	errtext->assign(_("warning: failed to drop permissions"));
	return true;
}

static bool drop_permissions(EixRc *eix) {
	bool set_uid(true);
	bool valid_user(true);
	uid_t uid;
	const string& user((*eix)["EIX_USER"]);
	if(user.empty() || !get_uid_of(user.c_str(), &uid)) {
		valid_user = false;
		uid_t i(eix->getInteger("EIX_UID"));
		if(i > 0) {
			uid = i;
		} else {
			set_uid = false;
		}
	}
	bool set_gid(true);
	gid_t gid;
	const string& group((*eix)["EIX_GROUP"]);
	if(group.empty() || !get_gid_of(group.c_str(), &gid)) {
		gid_t i(eix->getInteger("EIX_GID"));
		if(i > 0) {
			gid = i;
		} else {
			set_gid = false;
		}
	}
	bool success(true);
	bool force_success; {
		eix::SignedBool w(eix->getBoolText("REQUIRE_DROP", "root"));
		force_success = ((w < 0) ?
#ifdef HAVE_GETUID
			(getuid() != 0)
#else
			true
#endif
			: (w == 0));
	}

#ifdef HAVE_SETUSER
	if(valid_user) {
		if(setuser(user.c_str(), NULLPTR, SU_COMPLETE)) {
			success = false;
		} else {
			force_success = true;
		}
	}
#endif
	if(set_gid) {
#if defined(HAVE_SETGID) && !defined(BROKEN_SETGID)
		{  // NOLINT(whitespace/braces)
#if defined(HAVE_GETGID) && !defined(BROKEN_GETGID)
			bool forcing_success(force_success || (getgid() == gid));
#endif
			if(setgid(gid)) {
#if defined(HAVE_GETGID) && !defined(BROKEN_GETGID)
				if(!forcing_success)
#endif
				success = false;
			}
		}
#endif
#if defined(HAVE_SETEGID) && !defined(BROKEN_SETEGID)
		{  // NOLINT(whitespace/braces)
#if defined(HAVE_GETEGID) && !defined(BROKEN_GETEGID)
			bool forcing_success(force_success || (getegid() == gid));
#endif
			if(setegid(gid)) {
#if defined(HAVE_GETEGID) && !defined(BROKEN_GETEGID)
				if(!forcing_success)
#endif
				success = false;
			}
		}
#endif
#ifdef HAVE_SETGROUPS
		// This is expected to fail if we are already uid
		setgroups(1, &gid);
#endif
#ifdef HAVE_INITGROUPS
		if(valid_user) {
			// This is expected to fail if we are already uid
			initgroups(user.c_str(), gid);
		}
#endif
	}
	if(set_uid) {
#if defined(HAVE_SETUID) && !defined(BROKEN_SETUID)
		{  // NOLINT(whitespace/braces)
#if defined(HAVE_GETUID) && !defined(BROKEN_GETUID)
			bool forcing_success(force_success || (getuid() == uid));
#endif
			if(setuid(uid)) {
#if defined(HAVE_GETUID) && !defined(BROKEN_GETUID)
				if(!forcing_success)
#endif
				success = false;
			}
		}
#endif
#if defined(HAVE_SETEUID) && !defined(BROKEN_SETEUID)
		{  // NOLINT(whitespace/braces)
#if defined(HAVE_GETEUID) && !defined(BROKEN_GETEUID)
			bool forcing_success(force_success || (geteuid() == uid));
#endif
			if(seteuid(uid)) {
#if defined(HAVE_GETEUID) && !defined(BROKEN_GETEUID)
				if(!forcing_success)
#endif
				success = false;
			}
		}
#endif
	}
	return (force_success || success);
}
