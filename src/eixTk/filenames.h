// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_FILENAMES_H_
#define SRC_EIXTK_FILENAMES_H_ 1

#include <string>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/sysutils.h"

/** canonicalize_file_name() if possible or some substitute */
std::string normalize_path(const char *path, bool resolve = true, bool want_slash = false);

/** Compare whether two (normalized) filenames are identical */
bool same_filenames(const char *mask, const char *name, bool glob = false, bool resolve_mask = true);

/** Compare whether (normalized) filename starts with mask */
bool filename_starts_with(const char *mask, const char *name, bool resolve_mask);

/** Return first match in a list of filenames/patterns. */
inline std::vector<std::string>::const_iterator
find_filenames(const std::vector<std::string>::const_iterator start,
		const std::vector<std::string>::const_iterator end, const char *search,
		bool list_of_patterns = false, bool resolve_list = false)
{
	for(std::vector<std::string>::const_iterator i(start); likely(i != end); ++i) {
		if(unlikely(same_filenames(i->c_str(), search, list_of_patterns, resolve_list))) {
			return i;
		}
	}
	return end;
}

/** Test whether filename appears to be a "virtual" overlay */
inline bool
is_virtual(const char *name)
{
	if(*name != '/')
		return true;
	return !is_dir(name);
}

#endif  // SRC_EIXTK_FILENAMES_H_
