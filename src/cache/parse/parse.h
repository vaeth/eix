// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_CACHE_PARSE_PARSE_H_
#define SRC_CACHE_PARSE_PARSE_H_ 1

#include <string>
#include <vector>

#include "cache/base.h"
#include "eixTk/null.h"
#include "portage/extendedversion.h"

class Category;
class EbuildExec;
class VarsReader;
class Version;

class ParseCache : public BasicCache {
	private:
		bool verbose;
		std::vector<BasicCache*> further;
		std::vector<bool> further_works;
		bool try_parse, nosubst;
		EbuildExec *ebuild_exec;
		std::vector<std::string> m_packages;
		std::string m_catpath;

		void set_checking(std::string &str, const char *item, const VarsReader &ebuild, bool *ok = NULLPTR);
		void parse_exec(const char *fullpath, const std::string &dirpath, bool read_onetime_info, bool &have_onetime_info, Package *pkg, Version *version);
		void readPackage(Category *cat, const std::string &pkg_name, const std::string &directory_path, const std::vector<std::string> &files);

	public:
		ParseCache() : BasicCache(), verbose(false), ebuild_exec(NULLPTR)
		{ }

		bool initialize(const std::string &name);

		~ParseCache();

		void setScheme(const char *prefix, const char *prefixport, const std::string &scheme);
		void setKey(ExtendedVersion::Overlay key);
		void setOverlayName(const std::string &name);
		void setErrorCallback(ErrorCallback error_callback);
		void setVerbose()
		{ verbose = true; }

		bool readCategoryPrepare(const char *cat_name);
		bool readCategory(Category *cat);
		void readCategoryFinalize();

		bool use_prefixport() const ATTRIBUTE_CONST_VIRTUAL
		{ return true; }

		const char *getType() const;
};

#endif  // SRC_CACHE_PARSE_PARSE_H_
