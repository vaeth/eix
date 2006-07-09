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
	if(recursive && pushback_files(dir, files, files_exclude, false))
	{
		bool rvalue=true;
		for(vector<string>::iterator it=files.begin();
			it<files.end(); ++it)
		{
			++depth;
			ASSERT(depth < 100,
				"Nesting level too deep in %s", dir.c_str());
			if(! pushback_lines(it->c_str(), v, removed_empty))
				rvalue=false;
			--depth;
		}
		return rvalue;
	}
	else
		return pushback_lines_file(file, v, removed_empty);
}

/** List of files in directory.
 * Pushed names of file in directory into string-vector if the don't match any
 * char * in given exlude list.
 * @param dir_path Path to directory
 * @param into pointer to vector of strings .. files get append here (with full path)
 * @param exclude list of char * that don't need to be put into vector
 * @param onlyfiles consider only ordinary files
 * @return false if everything is ok */
bool pushback_files(string &dir_path, vector<string> &into, const char *exclude[], bool onlyfiles)
{
	struct stat static_stat;
	DIR *dir = opendir(dir_path.c_str());
	if( !(dir) ) return false;
	/* one for \0 and on for /, to be on the save side */

	struct dirent *dir_entry;
	while((dir_entry = readdir(dir)))
	{
		if(exclude)
		{
			char **_p = (char **)exclude;
			for(;*_p;_p++) /* Look if it's in exclude */
				if(strcmp(*_p, dir_entry->d_name) == 0)
					break;
			if(*_p)
				continue;

		}
		if(onlyfiles)
		{
			if(stat((dir_path + dir_entry->d_name).c_str(), &static_stat)
					|| !S_ISREG(static_stat.st_mode))
				continue;
		}
		into.push_back(dir_path + dir_entry->d_name);
	}
	closedir(dir);
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
