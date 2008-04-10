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
	cerr << " canonicalize_filename(), with";
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

string normalize_path(const char *path, bool resolve)
{
	if(!*path)
		return "";
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
			string name(normalized);
			free(normalized);
			return name;
		}
	}
	string name(path);
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
	// Erase trailing / if it is not the first character
	string::size_type s = name.size();
	if(s > 1) {
		if(name[--s] == '/')
			name.erase(s, 1);
	}
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
