// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_CACHE_METADATA_METADATA_H_
#define SRC_CACHE_METADATA_METADATA_H_ 1

#include <ctime>

#include <string>

#include "cache/base.h"
#include "cache/common/reader.h"
#include "eixTk/dialect.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/sysutils.h"

class Category;
class Depend;
class Package;
class Version;

class MetadataCache : public BasicCache {
	private:
		typedef enum {
			PATH_METADATA,
			PATH_METADATAMD5,
			PATH_METADATAMD5OR,
			PATH_FULL,
			PATH_REPOSITORY
		} PathType;
		PathType path_type;
		bool flat, have_override_path;
		std::string override_path;
		std::string m_type;
		std::string m_catpath;
		WordVec names;

		BasicReader *reader;

		void setType(PathType set_path_type, bool set_flat);
		void setFlat(bool set_flat);

	public:
		MetadataCache() : reader(NULLPTR) {
		}

		~MetadataCache() {
			delete reader;
		}

		bool initialize(const std::string& name);

		bool readCategoryPrepare(const char *cat_name) OVERRIDE ATTRIBUTE_NONNULL_;
		bool readCategory(Category *cat) OVERRIDE ATTRIBUTE_NONNULL_;
		void readCategoryFinalize() OVERRIDE;

		const char *get_md5sum(const std::string &pkg_name, const std::string &ver_name) const OVERRIDE ATTRIBUTE_NONNULL_;
		bool get_time(time_t *t, const std::string &pkg_name, const std::string &ver_name) const OVERRIDE ATTRIBUTE_NONNULL_;

		void get_version_info(const std::string &pkg_name, const std::string &ver_name, Version *version) const OVERRIDE ATTRIBUTE_NONNULL_;
		void get_common_info(const std::string &pkg_name, const std::string &ver_name, Package *pkg) const OVERRIDE ATTRIBUTE_NONNULL_;

		bool use_prefixport() const OVERRIDE ATTRIBUTE_PURE;

		const char *getType() const OVERRIDE {
			return m_type.c_str();
		}
};

#endif  // SRC_CACHE_METADATA_METADATA_H_
