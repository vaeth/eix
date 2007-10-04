// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

#ifndef __EBUILD_H__
#define __EBUILD_H__

#include <portage/cache/base.h>
#include <string>

class Package;
class Version;

class EbuildCache : public BasicCache {
		friend void ebuild_sig_handler(int sig);
	private:
		static EbuildCache *handler_arg;
		bool have_set_signals;
		std::string *cachefile;
		typedef void signal_handler(int sig);
		signal_handler *handleTERM, *handleINT, *handleHUP;
		bool use_ebuild_sh;

		void add_handler();
		void remove_handler();
		bool make_tempfile();
		bool make_cachefile(const char *name, const std::string &dir, const Package &package, const Version &version);
		void delete_cachefile();
	public:
		EbuildCache(bool use_sh = false) : BasicCache(), have_set_signals(false), cachefile(NULL), use_ebuild_sh(use_sh)
		{ }

		~EbuildCache()
		{ delete_cachefile(); }

		void readPackage(Category &vec, const char *pkg_name, std::string *directory_path, struct dirent **list, int numfiles) throw(ExBasic);
		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return true; }

		bool use_prefixexec() const
		{ return true; }

		const char *getType() const
		{
			if(use_ebuild_sh)
				return "ebuild*";
			return "ebuild";
		}
};

#endif /* __EBUILD_H__ */
