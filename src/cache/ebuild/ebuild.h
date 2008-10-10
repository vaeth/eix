// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __EBUILD_H__
#define __EBUILD_H__

#include <cache/base.h>
#include <cache/common/ebuild_exec.h>

class EbuildCache : public BasicCache {
	private:
		EbuildExec ebuild_exec;
	public:
		EbuildCache(bool use_sh = false) :
			BasicCache(),
			ebuild_exec(use_sh, this)
		{ }

		void readPackage(Category &vec, const std::string &pkg_name, const std::string &directory_path, const std::vector<std::string> &files) throw(ExBasic);
		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return true; }

		const char *getType() const
		{
			if(ebuild_exec.use_sh())
				return "ebuild*";
			return "ebuild";
		}
};

#endif /* __EBUILD_H__ */
