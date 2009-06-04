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

		bool readCategory(Category &vec) throw(ExBasic);

		bool readCategoryPrepare(Category &vec) throw(ExBasic);
		void readCategoryFinalize();

		bool use_prefixport() const
		{ return (path_type == PATH_METADATA); }

		const char *getType() const
		{ return m_type.c_str(); }
};

#endif /* EIX__METADATA_H__ */
