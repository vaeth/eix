// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>
#include <vector>

#include "cache/base.h"
#include "cache/common/ebuild_exec.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/sysutils.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "portage/conf/portagesettings.h"

extern char **environ;

using std::map;
using std::string;
using std::vector;

class EbuildExecSettings {
public:
	string exec_ebuild, exec_ebuild_sh, ebuild_depend_temp;
	string portage_rootpath, portage_bin_path;

	void init();
};

EbuildExec *EbuildExec::handler_arg;

void
ebuild_sig_handler(int sig)
{
	EbuildExec::handler_arg->got_exit_signal = true;
	EbuildExec::handler_arg->type_of_exit_signal = sig;
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
	handler_arg = this;
	// Set the signals "empty" to avoid a race condition:
	// On a signal, we should cleanup only the signals actually set.
	got_exit_signal = false;
#ifdef HAVE_SIGACTION
	sigaction(SIGHUP, NULLPTR, &handleHUP);
	sigaction(SIGHUP, NULLPTR, &handleINT);
	sigaction(SIGHUP, NULLPTR, &handleTERM);
	have_set_signals = true;
	m_handler.sa_handler = ebuild_sig_handler;
	m_handler.sa_flags = 0;
	sigemptyset(&(m_handler.sa_mask));
	if((handleHUP.sa_handler != SIG_IGN)
#ifdef SA_SIGINFO
		|| ((handleHUP.sa_flags & SA_SIGINFO) != 0)
#endif
	)
		sigaction(SIGHUP, &m_handler, NULLPTR);
	if((handleINT.sa_handler != SIG_IGN)
#ifdef SA_SIGINFO
		|| ((handleINT.sa_flags & SA_SIGINFO) != 0)
#endif
	)
		sigaction(SIGINT, &m_handler, NULLPTR);
	if((handleTERM.sa_handler != SIG_IGN)
#ifdef SA_SIGINFO
		|| ((handleTERM.sa_flags & SA_SIGINFO) != 0)
#endif
	)
		sigaction(SIGTERM, &m_handler, NULLPTR);
#else
	// ifndef HAVE_SIGACTION
	handleHUP  = signal(SIGHUP, SIG_IGN);
	handleINT  = signal(SIGINT, SIG_IGN);
	handleTERM = signal(SIGTERM, SIG_IGN);
	have_set_signals = true;
	if(handleHUP != SIG_IGN)
		signal(SIGHUP, ebuild_sig_handler);
	if(handleINT != SIG_IGN)
		signal(SIGINT, ebuild_sig_handler);
	if(handleTERM != SIG_IGN)
		signal(SIGTERM, ebuild_sig_handler);
#endif
#pragma GCC diagnostic pop
}

void
EbuildExec::remove_handler()
{
	if(!have_set_signals)
		return;
#ifdef HAVE_SIGACTION
	sigaction(SIGHUP,  &handleHUP,  NULLPTR);
	sigaction(SIGINT,  &handleHUP,  NULLPTR);
	sigaction(SIGTERM, &handleTERM, NULLPTR);
#else
	signal(SIGHUP,  handleHUP);
	signal(SIGINT,  handleINT);
	signal(SIGTERM, handleTERM);
#endif
	have_set_signals = false;
}

// You should have called add_handler() in advance
bool
EbuildExec::make_tempfile()
{
	char *temp(new char[256]);
	if(unlikely(temp == NULLPTR))
		return false;
	strcpy(temp, "/tmp/ebuild-cache.XXXXXXXX");  // NOLINT(runtime/printf)
	int fd(mkstemp(temp));
	if(fd == -1) {
		delete temp;
		return false;
	}
	calc_settings();
	cachefile = temp;
	cache_defined = true;
	delete temp;
	close(fd);
	return true;
}

void
EbuildExec::delete_cachefile()
{
	if(unlikely(!cache_defined))
		return;
	const char *c(cachefile.c_str());
	if(is_pure_file(c)) {
		if(unlink(c) < 0)
			base->m_error_callback(eix::format(_("Can't unlink tempfile %s")) % c);
		else if(is_file(c))
			base->m_error_callback(eix::format(_("Tempfile %s still there after unlink")) % c);
	} else {
		base->m_error_callback(eix::format(_("Tempfile %s is not a file")) % c);
	}
	remove_handler();
	cache_defined = false;
	cachefile.clear();
}

/// This is a subfunction of make_cachefile() to ensure that make_cachefile()
/// has no local variable when vfork() is called.
void
EbuildExec::calc_environment(const char *name, const string &dir, const Package &package, const Version &version)
{
	c_env = NULLPTR;
	envstrings = NULLPTR;
#ifdef HAVE_SETENV
	if(!use_ebuild_sh)
		return;
#endif
	map<string, string> env;
#ifndef HAVE_SETENV
	if(!use_ebuild_sh) {
		if(!(base->portagesettings->export_portdir_overlay))
			return;
		for(char **e(environ); likely(*e != NULLPTR); ++e) {
			const char *s(strchr(*e, '='));
			if(likely(s != NULLPTR))
				env[string(*e, s - (*e))] = s + 1;
		}
		env["PORTDIR_OVERLAY"] = (*(base->portagesettings))["PORTDIR_OVERLAY"].c_str();
	} else  // NOLINT(readability/braces)
#endif
	{
		base->env_add_package(env, package, version, dir, name);
		env["dbkey"] = cachefile;
		const string &portage_rootpath(settings->portage_rootpath);
		if(likely(!portage_rootpath.empty())) {
			env["PORTAGE_ROOTPATH"] = portage_rootpath;
		}
		const string &portage_bin_path(settings->portage_bin_path);
		if(likely(!portage_bin_path.empty())) {
			env["PORTAGE_BIN_PATH"] = portage_bin_path;
		}
	}

	// transform env into c_env (pointing to envstrings[i].c_str())
	c_env = new const char *[env.size() + 1];
	vector<string>::size_type i(0);
	if(!env.empty()) {
		envstrings = new vector<string>(env.size());
		for(map<string, string>::const_iterator it = env.begin();
			it != env.end(); ++it) {
			(*envstrings)[i] = ((it->first) + '=' + (it->second));
			c_env[i] = (*envstrings)[i].c_str();
			++i;
		}
	}
	c_env[i] = NULLPTR;
}

static const int EXECLE_FAILED = 17;

string *
EbuildExec::make_cachefile(const char *name, const string &dir, const Package &package, const Version &version)
{
	calc_settings();

	// Make cachefile and calculate exec_name

	add_handler();
	if(use_ebuild_sh) {
		exec_name = settings->exec_ebuild_sh.c_str();
		if(!make_tempfile()) {
			base->m_error_callback(_("Creation of tempfile failed"));
			remove_handler();
			return NULLPTR;
		}
	} else {
		exec_name = settings->exec_ebuild.c_str();
		cachefile = settings->ebuild_depend_temp;
		cache_defined = true;
	}
	calc_environment(name, dir, package, version);

#ifdef HAVE_VFORK
	pid_t child = vfork();
#else
	pid_t child = fork();
#endif
	if(child == -1) {
		base->m_error_callback(_("Forking failed"));
		return NULLPTR;
	}
	if(child == 0)
	{
		if(use_ebuild_sh) {
			execle(exec_name, exec_name, "depend", static_cast<const char *>(NULLPTR), c_env);
		} else {
#ifndef HAVE_SETENV
			if(c_env != NULLPTR)
				execle(exec_name, exec_name, name, "depend", static_cast<const char *>(NULLPTR), c_env);
			else
#endif
				execl(exec_name, exec_name, name, "depend", static_cast<const char *>(NULLPTR));
		}
		_exit(EXECLE_FAILED);
	}
	while(waitpid(child, &exec_status, 0) != child ) { }

	// Free memory needed only for the child process:
	delete c_env;
	delete envstrings;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
	// Only now we check for the child exit status or signals:
	if(got_exit_signal) {
		base->m_error_callback(eix::format(_("Got signal %s")) % type_of_exit_signal);
	} else if(WIFSIGNALED(exec_status)) {
		got_exit_signal = true;
		type_of_exit_signal = WTERMSIG(exec_status);
		base->m_error_callback(eix::format(_("Ebuild got signal %s")) % type_of_exit_signal);
	}
	if(got_exit_signal) {
		delete_cachefile();
		raise(type_of_exit_signal);
		return NULLPTR;
	}
	if(WIFEXITED(exec_status)) {
		if(!(WEXITSTATUS(exec_status)))  // the only good case:
			return &cachefile;
		if((WEXITSTATUS(exec_status)) == EXECLE_FAILED)
			base->m_error_callback(eix::format(_("Could not start %s")) % exec_name);
		else
			base->m_error_callback(eix::format(_("Ebuild failed with status %s")) % WEXITSTATUS(exec_status));
	} else {
		base->m_error_callback(_("Child aborted in a strange way"));
	}
#pragma GCC diagnostic pop
	delete_cachefile();
	return NULLPTR;
}

void
EbuildExecSettings::init()
{
	EixRc &eix(get_eixrc());
	exec_ebuild = eix["EXEC_EBUILD"];
	exec_ebuild_sh = eix["EXEC_EBUILD_SH"];
	ebuild_depend_temp = eix["EBUILD_DEPEND_TEMP"];
	portage_rootpath = eix["PORTAGE_ROOTPATH"];
	portage_bin_path = eix["PORTAGE_BIN_PATH"];
}

EbuildExecSettings *EbuildExec::settings = NULLPTR;

void
EbuildExec::calc_settings()
{
	if(unlikely(settings == NULLPTR)) {
		settings = new EbuildExecSettings;
		settings->init();
	}
}
