/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "config.h"

#include "utils.h"

#include <global.h>

#include <eixTk/exceptions.h>
#include <eixTk/stringutils.h>

#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

/** push_back every line of file into v. */
bool pushback_lines_file(const char *file, vector<string> *v, bool removed_empty)
{
	string line;
	ifstream ifstr(file);
	if( ifstr.is_open() ) {
		do {
			getline(ifstr, line);
			trim(&line);

			// strip #blaaa
			string::size_type x = line.find_first_of("#");
			if(x != string::npos)
			{
				line.erase(x);
				trim(&line);
			}

			if(line.size() == 0 && removed_empty)
				continue;
			v->push_back(line);
			if( ifstr.bad() )
				return false;
		} while( !ifstr.eof() );
		ifstr.close();
		return true;
	}
	return false;
}

/** push_back every line of file or dir into v. */
bool pushback_lines(const char *file, vector<string> *v, bool removed_empty, bool recursive)
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
			ASSERT(depth < 100,
				"Nesting level too deep in %s", dir.c_str());
			if(! pushback_lines(it->c_str(), v, removed_empty, true))
				rvalue=false;
			--depth;
		}
		return rvalue;
	}
	else
		return pushback_lines_file(file, v, removed_empty);
}

/** These variables and function are only supposed to be used from
 *  pushback_files. We cannot use a class here, because scandir wants a
 *  "blank" selector-function */
static const char **pushback_files_exclude;
static bool pushback_files_no_hidden;
static short pushback_files_only_type;
static const string *pushback_files_dir_path; // defined if pushback_files_only_type is nonzero
int pushback_files_selector(SCANDIR_ARG3 dir_entry)
{
	if(pushback_files_no_hidden)
	{
		if((dir_entry->d_name)[0] == '.')
			return 0;
	}
	if(pushback_files_exclude)
	{
		const char **_p = pushback_files_exclude;
		for( ; *_p ; ++_p) /* Look if it's in exclude */
			if(strcmp(*_p, dir_entry->d_name) == 0)
				return 0;
	}
	if(!pushback_files_only_type)
		return 1;
	struct stat static_stat;
	if(stat(((*pushback_files_dir_path) + dir_entry->d_name).c_str(), &static_stat))
		return 0;
	if(pushback_files_only_type & 1)
		if(S_ISREG(static_stat.st_mode))
			return 1;
	if(pushback_files_only_type & 2)
		if(S_ISDIR(static_stat.st_mode))
			return 1;
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
	struct dirent **namelist = NULL;
	int num = scandir(dir_path.c_str(), &namelist,
		pushback_files_selector, alphasort);
	bool ok;
	if(num < 0)
		ok = false;
	else
	{
		ok = true;
		for(int i = 0; i < num; ++i)
		{
			if(full_path)
				into.push_back(dir_path + (namelist[i]->d_name));
			else
				into.push_back(namelist[i]->d_name);
		}
	}
	if(namelist)
		free(namelist);
	return ok;
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
