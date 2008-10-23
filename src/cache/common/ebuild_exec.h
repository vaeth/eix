// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __EBUILD_EXEC_H__
#define __EBUILD_EXEC_H__

#include <eixTk/sysutils.h>

#include <string>
#include <vector>

class BasicCache;
class Package;
class Version;

class EbuildExec {
		friend void ebuild_sig_handler(int sig);
	private:
		const BasicCache *base;
		static EbuildExec *handler_arg;
		volatile bool have_set_signals, got_exit_signal, cache_defined;
		volatile int type_of_exit_signal;
		std::string cachefile;
		typedef void signal_handler(int sig);
		// cache/common/ebuild_exec.h|30| error: ignoring 'volatile' qualifiers added to function type 'void ()(int)'
		/* volatile */ signal_handler *handleTERM, *handleINT, *handleHUP;
		bool use_ebuild_sh;
		/// local data for make_cachefile which should be saved for vfork:
		const char *exec_name;
		const char **c_env;
		int exec_status;
		std::vector<std::string> *envstrings;
		void calc_environment(const char *name, const std::string &dir, const Package &package, const Version &version);

		static std::string exec_ebuild, exec_ebuild_sh, ebuild_depend_temp;
		static std::string portage_rootpath, portage_bin_path;
		static bool know_settings, set_uid, set_gid;
		static uid_t uid;
		static gid_t gid;

		void add_handler();
		void remove_handler();
		bool make_tempfile();
		static void calc_settings();
	public:
		std::string *make_cachefile(const char *name, const std::string &dir, const Package &package, const Version &version);
		void delete_cachefile();

		EbuildExec(bool will_use_sh, const BasicCache *b) :
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

#endif /* __EBUILD_EXEC_H__ */
