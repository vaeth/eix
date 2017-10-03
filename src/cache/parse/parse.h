// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_PARSE_PARSE_H_
#define SRC_CACHE_PARSE_PARSE_H_ 1

#include <config.h>

#include <string>
#include <vector>

#include "cache/base.h"
#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "portage/extendedversion.h"

class Category;
class EbuildExec;
class VarsReader;
class Version;

class ParseCache FINAL : public BasicCache {
	private:
		bool verbose;
		typedef std::vector<BasicCache*> FurtherCaches;
		FurtherCaches further;
		typedef std::vector<bool> FurtherWorks;
		FurtherWorks further_works;
		bool try_parse, nosubst, ebuild_sh;
		EbuildExec *ebuild_exec;
		WordVec m_packages;
		std::string m_catpath;

		ATTRIBUTE_NONNULL((2, 3)) void set_checking(std::string *str, const char *item, const VarsReader& ebuild, bool *ok);
		ATTRIBUTE_NONNULL_ void set_checking(std::string *str, const char *item, const VarsReader& ebuild) {
			set_checking(str, item, ebuild, NULLPTR);
		}

		ATTRIBUTE_NONNULL_ void parse_exec(const char *fullpath, const std::string& dirpath, bool read_onetime_info, bool *have_onetime_info, Package *pkg, Version *version);
		ATTRIBUTE_NONNULL_ void readPackage(Category *cat, const std::string& pkg_name, const std::string& directory_path, const WordVec& files);

	public:
		ParseCache() : BasicCache(), verbose(false), ebuild_exec(NULLPTR) {
		}

		bool initialize(const std::string& name);

		~ParseCache();

		void setScheme(const char *prefix, const char *prefixport, const std::string& scheme) OVERRIDE;
		void setKey(ExtendedVersion::Overlay key) OVERRIDE;
		void setOverlayName(const std::string& name) OVERRIDE;
		void setErrorCallback(ErrorCallback error_callback) OVERRIDE;

		void setVerbose() OVERRIDE {
			verbose = true;
		}

		ATTRIBUTE_NONNULL_ bool readCategoryPrepare(const char *cat_name) OVERRIDE;
		ATTRIBUTE_NONNULL_ bool readCategory(Category *cat) OVERRIDE;
		void readCategoryFinalize() OVERRIDE;

		ATTRIBUTE_CONST_VIRTUAL bool use_prefixport() const OVERRIDE {
			return true;
		}

		const char *getType() const OVERRIDE;
};

#endif  // SRC_CACHE_PARSE_PARSE_H_
