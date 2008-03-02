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

#include <string>

class BasicCache;
class Package;
class Version;

class EbuildExec {
		friend void ebuild_sig_handler(int sig);
	private:
		const BasicCache *base;
		static EbuildExec *handler_arg;
		bool have_set_signals;
		std::string *cachefile;
		typedef void signal_handler(int sig);
		signal_handler *handleTERM, *handleINT, *handleHUP;
		bool use_ebuild_sh;

		void add_handler();
		void remove_handler();
		bool make_tempfile();
	public:
		std::string *make_cachefile(const char *name, const std::string &dir, const Package &package, const Version &version);
		void delete_cachefile();

		EbuildExec(bool will_use_sh, const BasicCache *b) :
			base(b),
			have_set_signals(false),
			cachefile(NULL),
			use_ebuild_sh(will_use_sh)
		{ }

		~EbuildExec()
		{ delete_cachefile(); }

		bool use_sh() const
		{ return use_ebuild_sh; }
};

#endif /* __EBUILD_EXEC_H__ */
