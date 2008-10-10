// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __PARSECACHE_H__
#define __PARSECACHE_H__

#include <cache/base.h>
#include <cache/common/ebuild_exec.h>

class VarsReader;

class ParseCache : public BasicCache {

	private:
		bool nosubst;
		EbuildExec *ebuild_exec;

		void set_checking(std::string &str, const char *item, const VarsReader &ebuild, bool *ok = NULL);
		void readPackage(Category &vec, const std::string &pkg_name, const std::string &directory_path, const std::vector<std::string> &files) throw(ExBasic);
	public:
		ParseCache(bool no_substitute = false, bool mixed = false, bool use_sh = false) :
			BasicCache(),
			nosubst(no_substitute),
			ebuild_exec(NULL)
		{ if(mixed) ebuild_exec = new EbuildExec(use_sh, this); }

		~ParseCache()
		{
			if(ebuild_exec) {
				ebuild_exec->delete_cachefile();
				delete ebuild_exec;
				ebuild_exec = NULL;
			}
		}

		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return true; }

		const char *getType() const
		{
			if(ebuild_exec) {
				if(ebuild_exec->use_sh()) {
					if(nosubst)
						return "parse*|ebuild*";
					return "parse|ebuild*";
				}
				if(nosubst)
					return "parse*|ebuild";
				return "parse|ebuild";
			}
			if(nosubst)
				return "parse*";
			return "parse";
		}
};

#endif /* __PARSECACHE_H__ */
