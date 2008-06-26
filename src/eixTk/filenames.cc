// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "filenames.h"
#include <fnmatch.h>
#include <cstdlib>
#include <cstring>

// Try to read PATH_MAX from climits/limits:

#if defined(HAVE_CLIMITS)
#include <climits>
#else
#if defined(HAVE_LIMITS_H)
#include <limits.h>
#endif
#endif

// On some systems PATH_MAX is only contained in sys/param.h:

#if !defined(PATH_MAX)
#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#endif

// If we still don't have PATH_MAX, try to use MAXPATHLEN:

#if !defined(PATH_MAX)
#if defined(MAXPATHLEN)
#define PATH_MAX MAXPATHLEN
#endif
#endif

using namespace std;

#if defined(DEBUG_NORMALIZE_PATH)
#include <iostream>
string original_normalize_path(const char *path, bool resolve);
string normalize_path(const char *path, bool resolve)
{
	cerr << "Debug: Calling normalize_path(\"" << path << "\", " << resolve << ")...\nDebug: (with";
#if !defined(HAVE_CANONICALIZE_FILE_NAME)
	cerr << "out";
#endif
	cerr << " canonicalize_file_name(), with";
#if !defined(HAVE_REALPATH)
	cerr << "out";
#endif
	cerr << " realpath(), with";
#if defined(PATH_MAX)
	cerr << " PATH_MAX=" << PATH_MAX;
#else
	cerr << "out PATH_MAX";
#endif
	cerr << ")\n";
	string s = original_normalize_path(path,resolve);
	cerr << "Debug: ... returned with: \"" << s << "\"\n";
	return s;
}
#define normalize_path(a,b) original_normalize_path(a,b)
#endif

#if !defined(HAVE_CANONICALIZE_FILE_NAME)
#if !defined(HAVE_REALPATH)
#error "Neither canonicalize_file_name() nor realpath() are available."
#endif
#endif

string normalize_path(const char *path, bool resolve, bool want_slash)
{
	if(!*path)
		return "";
	string name;
	if(resolve)
	{
		char *normalized = NULL;
#if defined(HAVE_CANONICALIZE_FILE_NAME)
		normalized = canonicalize_file_name(path);
		if(normalized) {
			if(!*normalized) {
				free(normalized);
				normalized = NULL;
			}
		}
#endif
#if defined(HAVE_REALPATH)
		if(!normalized) {
#if defined(PATH_MAX)
			// Some implementations of realpath are vulnerable
			// against internal buffer overflow, so better test:
			if(strlen(path) < PATH_MAX) {
				normalized = static_cast<char *>(malloc(PATH_MAX + 1));
				if(normalized) {
					if(!realpath(path, normalized))
					{
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
			if(normalized) {
				if(!*normalized) {
					free(normalized);
					normalized = NULL;
				}
			}
		}
#endif
		if(normalized) {
			name = normalized;
			free(normalized);
		}
		else
			name = path;
	}
	else
		name = path;
	for(string::size_type i = 0; i < name.size(); ++i)
	{
		// Erase all / following one /
		if(name[i] == '/') {
			string::size_type n = 0;
			for(string::size_type j = i + 1 ;
				j < name.size(); ++j) {
				if(name[j] != '/')
					break;
				++n;
			}
			if(n)
				name.erase(i + 1, n);
		}
	}
	// Erase trailing / if it is not the first character and !want_slash
	// Possibly append / if want_slash

	string::size_type s = name.size();
	if(!s)
		return name;
	if(name[--s] == '/') {
		if((!s) && (!want_slash))
			name.erase(s, 1);
	}
	else if(want_slash)
		return name + '/';
	return name;
}

/** Compare whether two (normalized) filenames are identical */
bool same_filenames(const char *mask, const char *name, bool glob, bool resolve_mask)
{
	string m = normalize_path(mask, resolve_mask);
	string n = normalize_path(name, false);
	if(!glob)
		return (m == n);
	return (fnmatch(m.c_str(), n.c_str(), 0) == 0);
}
