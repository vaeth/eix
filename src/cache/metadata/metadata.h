// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#if !defined(EIX__METADATA_H__)
#define EIX__METADATA_H__

#include <cache/base.h>

class MetadataCache : public BasicCache {
	protected:
		typedef enum {
			PATH_METADATA,
			PATH_FULL,
			PATH_REPOSITORY
		} PathType;
		PathType path_type;
		bool flat, have_override_path;
		std::string override_path;
		std::string m_type;

		void setType(PathType set_path_type, bool set_flat);
	public:
		bool initialize(const std::string &name);

		bool readCategory(Category &vec) throw(ExBasic);

		bool use_prefixport() const
		{ return (path_type == PATH_METADATA); }

		const char *getType() const
		{ return m_type.c_str(); }
};

#endif /* EIX__METADATA_H__ */
