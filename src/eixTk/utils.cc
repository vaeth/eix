// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/utils.h"
#include <config.h>  // IWYU pragma: keep
#ifndef PACKAGE_VERSION
#include <config_vers.h>
#endif

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <string>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
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

ATTRIBUTE_NONNULL((1, 2)) static bool pushback_lines_file(const char *file, WordVec *v, bool keep_empty, eix::SignedBool keep_comments, string *errtext);
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
			if(likely(std::strcmp(name, ".") && std::strcmp(name, "..") && (*select)(d))) {
				namelist->PUSH_BACK(MOVE(name));
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
			*errtext = eix::format(_("cannot open %s: %s")) % file % std::strerror(errno);
		}
		return false;
	}
	while(likely(ifstr.good())) {
		getline(ifstr, line);
		if(unlikely(line.empty() && unlikely(!ifstr.good()))) {
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
			v->PUSH_BACK(MOVE(line));
		}
	}
	if(likely(ifstr.eof())) {  // if we have eof, everything went well
		return true;
	}
	if(errtext != NULLPTR) {
		*errtext = eix::format(_("error reading %s: %s")) % file % std::strerror(errno);
	}
	return false;
}

/**
push_back every line of file or dir into v.
**/
bool pushback_lines(const char *file, LineVec *v, bool recursive, bool keep_empty, eix::SignedBool keep_comments, string *errtext) {
	if(!recursive) {
		return pushback_lines_file(file, v, keep_empty, keep_comments, errtext);
	}
	WordVec files;
	bool rvalue(pushback_files_recurse(file, &files, true, errtext));
	for(WordVec::const_iterator it(files.begin()); likely(it != files.end()); ++it) {
		if(unlikely(!pushback_lines_file(it->c_str(), v, keep_empty, keep_comments, errtext))) {
			rvalue = false;
		}
	}
	return rvalue;
}

static bool is_only_type(const string &file, int only_type) {
	if (only_type == 0) {
		return true;
	}
	struct stat static_stat;
	if(likely(stat(file.c_str(), &static_stat) == 0)) {
		if (((only_type & 1) != 0)  && S_ISREG(static_stat.st_mode)) {
			return true;
		}
		if (((only_type & 2) != 0) && S_ISDIR(static_stat.st_mode)) {
			return true;
		}
	}
	return false;
}


/**
These variables and function are only supposed to be used from
pushback_files. We cannot use a class here, because scandir wants a
"blank" selector-function
**/
static const char *const *pushback_files_exclude;
static bool pushback_files_no_hidden;
static size_t pushback_files_only_type;
static string pushback_files_dir_path;  // defined if pushback_files_only_type is nonzero
static int pushback_files_selector(SCANDIR_ARG3 dir_entry) {
	// Empty names shouldn't occur. Just to be sure, we ignore them:
	if((dir_entry->d_name)[0] == '\0') {
		return 0;
	}

	if(likely(pushback_files_no_hidden)) {
		// files starting with '.' are hidden.
		if((dir_entry->d_name)[0] == '.') {
			return 0;
		}
		// files ending with '~' are hidden.
		// (Note that we already excluded the bad case of empty names).
		if((dir_entry->d_name)[std::strlen(dir_entry->d_name) - 1] == '~') {
			return 0;
		}
	}
	if(pushback_files_exclude != NULLPTR) {
		// Look if it's in exclude
		for(const char *const *p(pushback_files_exclude); likely(*p != NULLPTR); ++p) {
			if(unlikely(std::strcmp(*p, dir_entry->d_name) == 0)) {
				return 0;
			}
		}
	}
	return is_only_type(pushback_files_dir_path + dir_entry->d_name, pushback_files_only_type) ? 1 : 0;
}

bool pushback_files(const string& dir_path, WordVec *into, const char *const exclude[], unsigned char only_type, bool no_hidden, bool full_path) {
	pushback_files_exclude = exclude;
	pushback_files_no_hidden = no_hidden;
	pushback_files_only_type = only_type;
	if(only_type != 0) {
		pushback_files_dir_path.assign(dir_path);
		pushback_files_dir_path.append(1, '/');
	}
	WordVec namelist;
	if(!scandir_cc(dir_path, &namelist, pushback_files_selector)) {
		return false;
	}
	for(WordVec::iterator it(namelist.begin());
		likely(it != namelist.end()); ++it) {
		if(full_path) {
			into->PUSH_BACK(dir_path + (*it));
		} else {
			into->PUSH_BACK(MOVE(*it));
		}
	}
	return true;
}

/**
Files excluded for recursive pushback_files
**/
const char *pushback_files_recurse_exclude[] = { "..", ".", "CVS", "RCS", "SCCS", NULLPTR };

bool pushback_files_recurse(const string& dir_path, const string& subpath, int depth, WordVec *into, bool full_path, string *errtext) {
	WordVec files;
	if(pushback_files(dir_path, &files, pushback_files_exclude, 3, true, false)) {
		string dir(dir_path);
		dir.append(1, '/');
		string subdir(subpath);
		if (depth != 0) {
			subdir.append(1, '/');
		}
		for(WordVec::const_iterator it(files.begin());
			likely(it != files.end()); ++it) {
			if(depth == 100) {
				if(errtext != NULLPTR) {
					*errtext = eix::format(_("nesting level too deep in %s")) % dir_path;
				}
				return false;
			}
			if(unlikely(!pushback_files_recurse(dir + (*it), subdir + (*it), depth + 1, into, full_path, errtext))) {
				return false;
			}
		}
		return true;
	}
	if ((depth == 0) && !is_only_type(dir_path, 1)) {
		return true;
	}
	if (full_path) {
		into->PUSH_BACK(MOVE(dir_path));
	} else {
		into->PUSH_BACK(MOVE(subpath));
	}
	return true;
}

void dump_version() {
	eix::say() % PACKAGE_VERSION;
	std::exit(EXIT_SUCCESS);
}
