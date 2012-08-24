// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_CACHE_METADATA_METADATA_H_
#define SRC_CACHE_METADATA_METADATA_H_ 1

#include <ctime>

#include <string>
#include <vector>

#include "cache/base.h"
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
		bool flat, have_override_path, checkmd5;
		std::string override_path;
		std::string m_type;
		std::string m_catpath;
		std::vector<std::string> names;

		typedef void (*x_get_keywords_slot_iuse_restrict_t)(const std::string &filename, std::string &keywords, std::string &slotname, std::string &iuse, std::string &restr, std::string &props, Depend &dep, BasicCache::ErrorCallback error_callback);
		typedef void (*x_read_file_t)(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback);
		x_get_keywords_slot_iuse_restrict_t x_get_keywords_slot_iuse_restrict;
		x_read_file_t x_read_file;

		void setType(PathType set_path_type, bool set_flat);
		void setFlat(bool set_flat);

	public:
		bool initialize(const std::string &name);

		bool readCategoryPrepare(const char *cat_name);
		bool readCategory(Category *cat);
		void readCategoryFinalize();

		const char *get_md5sum(const char *pkg_name, const char *ver_name) const;
		time_t get_time(const char *pkg_name, const char *ver_name) const
		{ return get_mtime((m_catpath + "/" + pkg_name + "-" + ver_name).c_str()); }

		void get_version_info(const char *pkg_name, const char *ver_name, Version *version) const;
		void get_common_info(const char *pkg_name, const char *ver_name, Package *pkg) const
		{ (*x_read_file)((m_catpath + "/" + pkg_name + "-" + ver_name).c_str(), pkg, m_error_callback); }

		bool use_prefixport() const ATTRIBUTE_PURE;

		const char *getType() const
		{ return m_type.c_str(); }
};

#endif  // SRC_CACHE_METADATA_METADATA_H_
