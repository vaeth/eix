// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "config.h"

#include "utils.h"

#include <global.h>

#include <eixTk/exceptions.h>
#include <eixTk/stringutils.h>

#include <algorithm>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>


using namespace std;

const unsigned int PercentStatus::hundred;

bool
scandir_cc(const string &dir, std::vector<std::string> &namelist, select_dirent select, bool sorted)
{
	namelist.clear();
	DIR *dirhandle = opendir(dir.c_str());
	if(!dirhandle)
		return false;
	const struct dirent *d;
	while((d = readdir(dirhandle)) != NULL) {
		const char *name = d->d_name;
		// Omit "." and ".." since we must not rely on their existence anyway
		if(strcmp(name, ".") && strcmp(name, "..") && (*select)(d))
			namelist.push_back(name);
	}
	closedir(dirhandle);
	if(sorted)
		sort(namelist.begin(), namelist.end());
	return true;
}

/** push_back every line of file into v. */
bool
pushback_lines_file(const char *file, vector<string> *v, bool remove_empty, bool remove_comments)
{
	string line;
	ifstream ifstr(file);
	if(!ifstr.is_open())
		return false;
	bool ok = true;
	while(!ifstr.eof()) {
		if(ifstr.bad()) {
			ok = false;
			break;
		}
		getline(ifstr, line);
		trim(&line);

		if(remove_comments) {
			string::size_type x = line.find_first_of('#');
			if(x != string::npos) {
				line.erase(x);
				trim(&line);
			}
		}

		if((!remove_empty) || (!line.empty()))
			v->push_back(line);
	}
	if(ok)
		ok = !ifstr.bad();
	ifstr.close();
	return ok;
}

/** push_back every line of file or dir into v. */
bool pushback_lines(const char *file, vector<string> *v, bool remove_empty, bool recursive, bool remove_comments)
{
	static const char *files_exclude[] = { "..", "." , NULL };
	static int depth=0;
	vector<string> files;
	string dir(file);
	dir += "/";
	if(recursive && pushback_files(dir, files, files_exclude, 3))
	{
		bool rvalue=true;
		for(vector<string>::iterator it=files.begin();
			it<files.end(); ++it)
		{
			++depth;
			if (depth == 100)
				throw ExBasic("Nesting level too deep in %r") % dir;
			if(! pushback_lines(it->c_str(), v, remove_empty, true, remove_comments))
				rvalue=false;
			--depth;
		}
		return rvalue;
	}
	else
		return pushback_lines_file(file, v, remove_empty, remove_comments);
}

/** These variables and function are only supposed to be used from
 *  pushback_files. We cannot use a class here, because scandir wants a
 *  "blank" selector-function */
static const char **pushback_files_exclude;
static bool pushback_files_no_hidden;
static short pushback_files_only_type;
static const string *pushback_files_dir_path; // defined if pushback_files_only_type is nonzero
int
pushback_files_selector(SCANDIR_ARG3 dir_entry)
{
	// Empty names shouldn't occur. Just to be sure, we ignore them:
	if(!((dir_entry->d_name)[0]))
		return 0;

	if(pushback_files_no_hidden)
	{
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
		for(const char **p = pushback_files_exclude ; *p ; ++p)
			if(strcmp(*p, dir_entry->d_name) == 0)
				return 0;
	}
	if(!pushback_files_only_type)
		return 1;
	struct stat static_stat;
	if(stat(((*pushback_files_dir_path) + dir_entry->d_name).c_str(), &static_stat))
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
bool pushback_files(const string &dir_path, vector<string> &into, const char *exclude[], short only_type, bool no_hidden, bool full_path)
{
	pushback_files_exclude = exclude;
	pushback_files_no_hidden = no_hidden;
	pushback_files_only_type = only_type;
	if(only_type)
		pushback_files_dir_path = &dir_path;
	vector<string> namelist;
	if(!scandir_cc(dir_path, namelist, pushback_files_selector))
		return false;
	for(vector<string>::const_iterator it = namelist.begin();
		it != namelist.end(); ++it) {
		if(full_path)
			into.push_back(dir_path + (*it));
		else
			into.push_back(*it);
	}
	return true;
}

/** Cycle through map using it, until it is it_end, append all values from it
 * to the value with the same key in append_to. */
void join_map(map<string,string> *append_to, map<string,string>::iterator it, map<string,string>::iterator it_end)
{
	while(it != it_end)
	{
		(*append_to)[it->first] =
			( ((*append_to)[it->first].size() != 0)
			  ? (*append_to)[it->first] + " " + it->second : it->second );
		++it;
	}
}

void dump_version(int exit_code)
{
	fputs(PACKAGE_STRING" ("
#if defined(GCC_VERSION)
			"gcc-"GCC_VERSION", "
#endif
#if defined(TARGET)
			TARGET
#endif
			")\n", stdout);
	if(exit_code != -1) {
		exit(exit_code);
	}
}
