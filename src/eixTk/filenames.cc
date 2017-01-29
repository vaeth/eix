// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

// #define DEBUG_NORMALIZE_PATH

#include <fnmatch.h>

// For unknown reason, cstdlib must be included before sys/param.h and climits:
// Otherwise gcc-4.7.1 -flto -D_FORTIFY_SOURCE=2 linking warns about PATH_MAX
#include <cstdlib>

// Try to read PATH_MAX from climits/limits:
#ifndef HAVE_CLIMITS
#ifdef HAVE_LIMITS_H
#include <limits.h>  // NOLINT(build/include_order)
#endif
#else
#include <climits>
#endif

// On some systems PATH_MAX is only contained in sys/param.h:
#ifndef PATH_MAX
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>  // NOLINT(build/include_order)
#endif
#endif

#include <cstring>

#ifdef DEBUG_NORMALIZE_PATH
#include <iostream>
#endif
#include <string>

#include "eixTk/attribute.h"
#include "eixTk/filenames.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/sysutils.h"

// If we still don't have PATH_MAX, try to use MAXPATHLEN:

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#endif
#endif

using std::string;

#ifdef DEBUG_NORMALIZE_PATH
using std::cerr;

#define NORMALIZE_PATH_EXPORT static
#define NORMALIZE_PATH original_normalize_path

ATTRIBUTE_NONNULL_ static string NORMALIZE_PATH(const char *path, bool resolve, bool want_slash);

string normalize_path(const char *path, bool resolve, bool want_slash) {
	cerr << "Debug: Calling normalize_path(\"" << path << "\", " << resolve << ")...\nDebug: (with";
#ifndef HAVE_CANONICALIZE_FILE_NAME
	cerr << "out";
#endif
	cerr << " canonicalize_file_name(), with";
#ifndef HAVE_REALPATH
	cerr << "out";
#endif
	cerr << " realpath(), with";
#ifdef PATH_MAX
	cerr << " PATH_MAX=" << PATH_MAX;
#else
	cerr << "out PATH_MAX";
#endif
	cerr << ")\n";
	string s(NORMALIZE_PATH(path, resolve, want_slash));
	cerr << "Debug: ... returned with: \"" << s << "\"\n";
	return s;
}
#else
#define NORMALIZE_PATH_EXPORT
#define NORMALIZE_PATH normalize_path
#endif

NORMALIZE_PATH_EXPORT string NORMALIZE_PATH(const char *path, bool resolve, bool want_slash) {
	if(*path == 0) {
		return "";
	}
	string name;
	if(resolve) {
		char *normalized(NULLPTR);
#ifdef HAVE_CANONICALIZE_FILE_NAME
		normalized = canonicalize_file_name(path);
		if(likely(normalized != NULLPTR)) {
			if(unlikely(*normalized == '\0')) {
				free(normalized);
				normalized = NULLPTR;
			}
		}
#endif
#ifdef HAVE_REALPATH
		if(unlikely(normalized == NULLPTR)) {
#ifdef PATH_MAX
			// Some implementations of realpath are vulnerable
			// against internal buffer overflow, so better test:
			if(likely(strlen(path) < PATH_MAX)) {
				normalized = static_cast<char *>(malloc(PATH_MAX + 1));
				if(likely(normalized != NULLPTR)) {
					if(unlikely(realpath(path, normalized) == NULLPTR)) {
						free(normalized);
						normalized = NULLPTR;
					}
				}
			}
#else
			// We have no idea about the maximal pathlen
			normalized = realpath(path, NULLPTR);
#endif
			// Let normalized="" act as normalized=NULLPTR:
			if(likely(normalized != NULLPTR)) {
				if(unlikely(*normalized == '\0')) {
					free(normalized);
					normalized = NULLPTR;
				}
			}
		}
#endif
		if(likely(normalized != NULLPTR)) {
			name = normalized;
			free(normalized);
		} else {
			name = path;
		}
	} else {
		name = path;
	}
	for(string::size_type i(0); likely(i < name.size()); ++i) {
		// Erase all / following one /
		if(name[i] == '/') {
			string::size_type n(0);
			for(string::size_type j(i + 1);
				likely(j < name.size()); ++j) {
				if(name[j] != '/')
					break;
				++n;
			}
			if(n != 0)
				name.erase(i + 1, n);
		}
	}
	// Erase trailing / if it is not the first character and !want_slash
	// Possibly append / if want_slash

	string::size_type s(name.size());
	if(unlikely(s == 0))
		return name;
	if(name[--s] == '/') {
		if((s != 0) && (!want_slash))
			name.erase(s);
	} else if(want_slash) {
		return name + '/';
	}
	return name;
}

/**
Compare whether two (normalized) filenames are identical
**/
bool same_filenames(const char *mask, const char *name, bool glob, bool resolve_mask) {
	string m(normalize_path(mask, resolve_mask, false));
	string n(normalize_path(name, false, false));
	if(!glob)
		return (m == n);
	return (!fnmatch(m.c_str(), n.c_str(), 0));
}

/**
Compare whether (normalized) filename starts with mask
**/
bool filename_starts_with(const char *mask, const char *name, bool resolve_mask) {
	string m(normalize_path(mask, resolve_mask, true));
	string n(normalize_path(name, false, true));
	if(m == n) {
		return true;
	}
	return (n.compare(0, m.size(), m) == 0);
}

/**
@return first match in a list of filenames/patterns
**/
WordVec::const_iterator find_filenames(const WordVec::const_iterator start,
		const WordVec::const_iterator end, const char *search,
		bool list_of_patterns, bool resolve_list) {
	for(WordVec::const_iterator i(start); likely(i != end); ++i) {
		if(unlikely(same_filenames(i->c_str(), search, list_of_patterns, resolve_list))) {
			return i;
		}
	}
	return end;
}

/**
Test whether filename appears to be a "virtual" overlay
**/
bool is_virtual(const char *name) {
	if(*name != '/') {
		return true;
	}
	return !is_dir(name);
}
