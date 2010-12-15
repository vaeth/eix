// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "filenames.h"
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>

#include <string>

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fnmatch.h>

#ifdef DEBUG_NORMALIZE_PATH
#include <iostream>
#endif

// Try to read PATH_MAX from climits/limits:

#ifdef HAVE_CLIMITS
#include <climits>
#else
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#endif

// On some systems PATH_MAX is only contained in sys/param.h:

#ifndef PATH_MAX
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#endif

// If we still don't have PATH_MAX, try to use MAXPATHLEN:

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#endif
#endif

using namespace std;

#ifdef DEBUG_NORMALIZE_PATH
string original_normalize_path(const char *path, bool resolve);
string normalize_path(const char *path, bool resolve)
{
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
	string s(original_normalize_path(path,resolve));
	cerr << "Debug: ... returned with: \"" << s << "\"\n";
	return s;
}
#define normalize_path(a,b) original_normalize_path(a,b)
#endif

string normalize_path(const char *path, bool resolve, bool want_slash)
{
	if(!*path)
		return emptystring;
	string name;
	if(resolve)
	{
		char *normalized(NULL);
#ifdef HAVE_CANONICALIZE_FILE_NAME
		normalized = canonicalize_file_name(path);
		if(likely(normalized != NULL)) {
			if(unlikely(*normalized == '\0')) {
				free(normalized);
				normalized = NULL;
			}
		}
#endif
#ifdef HAVE_REALPATH
		if(unlikely(normalized == NULL)) {
#ifdef PATH_MAX
			// Some implementations of realpath are vulnerable
			// against internal buffer overflow, so better test:
			if(likely(strlen(path) < PATH_MAX)) {
				normalized = static_cast<char *>(malloc(PATH_MAX + 1));
				if(likely(normalized != NULL)) {
					if(unlikely(realpath(path, normalized) == 0)) {
						free(normalized);
						normalized = NULL;
					}
				}
			}
#else
			// We have no idea about the maximal pathlen
			normalized = realpath(path, NULL);
#endif
			// Let normalized="" act as normalized=NULL:
			if(likely(normalized != NULL)) {
				if(unlikely(*normalized == '\0')) {
					free(normalized);
					normalized = NULL;
				}
			}
		}
#endif
		if(likely(normalized != NULL)) {
			name = normalized;
			free(normalized);
		}
		else
			name = path;
	}
	else
		name = path;
	for(string::size_type i(0); likely(i < name.size()); ++i)
	{
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
	}
	else if(want_slash)
		return name + '/';
	return name;
}

/** Compare whether two (normalized) filenames are identical */
bool same_filenames(const char *mask, const char *name, bool glob, bool resolve_mask)
{
	string m(normalize_path(mask, resolve_mask, false));
	string n(normalize_path(name, false, false));
	if(!glob)
		return (m == n);
	return (!fnmatch(m.c_str(), n.c_str(), 0));
}
