// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <string>

#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/utils.h"
#include "eixrc/global.h"

using std::string;

class Directory {
	private:
		DIR *dh;

	public:
		Directory() : dh(NULLPTR) {
		}

		bool opendirectory(const char *name) {
			return ((dh = opendir(name)) != NULLPTR);
		}

		struct dirent *read() {
			return readdir(dh);  // NOLINT(runtime/threadsafe_fn)
		}

		~Directory() {
			if(dh != NULLPTR) {
				closedir(dh);
			}
		}
};

static bool pushback_lines_file(const char *file, WordVec *v, bool keep_empty, eix::SignedBool keep_comments, string *errtext) ATTRIBUTE_NONNULL((1, 2));
static int pushback_files_selector(SCANDIR_ARG3 dir_entry);

bool scandir_cc(const string& dir, WordVec *namelist, select_dirent select, bool sorted) {
	namelist->clear(); {
		Directory my_dir;
		if(!my_dir.opendirectory(dir.c_str())) {
			return false;
		}
		struct dirent *d;
		while(likely((d = my_dir.read()) != NULLPTR)) {
			const char *name(d->d_name);
			// Omit "." and ".." since we must not rely on their existence anyway
			if(likely(strcmp(name, ".") && strcmp(name, "..") && (*select)(d))) {
				namelist->push_back(name);
			}
		}
	}
	if(sorted) {
		sort(namelist->begin(), namelist->end());
	}
	return true;
}

/**
push_back every line of file into v.
**/
static bool pushback_lines_file(const char *file, LineVec *v, bool keep_empty, eix::SignedBool keep_comments, string *errtext) {
	string line;
	std::ifstream ifstr(file);
	if(unlikely(!ifstr.is_open())) {
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("cannot open %s: %s")) % file % strerror(errno);
		}
		return false;
	}
	while(likely(ifstr.good())) {
		getline(ifstr, line);
		if(unlikely(line.empty() && unlikely(!ifstr.good())))  {
			break;
		}
		if(keep_comments <= 0) {
			string::size_type x(line.find('#'));
			if(unlikely(x != string::npos)) {
				if(likely((keep_comments == 0) || (x != 0))) {
					line.erase(x);
				}
			}
		}

		trim(&line);

		if(keep_empty || (!line.empty())) {
			v->push_back(line);
		}
	}
	if(likely(ifstr.eof())) {  // if we have eof, everything went well
		return true;
	}
	if(errtext != NULLPTR) {
		*errtext = eix::format(_("error reading %s: %s")) % file % strerror;
	}
	return false;
}

/**
Files excluded for pushback_lines in recursive mode
**/
const char *pushback_lines_exclude[] = { "..", ".", "CVS", "RCS", "SCCS", NULLPTR };

/**
push_back every line of file or dir into v.
**/
bool pushback_lines(const char *file, LineVec *v, bool recursive, bool keep_empty, eix::SignedBool keep_comments, string *errtext) {
	static int depth(0);
	WordVec files;
	string dir(file);
	dir.append(1, '/');
	if(recursive && pushback_files(dir, &files, pushback_lines_exclude, 3)) {
		bool rvalue(true);
		for(WordVec::const_iterator it(files.begin());
			likely(it != files.end()); ++it) {
			++depth;
			if (depth == 100) {
				if(errtext != NULLPTR) {
					*errtext = eix::format(_("nesting level too deep in %r")) % dir;
				}
				return false;
			}
			if(unlikely(!pushback_lines(it->c_str(), v, true, keep_empty, keep_comments, errtext))) {
				rvalue = false;
			}
			--depth;
		}
		return rvalue;
	} else {
		return pushback_lines_file(file, v, keep_empty, keep_comments, errtext);
	}
}

/**
These variables and function are only supposed to be used from
pushback_files. We cannot use a class here, because scandir wants a
"blank" selector-function
**/
static const char **pushback_files_exclude;
static bool pushback_files_no_hidden;
static size_t pushback_files_only_type;
static const string *pushback_files_dir_path;  // defined if pushback_files_only_type is nonzero
static int pushback_files_selector(SCANDIR_ARG3 dir_entry) {
	// Empty names shouldn't occur. Just to be sure, we ignore them:
	if(!((dir_entry->d_name)[0])) {
		return 0;
	}

	if(likely(pushback_files_no_hidden)) {
		// files starting with '.' are hidden.
		if((dir_entry->d_name)[0] == '.') {
			return 0;
		}
		// files ending with '~' are hidden.
		// (Note that we already excluded the bad case of empty names).
		if((dir_entry->d_name)[strlen(dir_entry->d_name) - 1] == '~') {
			return 0;
		}
	}
	if(pushback_files_exclude) {
		// Look if it's in exclude
		for(const char **p(pushback_files_exclude); likely(*p != NULLPTR); ++p) {
			if(unlikely(strcmp(*p, dir_entry->d_name) == 0)) {
				return 0;
			}
		}
	}
	if(likely(pushback_files_only_type == 0)) {
		return 1;
	}
	struct stat static_stat;
	if(unlikely(stat(((*pushback_files_dir_path) + dir_entry->d_name).c_str(), &static_stat))) {
		return 0;
	}
	if(pushback_files_only_type & 1) {
		if(S_ISREG(static_stat.st_mode)) {
			return 1;
		}
	}
	if(pushback_files_only_type & 2) {
		if(S_ISDIR(static_stat.st_mode)) {
			return 1;
		}
	}
	return 0;
}

/**
List of files in directory.
Pushed names of file in directory into string-vector if the don't match any
char * in given exlude list.
@param dir_path Path to directory
@param into pointer to WordVec .. files get append here (with full path)
@param exclude list of char * that don't need to be put into vector
@param only_type: if 1: consider only ordinary files, if 2: consider only dirs, if 3: consider only files or dirs
@param no_hidden ignore hidden files
@param full_path return full pathnames
@return true if everything is ok
**/
bool pushback_files(const string& dir_path, WordVec *into, const char *exclude[], unsigned char only_type, bool no_hidden, bool full_path) {
	pushback_files_exclude = exclude;
	pushback_files_no_hidden = no_hidden;
	pushback_files_only_type = only_type;
	if(only_type) {
		pushback_files_dir_path = &dir_path;
	}
	WordVec namelist;
	if(!scandir_cc(dir_path, &namelist, pushback_files_selector)) {
		return false;
	}
	for(WordVec::const_iterator it(namelist.begin());
		likely(it != namelist.end()); ++it) {
		if(full_path) {
			into->push_back(dir_path + (*it));
		} else {
			into->push_back(*it);
		}
	}
	return true;
}

/**
Cycle through map using it, until it is it_end, append all values from it
to the value with the same key in append_to.
**/
void join_map(WordMap *append_to, WordMap::const_iterator it, WordMap::const_iterator it_end) {
	for(; likely(it != it_end); ++it) {
		string& to((*append_to)[it->first]);
		if(to.empty()) {
			to = it->second;
		} else {
			to.append(1, ' ');
			to.append(it->second);
		}
	}
}

void dump_version() {
	fputs(PACKAGE_VERSION "\n", stdout);
	exit(EXIT_SUCCESS);
}
