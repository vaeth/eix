// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_CACHE_COMMON_EBUILD_EXEC_H_
#define SRC_CACHE_COMMON_EBUILD_EXEC_H_ 1

#include <csignal>

#include <string>
#include <vector>

class EbuildExecSettings;
class BasicCache;
class Package;
class Version;

void ebuild_sig_handler(int sig) ATTRIBUTE_SIGNAL;

class EbuildExec {
		friend void ebuild_sig_handler(int sig) ATTRIBUTE_SIGNAL;

	private:
		const BasicCache *base;
		static EbuildExec *handler_arg;
		volatile bool have_set_signals, got_exit_signal, cache_defined;
		volatile int type_of_exit_signal;
		std::string cachefile;
#ifdef HAVE_SIGACTION
		struct sigaction handleTERM, handleINT, handleHUP, m_handler;
#else
		typedef void signal_handler(int sig);
		// cache/common/ebuild_exec.h|30| error: ignoring 'volatile' qualifiers added to function type 'void ()(int)'
		/* volatile */ signal_handler *handleTERM, *handleINT, *handleHUP;
#endif
		bool use_ebuild_sh;
		/// local data for make_cachefile which should be saved for vfork:
		const char *exec_name;
		const char **c_env;
		int exec_status;
		std::vector<std::string> *envstrings;
		void calc_environment(const char *name, const std::string &dir, const Package &package, const Version &version) ATTRIBUTE_NONNULL_;

		static EbuildExecSettings *settings;

		void add_handler();
		void remove_handler();
		bool make_tempfile();
		static void calc_settings();

	public:
		std::string *make_cachefile(const char *name, const std::string &dir, const Package &package, const Version &version) ATTRIBUTE_NONNULL_;
		void delete_cachefile();

		EbuildExec(bool will_use_sh, const BasicCache *b) ATTRIBUTE_NONNULL_ :
			base(b),
			have_set_signals(false),
			cache_defined(false),
			use_ebuild_sh(will_use_sh)
		{ }

		~EbuildExec()
		{ delete_cachefile(); }

		bool use_sh() const
		{ return use_ebuild_sh; }
};

#endif  // SRC_CACHE_COMMON_EBUILD_EXEC_H_
