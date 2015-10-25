// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_FILENAMES_H_
#define SRC_EIXTK_FILENAMES_H_ 1

#include <string>

#include "eixTk/stringtypes.h"

/**
canonicalize_file_name() if possible or some substitute
**/
std::string normalize_path(const char *path, bool resolve, bool want_slash) ATTRIBUTE_NONNULL_;
inline static std::string normalize_path(const char *path, bool resolve) ATTRIBUTE_NONNULL_;
inline static std::string normalize_path(const char *path, bool resolve) {
	return normalize_path(path, resolve, false);
}
inline static std::string normalize_path(const char *path) ATTRIBUTE_NONNULL_;
inline static std::string normalize_path(const char *path) {
	return normalize_path(path, true);
}

/**
Compare whether two (normalized) filenames are identical
**/
bool same_filenames(const char *mask, const char *name, bool glob, bool resolve_mask) ATTRIBUTE_NONNULL_;
inline static bool same_filenames(const char *mask, const char *name, bool glob) ATTRIBUTE_NONNULL_;
inline static bool same_filenames(const char *mask, const char *name, bool glob) {
	return same_filenames(mask, name, glob, true);
}
inline static bool same_filenames(const char *mask, const char *name) ATTRIBUTE_NONNULL_;
inline static bool same_filenames(const char *mask, const char *name) {
	return same_filenames(mask, name, false);
}

/**
Compare whether (normalized) filename starts with mask
**/
bool filename_starts_with(const char *mask, const char *name, bool resolve_mask) ATTRIBUTE_NONNULL_;

/**
@return first match in a list of filenames/patterns
**/
WordVec::const_iterator find_filenames(const WordVec::const_iterator start,
	const WordVec::const_iterator end, const char *search,
	bool list_of_patterns, bool resolve_list) ATTRIBUTE_NONNULL_;
inline static WordVec::const_iterator find_filenames(const WordVec::const_iterator start,
	const WordVec::const_iterator end, const char *search,
	bool list_of_patterns) ATTRIBUTE_NONNULL_;
inline static WordVec::const_iterator find_filenames(const WordVec::const_iterator start,
	const WordVec::const_iterator end, const char *search,
	bool list_of_patterns) {
	return find_filenames(start, end, search, list_of_patterns, false);
}
inline static WordVec::const_iterator find_filenames(const WordVec::const_iterator start,
	const WordVec::const_iterator end, const char *search) ATTRIBUTE_NONNULL_;
inline static WordVec::const_iterator find_filenames(const WordVec::const_iterator start,
	const WordVec::const_iterator end, const char *search) {
	return find_filenames(start, end, search, false);
}

/**
Test whether filename appears to be a "virtual" overlay
**/
bool is_virtual(const char *name) ATTRIBUTE_NONNULL_;

#endif  // SRC_EIXTK_FILENAMES_H_
