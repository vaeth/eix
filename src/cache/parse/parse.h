// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__PARSECACHE_H__
#define EIX__PARSECACHE_H__ 1

#include <cache/base.h>
#include <cache/common/ebuild_exec.h>

class VarsReader;

class ParseCache : public BasicCache {

	private:
		std::vector<BasicCache*> further;
		std::vector<bool> further_works;
		bool try_parse, nosubst;
		EbuildExec *ebuild_exec;
		std::vector<std::string> packages;
		std::string catpath;

		void set_checking(std::string &str, const char *item, const VarsReader &ebuild, bool *ok = NULL);
		void parse_exec(const char *fullpath, const std::string &dirpath, bool read_onetime_info, bool &have_onetime_info, Package *pkg, Version *version);
		void readPackage(Category &vec, const std::string &pkg_name, const std::string &directory_path, const std::vector<std::string> &files) throw(ExBasic);
	public:
		ParseCache() : BasicCache(),
			ebuild_exec(NULL)
		{ }

		bool initialize(const std::string &name);

		~ParseCache();

		void setScheme(const char *prefix, const char *prefixport, std::string scheme);
		void setKey(Version::Overlay key);
		void setOverlayName(std::string name);
		void setErrorCallback(ErrorCallback error_callback);

		bool readCategory(Category &vec) throw(ExBasic);
		bool readCategoryPrepare(Category &vec) throw(ExBasic);
		void readCategoryFinalize();

		bool use_prefixport() const
		{ return true; }

		const char *getType() const;
};

#endif /* EIX__PARSECACHE_H__ */
