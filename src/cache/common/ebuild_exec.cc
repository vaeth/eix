// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "cache/common/ebuild_exec.h"
#include <config.h>  // IWYU pragma: keep

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <csignal>
#include <cstdlib>
#include <cstring>

#include <string>

#include "cache/base.h"
#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/eixarray.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "portage/conf/portagesettings.h"

extern char **environ;

using std::string;

class EbuildExecSettings {
		friend class EbuildExec;

	private:
#ifndef HAVE_SETENV
		string exec_ebuild;
#endif
		string ebuild_depend_temp, tmpdir;
		string portage_rootpath;
		string portage_bin_path, portage_pym_path, exec_ebuild_sh;
		bool read_portage_paths, know_portage_paths;

		void init();
		bool init_ebuild_sh(const EbuildExec *e);
};

EbuildExec *EbuildExec::handler_arg;

void ebuild_sig_handler(int sig) {
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

void EbuildExec::add_handler() {
GCC_DIAG_OFF(old-style-cast)
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
	handleHUP  = std::signal(SIGHUP, SIG_IGN);
	handleINT  = std::signal(SIGINT, SIG_IGN);
	handleTERM = std::signal(SIGTERM, SIG_IGN);
	have_set_signals = true;
	if(handleHUP != SIG_IGN) {
		std::signal(SIGHUP, ebuild_sig_handler);
	}
	if(handleINT != SIG_IGN) {
		std::signal(SIGINT, ebuild_sig_handler);
	}
	if(handleTERM != SIG_IGN) {
		std::signal(SIGTERM, ebuild_sig_handler);
	}
#endif
GCC_DIAG_ON(old-style-cast)
}

void EbuildExec::remove_handler() {
	if(!have_set_signals)
		return;
#ifdef HAVE_SIGACTION
	sigaction(SIGHUP,  &handleHUP,  NULLPTR);
	sigaction(SIGINT,  &handleHUP,  NULLPTR);
	sigaction(SIGTERM, &handleTERM, NULLPTR);
#else
	std::signal(SIGHUP,  handleHUP);
	std::signal(SIGINT,  handleINT);
	std::signal(SIGTERM, handleTERM);
#endif
	have_set_signals = false;
}

// You should have called add_handler() in advance
int EbuildExec::make_tempfile() {
	const string &tmpdir = settings->tmpdir;
	string::size_type l(tmpdir.size());
	char *temp = new char[256 + l];
	if(l == 0) {
		std::strcpy(temp, "/tmp/ebuild-cache.XXXXXXXX");  // NOLINT(runtime/printf)
	} else {
		std::strcpy(temp, tmpdir.c_str());  // NOLINT(runtime/printf)
		std::strcpy(temp + l, "/ebuild-cache.XXXXXXXX");  // NOLINT(runtime/printf)
	}
	int fd(mkstemp(temp));
	if(fd == -1) {
		delete[] temp;
		return fd;
	}
	cachefile.assign(temp);
	cache_defined = true;
	delete[] temp;
	return fd;
}

void EbuildExec::delete_cachefile() {
	if(unlikely(!cache_defined)) {
		return;
	}
	const char *c(cachefile.c_str());
	if(is_pure_file(c)) {
		if(unlink(c) < 0) {
			base->m_error_callback(eix::format(_("cannot unlink tempfile %s")) % c);
		} else if(is_file(c)) {
			base->m_error_callback(eix::format(_("tempfile %s still there after unlink")) % c);
		}
	} else {
		base->m_error_callback(eix::format(_("tempfile %s is not a file")) % c);
	}
	remove_handler();
	cache_defined = false;
	cachefile.clear();
}

/**
This is a subfunction of make_cachefile() to ensure that make_cachefile()
has no local variable when vfork() is called.
**/
void EbuildExec::calc_environment(const char *name, const string& dir, const Package& package, const Version& version, const string& eapi, int fd) {
	c_env = NULLPTR;
	envstrings = NULLPTR;
	// non-sh: environment is kept except for possibly new PORTDIR_OVERLAY
	if(!use_ebuild_sh) {  // Shortcut if this is done globally or undesired
#ifndef HAVE_SETENV
		if(!(base->portagesettings->export_portdir_overlay))
#endif
		return;
	}
	WordIterateMap env;
#ifndef HAVE_SETENV
	if(!use_ebuild_sh) {
		for(char **e(environ); likely(*e != NULLPTR); ++e) {
			const char *s(std::strchr(*e, '='));
			if(likely(s != NULLPTR)) {
				env[string(*e, s - (*e))] = s + 1;
			}
		}
	} else  // NOLINT(readability/braces)
	// ifndef HAVE_SETENV if(!use_ebuild_sh) we have already returned
#endif
	// if(use_ebuild_sh)
	{  // NOLINT(whitespace/braces)
		base->env_add_package(&env, package, version, dir, name);
		env["dbkey"] = cachefile;
		const string& portage_rootpath(settings->portage_rootpath);
		if(likely(!portage_rootpath.empty())) {
			env["PORTAGE_ROOTPATH"] = portage_rootpath;
		}
		env["EAPI"] = eapi;
		env["PORTAGE_BIN_PATH"] = settings->portage_bin_path;
		env["PORTAGE_PYM_PATH"] = settings->portage_pym_path;
		env["PORTAGE_REPO_NAME"] = base->getOverlayName();
		if(fd != -1) {
			env["PORTAGE_PIPE_FD"] = eix::format("%d") % fd;
		}
		WordVec eclasses;
		eclasses.PUSH_BACK(base->getPrefixedPath());
		RepoList& repos(base->portagesettings->repos);
		// eclasses.PUSH_BACK((*(base->portagesettings))["PORTDIR"]);
		// for(RepoList::const_iterator it(repos.second());
		for(RepoList::const_iterator it(repos.begin());
			likely(it != repos.end()); ++it) {
			if(likely(it->path != eclasses[0])) {
				eclasses.PUSH_BACK(it->path);
			}
		}
		for(WordVec::iterator it(eclasses.begin());
			likely(it != eclasses.end()); ++it) {
			escape_string(&*it, shellspecial);
		}
		join_to_string(&env["PORTAGE_ECLASS_LOCATIONS"], eclasses);
	}
	env["PORTDIR_OVERLAY"] = (*(base->portagesettings))["PORTDIR_OVERLAY"].c_str();
	if(settings->tmpdir.empty()) {
		WordIterateMap::iterator i(env.find("TMPDIR"));
		if(i != env.end()) {
			env.erase(i);
		}
	} else {
		env["TMPDIR"] = settings->tmpdir;
	}

	// transform env into c_env (pointing to envstrings[i].c_str())
	c_env = new const char *[env.size() + 1];
	WordVec::size_type i(0);
	if(!env.empty()) {
		envstrings = new WordVec(env.size());
		for(WordIterateMap::const_iterator it(env.begin());
			likely(it != env.end()); ++it) {
			(*envstrings)[i] = ((it->first) + '=' + (it->second));
			c_env[i] = (*envstrings)[i].c_str();
			++i;
		}
	}
	c_env[i] = NULLPTR;
}

static CONSTEXPR const int EXECLE_FAILED = 127;

string *EbuildExec::make_cachefile(const char *name, const string& dir, const Package& package, const Version& version, const string& eapi) {
	if(unlikely(!calc_settings())) {
		return NULLPTR;
	}

	// Make cachefile and calculate exec_name

	add_handler();
	int fd = -1;
	if(use_ebuild_sh) {
		exec_name = settings->exec_ebuild_sh.c_str();
		fd = make_tempfile();
		if(fd == -1) {
			base->m_error_callback(_("creation of tempfile failed"));
			remove_handler();
			return NULLPTR;
		}
	} else {
		exec_name = "ebuild";
		cachefile = settings->ebuild_depend_temp;
		cache_defined = true;
	}
	calc_environment(name, dir, package, version, eapi, fd);
#ifndef HAVE_SETENV
	if((!use_ebuild_sh) && (c_env != NULLPTR)) {
		exec_name = settings->exec_ebuild.c_str();
	}
#endif

#ifdef HAVE_VFORK
	pid_t child = vfork();
#else
	pid_t child = fork();
#endif
	if(unlikely(child == -1)) {
		base->m_error_callback(_("forking failed"));
		return NULLPTR;
	}
	if(child == 0) {
		if(use_ebuild_sh) {
			execle(exec_name, exec_name, "depend", static_cast<const char *>(NULLPTR), c_env);
		} else {
#ifndef HAVE_SETENV
			if(c_env != NULLPTR)
				execle(exec_name, exec_name, name, "depend", static_cast<const char *>(NULLPTR), c_env);
			else
#endif
				execlp(exec_name, exec_name, name, "depend", static_cast<const char *>(NULLPTR));
		}
		_exit(EXECLE_FAILED);
	}
	while(waitpid(child, &exec_status, 0) != child ) { }
	if(fd != -1) {
		close(fd);
	}

	// Free memory needed only for the child process:
	delete[] c_env;
	delete envstrings;

GCC_DIAG_OFF(old-style-cast)
	// Only now we check for the child exit status or signals:
	if(unlikely(got_exit_signal)) {
		base->m_error_callback(eix::format(_("got signal %s")) % type_of_exit_signal);
	} else if(unlikely(WIFSIGNALED(exec_status))) {
		got_exit_signal = true;
		type_of_exit_signal = WTERMSIG(exec_status);
		base->m_error_callback(eix::format(_("ebuild got signal %s")) % type_of_exit_signal);
	}
	if(unlikely(got_exit_signal)) {
		delete_cachefile();
		raise(type_of_exit_signal);
		return NULLPTR;
	}
	if(likely(WIFEXITED(exec_status))) {
		if(likely(!(WEXITSTATUS(exec_status)))) {  // the only good case:
			return &cachefile;
		}
		if((WEXITSTATUS(exec_status)) == EXECLE_FAILED) {
			base->m_error_callback(eix::format(_("could not start %s")) % exec_name);
		} else {
			base->m_error_callback(eix::format(_("ebuild failed with status %s")) % WEXITSTATUS(exec_status));
		}
	} else {
		base->m_error_callback(_("child aborted in a strange way"));
	}
GCC_DIAG_ON(old-style-cast)
	delete_cachefile();
	return NULLPTR;
}

bool EbuildExec::portageq(std::string *result, const char *var) const {
	int fds[2];

	if(unlikely(pipe(fds) != 0)) {
		base->m_error_callback(_("cannot setup pipe"));
		return false;
	}

	pid_t child = fork();

	if(unlikely(child == -1)) {
		base->m_error_callback(_("forking failed"));
		return false;
	}
	if(child == 0) {
		close(fds[0]);
		dup2(fds[1], 1);
		close(fds[1]);
		execlp("portageq", "portageq", "envvar", var, static_cast<const char *>(NULLPTR));
		_exit(EXECLE_FAILED);
	}
	close(fds[1]);
	eix::array<char, 8192> buffer;
	size_t curr;
	string res;
GCC_DIAG_OFF(sign-conversion)
	while((curr = read(fds[0], buffer.data(), buffer.size())),
		((curr > 0) && (curr <= buffer.size()))) {
		res.append(buffer.data(), curr);
	}
GCC_DIAG_ON(sign-conversion)
	close(fds[0]);
	int ret_status;
	while(waitpid(child, &ret_status, 0) != child ) { }
	if((ret_status != 0) || res.empty()) {
		base->m_error_callback(_("portageq failed"));
		return false;
	}
	result->assign(res, 0, res.size() - 1);
	return true;
}

void EbuildExecSettings::init() {
	EixRc& eix(get_eixrc());
	ebuild_depend_temp = eix["EBUILD_DEPEND_TEMP"];
	tmpdir = eix["EIX_TMPDIR"];
#ifndef HAVE_SETENV
	exec_ebuild = eix["EPREFIX_PORTAGE_EXEC"];
	exec_ebuild.append("/usr/bin/ebuild");
#endif
	exec_ebuild_sh = "ebuild.sh";
	portage_rootpath = eix["PORTAGE_ROOTPATH"];
	read_portage_paths = false;
}

bool EbuildExecSettings::init_ebuild_sh(const EbuildExec *e) {
	if(likely(read_portage_paths)) {
		return know_portage_paths;
	}
	read_portage_paths = true;
	if(unlikely(!e->portageq(&portage_bin_path, "PORTAGE_BIN_PATH")) ||
		unlikely(!e->portageq(&portage_pym_path, "PORTAGE_PYM_PATH"))) {
		know_portage_paths = false;
		return false;
	}
	exec_ebuild_sh = portage_bin_path;
	exec_ebuild_sh.append("/ebuild.sh");
	know_portage_paths = true;
	return true;
}

EbuildExecSettings *EbuildExec::settings = NULLPTR;

bool EbuildExec::calc_settings() {
	if(unlikely(settings == NULLPTR)) {
		settings = new EbuildExecSettings;
		settings->init();
	}
	if(!use_ebuild_sh) {
		return true;
	}
	return settings->init_ebuild_sh(this);
}
