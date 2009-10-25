// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__METADATA_H__
#define EIX__METADATA_H__ 1

#include <cache/base.h>
#include <eixTk/exceptions.h>
#include <eixTk/sysutils.h>

#include <string>
#include <vector>

#include <ctime>

class Category;
class Package;
class Version;

class MetadataCache : public BasicCache {

	private:
		typedef enum {
			PATH_METADATA,
			PATH_FULL,
			PATH_REPOSITORY
		} PathType;
		PathType path_type;
		bool flat, have_override_path;
		std::string override_path;
		std::string m_type;
		std::string catpath;
		std::vector<std::string> names;

		typedef void (*x_get_keywords_slot_iuse_restrict_t)(const std::string &filename, std::string &keywords, std::string &slotname, std::string &iuse, std::string &restr, std::string &props, BasicCache::ErrorCallback error_callback);
		typedef void (*x_read_file_t)(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback);
		x_get_keywords_slot_iuse_restrict_t x_get_keywords_slot_iuse_restrict;
		x_read_file_t x_read_file;

		void setType(PathType set_path_type, bool set_flat);
	public:
		bool initialize(const std::string &name);

		bool readCategory(const char *cat_name, Category &cat) throw(ExBasic);

		bool readCategoryPrepare(const char *cat_name) throw(ExBasic);
		void readCategoryFinalize();

		time_t get_time(const char *pkg_name, const char *ver_name) const
		{ return get_mtime((catpath + "/" + pkg_name + "-" + ver_name).c_str()); }
		void get_version_info(const char *pkg_name, const char *ver_name, Version *version) const;
		void get_common_info(const char *pkg_name, const char *ver_name, Package *pkg) const
		{ (*x_read_file)((catpath + "/" + pkg_name + "-" + ver_name).c_str(), pkg, m_error_callback); }

		bool use_prefixport() const
		{ return (path_type == PATH_METADATA); }

		const char *getType() const
		{ return m_type.c_str(); }
};

#endif /* EIX__METADATA_H__ */
