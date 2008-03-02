// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "ebuild_exec.h"

#include <cache/base.h>
#include <global.h>

#include <config.h>
#include <csignal>
#include <sys/wait.h>
#include <sys/stat.h>

using namespace std;

const char *EBUILD_SH_EXEC     = "/usr/lib/portage/bin/ebuild.sh";
const char *EBUILD_EXEC        = "/usr/bin/ebuild";
const char *EBUILD_DEPEND_TEMP = "/var/cache/edb/dep/aux_db_key_temp";


EbuildExec *EbuildExec::handler_arg;
void
ebuild_sig_handler(int sig)
{
	UNUSED(sig);
	EbuildExec::handler_arg->delete_cachefile();
}

// Take care:
// Since handler_arg is static, add_handler will not be reentrant,
// even for different instances of EbuildExec.
// However, as long as add_handler() and remove_handler() are
// only called internally within the same public function
// of EbuildExec (and EbuildExec does not call other instances),
// this is not a problem from "outside" this class.

void
EbuildExec::add_handler()
{
	handler_arg = this;
	// Set the signals "empty" to avoid a race condition:
	// On a signal, we should cleanup only the signals actually set.
	handleHUP  = handleINT = handleTERM = ebuild_sig_handler;
	have_set_signals = true;
	handleHUP  = signal(SIGHUP, ebuild_sig_handler);
	if(handleHUP == SIG_IGN)
		signal(SIGHUP, SIG_IGN);
	handleINT  = signal(SIGINT, ebuild_sig_handler);
	if(handleINT == SIG_IGN)
		signal(SIGINT, SIG_IGN);
	handleTERM = signal(SIGTERM, ebuild_sig_handler);
	if(handleTERM == SIG_IGN)
		signal(SIGTERM, SIG_IGN);
}

void
EbuildExec::remove_handler()
{
	if(!have_set_signals)
		return;
	signal(SIGHUP,  handleHUP);
	signal(SIGINT,  handleINT);
	signal(SIGTERM, handleTERM);
	have_set_signals = false;
}

// You should have called add_handler() in advance
bool
EbuildExec::make_tempfile()
{
	char *temp = static_cast<char *>(malloc(256 * sizeof(char)));
	if(!temp)
		return false;
	strcpy(temp, "/tmp/ebuild-cache.XXXXXX");
	int fd = mkstemp(temp);
	if(fd == -1)
		return false;
	calc_permissions();
	if(set_uid || set_gid)
		fchown(fd, (set_uid ? uid : -1) , (set_gid ? gid : -1));
	cachefile = new string(temp);
	free(temp);
	close(fd);
	return true;
}

void
EbuildExec::delete_cachefile()
{
	if(!cachefile)
		return;
	if(unlink(cachefile->c_str()) < 0)
		base->m_error_callback(eix::format("Can't unlink %s") % (*cachefile));
	cachefile = NULL;
	remove_handler();
}

string *
EbuildExec::make_cachefile(const char *name, const string &dir, const Package &package, const Version &version)
{
	map<string, string> env;
	add_handler();
	if(use_ebuild_sh)
	{
		if(!make_tempfile())
		{
			remove_handler();
			return NULL;
		}
		base->env_add_package(env, package, version, dir, name);
		env["dbkey"] = *cachefile;
	}
	else
		cachefile = new string(base->m_prefix_exec + EBUILD_DEPEND_TEMP);
	calc_permissions();
#if defined(HAVE_VFORK)
	pid_t child = vfork();
#else
	pid_t child = fork();
#endif
	if(child == -1) {
		base->m_error_callback("Forking failed");
		return NULL;
	}
	if(child == 0)
	{
		if(set_gid)
			setgid(gid);
		if(set_uid)
			setuid(uid);
		if(!use_ebuild_sh)
		{
			string ebuild = base->m_prefix_exec + EBUILD_EXEC;
			execl(ebuild.c_str(), ebuild.c_str(), name, "depend", static_cast<const char *>(NULL));
			exit(2);
		}
		const char **myenv = static_cast<const char **>(malloc((env.size() + 1) * sizeof(const char *)));
		const char **ptr = myenv;
		for(map<string, string>::const_iterator it = env.begin();
			it != env.end(); ++it, ++ptr) {
			string *s = new string((it->first) + '=' + (it->second));
			*ptr = s->c_str();
		}
		*ptr = NULL;
		string ebuild_sh = base->m_prefix_exec + EBUILD_SH_EXEC;
		execle(ebuild_sh.c_str(), ebuild_sh.c_str(), "depend", static_cast<const char *>(NULL), myenv);
		exit(2);
	}
	int exec_status;
	while( waitpid( child, &exec_status, 0) != child ) ;
	if(exec_status)
	{
		delete_cachefile();
		return NULL;
	}
	return cachefile;
}

bool EbuildExec::know_permissions = false;
bool EbuildExec::set_uid, EbuildExec::set_gid;
uid_t EbuildExec::uid;
gid_t EbuildExec::gid;

void
EbuildExec::calc_permissions()
{
	if(know_permissions)
		return;
	know_permissions = set_uid = set_gid = true;
	EixRc &eix = get_eixrc(NULL);
	string &s = eix["EBUILD_USER"];
	if(s.empty() || !get_uid_of(s.c_str(), &uid)) {
		int i = eix.getInteger("EBUILD_UID");
		if(i > 0)
			uid = i;
		else
			set_uid = false;
	}
	s = eix["EBUILD_GROUP"];
	if(s.empty() || !get_uid_of(s.c_str(), &gid)) {
		int i = eix.getInteger("EBUILD_GID");
		if(i > 0)
			gid = i;
		else
			set_gid = false;
	}
}
