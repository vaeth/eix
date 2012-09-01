// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_UTILS_H_
#define SRC_EIXTK_UTILS_H_ 1

#include <map>
#include <string>
#include <vector>

#include "eixTk/null.h"

/** scandir which even works on poor man's systems.
    We keep the original type for the callback function
    (including possible case distinctions whether its argument is const)
    for the case that we *have* to use scandir() for the implementation
    on some systems (which however is rather unlikely) */
struct dirent;
#define SCANDIR_ARG3 const struct dirent *
typedef int (*select_dirent)(SCANDIR_ARG3 dir_entry);
bool scandir_cc(const std::string &dir, std::vector<std::string> *namelist, select_dirent select, bool sorted = true) ATTRIBUTE_NONNULL_;

/** push_back every line of file or dir into v. */
bool pushback_lines(const char *file, std::vector<std::string> *v, bool remove_empty = true, bool recursive = false, bool remove_comments = true, std::string *errtext = NULLPTR) ATTRIBUTE_NONNULL((1, 2));

/** List of files in directory.
 * Pushed names of file in directory into string-vector if they don't match any
 * char * in given exlude list.
 * @param dir_path Path to directory
 * @param into pointer to vector of strings .. files get append here (with full path)
 * @param exlude list of char * that don't need to be put into vector
 * @param only_type: if 1: consider only ordinary files, if 2: consider only dirs, if 3: consider only files or dirs
 * @param no_hidden ignore hidden files
 * @param full_path return full pathnames
 * @return true if everything is ok. Nonexisting directory is not ok. */
bool pushback_files(const std::string &dir_path, std::vector<std::string> *into, const char *exclude[] = NULLPTR, unsigned char only_files = 1, bool no_hidden = true, bool full_path = true) ATTRIBUTE_NONNULL((2));

/** Cycle through map using it, until it is it_end, append all values from it
 * to the value with the same key in append_to. */
void join_map(std::map<std::string, std::string> *append_to, std::map<std::string, std::string>::iterator it, std::map<std::string, std::string>::iterator it_end) ATTRIBUTE_NONNULL_;

/* Print version of eix to stdout. */
void dump_version() ATTRIBUTE_NORETURN;

#endif  // SRC_EIXTK_UTILS_H_
