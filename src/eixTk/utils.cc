// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixTk/utils.h"
#include "eixrc/global.h"

using std::map;
using std::string;
using std::vector;

bool
scandir_cc(const string &dir, vector<string> *namelist, select_dirent select, bool sorted)
{
	namelist->clear();
	DIR *dirhandle(opendir(dir.c_str()));
	if(dirhandle == NULLPTR) {
		return false;
	}
	struct dirent res, *d;
	while(likely((readdir_r(dirhandle, &res, &d) == 0) && (d != NULLPTR))) {
		const char *name(d->d_name);
		// Omit "." and ".." since we must not rely on their existence anyway
		if(likely(strcmp(name, ".") && strcmp(name, "..") && (*select)(d))) {
			namelist->push_back(name);
		}
	}
	closedir(dirhandle);
	if(sorted) {
		sort(namelist->begin(), namelist->end());
	}
	return true;
}

/** push_back every line of file into v. */
static bool
pushback_lines_file(const char *file, vector<string> *v, bool remove_empty, bool remove_comments, string *errtext)
{
	string line;
	std::ifstream ifstr(file);
	if(!ifstr.is_open()) {
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("Can't open %s: %s")) % file % strerror(errno);
		}
		return false;
	}
	while(likely(getline(ifstr, line) != NULLPTR)) {
		if(remove_comments) {
			string::size_type x(line.find('#'));
			if(unlikely(x != string::npos))
				line.erase(x);
		}

		trim(&line);

		if((!remove_empty) || (!line.empty())) {
			v->push_back(line);
		}
	}
	if(ifstr.eof()) {  // if we have eof, everything went well
		return true;
	}
	if(errtext != NULLPTR) {
		*errtext = eix::format(_("Error reading %s: %s")) % file % strerror;
	}
	return false;
}

/** push_back every line of file or dir into v. */
bool
pushback_lines(const char *file, vector<string> *v, bool remove_empty, bool recursive, bool remove_comments, string *errtext)
{
	static const char *files_exclude[] = { "..", ".", "CVS", "RCS", "SCCS", NULLPTR };
	static int depth(0);
	vector<string> files;
	string dir(file);
	dir += "/";
	if(recursive && pushback_files(dir, files, files_exclude, 3)) {
		bool rvalue(true);
		for(vector<string>::iterator it(files.begin());
			likely(it != files.end()); ++it) {
			++depth;
			if (depth == 100) {
				if(errtext != NULLPTR) {
					*errtext = eix::format(_("Nesting level too deep in %r")) % dir;
				}
				return false;
			}
			if(unlikely(!pushback_lines(it->c_str(), v, remove_empty, true, remove_comments, errtext)))
				rvalue = false;
			--depth;
		}
		return rvalue;
	} else {
		return pushback_lines_file(file, v, remove_empty, remove_comments, errtext);
	}
}

/** These variables and function are only supposed to be used from
 *  pushback_files. We cannot use a class here, because scandir wants a
 *  "blank" selector-function */
static const char **pushback_files_exclude;
static bool pushback_files_no_hidden;
static size_t pushback_files_only_type;
static const string *pushback_files_dir_path;  // defined if pushback_files_only_type is nonzero
static int
pushback_files_selector(SCANDIR_ARG3 dir_entry)
{
	// Empty names shouldn't occur. Just to be sure, we ignore them:
	if(!((dir_entry->d_name)[0]))
		return 0;

	if(likely(pushback_files_no_hidden)) {
		// files starting with '.' are hidden.
		if((dir_entry->d_name)[0] == '.')
			return 0;
		// files ending with '~' are hidden.
		// (Note that we already excluded the bad case of empty names).
		if((dir_entry->d_name)[strlen(dir_entry->d_name) - 1] == '~')
			return 0;
	}
	if(pushback_files_exclude)
	{
		// Look if it's in exclude
		for(const char **p(pushback_files_exclude); likely(*p != NULLPTR); ++p)
			if(unlikely(strcmp(*p, dir_entry->d_name) == 0))
				return 0;
	}
	if(likely(pushback_files_only_type == 0))
		return 1;
	struct stat static_stat;
	if(unlikely(stat(((*pushback_files_dir_path) + dir_entry->d_name).c_str(), &static_stat)))
		return 0;
	if(pushback_files_only_type & 1) {
		if(S_ISREG(static_stat.st_mode))
			return 1;
	}
	if(pushback_files_only_type & 2) {
		if(S_ISDIR(static_stat.st_mode))
			return 1;
	}
	return 0;
}

/** List of files in directory.
 * Pushed names of file in directory into string-vector if the don't match any
 * char * in given exlude list.
 * @param dir_path Path to directory
 * @param into pointer to vector of strings .. files get append here (with full path)
 * @param exclude list of char * that don't need to be put into vector
 * @param only_type: if 1: consider only ordinary files, if 2: consider only dirs, if 3: consider only files or dirs
 * @param no_hidden ignore hidden files
 * @param full_path return full pathnames
 * @return true if everything is ok */
bool
pushback_files(const string &dir_path, vector<string> &into, const char *exclude[], unsigned char only_type, bool no_hidden, bool full_path)
{
	pushback_files_exclude = exclude;
	pushback_files_no_hidden = no_hidden;
	pushback_files_only_type = only_type;
	if(only_type)
		pushback_files_dir_path = &dir_path;
	vector<string> namelist;
	if(!scandir_cc(dir_path, &namelist, pushback_files_selector))
		return false;
	for(vector<string>::const_iterator it(namelist.begin());
		likely(it != namelist.end()); ++it) {
		if(full_path)
			into.push_back(dir_path + (*it));
		else
			into.push_back(*it);
	}
	return true;
}

/** Cycle through map using it, until it is it_end, append all values from it
 * to the value with the same key in append_to. */
void join_map(map<string, string> *append_to, map<string, string>::iterator it, map<string, string>::iterator it_end)
{
	while(likely(it != it_end)) {
		(*append_to)[it->first] =
			( ((*append_to)[it->first].size() != 0)
			  ? (*append_to)[it->first] + " " + it->second : it->second );
		++it;
	}
}

void dump_version()
{
	fputs(PACKAGE_STRING
#if defined(GCC_VERSION) || defined(TARGET)
		" ("
#ifdef GCC_VERSION
			"gcc-" GCC_VERSION
#ifdef TARGET
				", "
#endif
#endif
#ifdef TARGET
			TARGET
#endif
		")"
#endif
		"\n", stdout);
	exit(EXIT_SUCCESS);
}
