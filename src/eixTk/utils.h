// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __UTILS_H__
#define __UTILS_H__

#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>

/** scandir which even works on poor man's systems.
    We keep the original type for the callback function
    (including possible case distinctions whether its argument is const)
    for the case that we *have* to use scandir() for the implementation
    on some systems (which however is rather unlikely) */
struct dirent;
#define SCANDIR_ARG3 const struct dirent *
typedef int (*select_dirent)(SCANDIR_ARG3 dir_entry);
bool scandir_cc(const std::string &dir, std::vector<std::string> &namelist, select_dirent select, bool sorted = true);

/** push_back every line of file or dir into v. */
bool pushback_lines(const char *file, std::vector<std::string> *v, bool remove_empty = true, bool recursive = false, bool remove_comments = true);

/** List of files in directory.
 * Pushed names of file in directory into string-vector if the don't match any
 * char * in given exlude list.
 * @param dir_path Path to directory
 * @param into pointer to vector of strings .. files get append here (with full path)
 * @param exlude list of char * that don't need to be put into vector
 * @param only_type: if 1: consider only ordinary files, if 2: consider only dirs, if 3: consider only files or dirs
 * @param no_hidden ignore hidden files
 * @param full_path return full pathnames
 * @return true if everything is ok */
bool pushback_files(const std::string &dir_path, std::vector<std::string> &into, const char *exclude[] = NULL, short only_files = 1, bool no_hidden = true, bool full_path = true);


/** Cycle through map using it, until it is it_end, append all values from it
 * to the value with the same key in append_to. */
void join_map(std::map<std::string,std::string> *append_to, std::map<std::string,std::string>::iterator it, std::map<std::string,std::string>::iterator it_end);

typedef unsigned int PercentU; /// The type for %u

/** A percent status for stdout.
 * Only shows a status */
class PercentStatus {
	protected:
		std::string m_prefix;
		static const unsigned int hundred = 100;
		unsigned int m_max, m_run;
		bool on_start;
	public:
		void reprint(const char *special = NULL, bool back = true) const
		{
			if(back)
				fputs("\b\b\b\b", stdout);
			if(special)
				puts(special);
			else if(on_start)
				fputs("  0%", stdout);
			else if(m_run >= m_max)
				puts("100%");
			else
				printf("%3u%%", PercentU((hundred * m_run) / m_max));
			fflush(stdout);
		}

		void interprint_start()
		{ reprint("", false); }

		void interprint_end(const char *special = NULL)
		{ fputs(m_prefix.c_str(), stdout); reprint(special, false); }

		void init(unsigned int max)
		{ m_max = max; m_run = 0; on_start = true; }

		PercentStatus(const char *prefix) : m_prefix(prefix)
		{ init(0); }

		/** Start status.
		 * @param max number of steps you want to do. */
		void start(unsigned int max)
		{ init(max); interprint_end(); }

		/** Print next step. */
		void operator ++()
		{
			on_start = false;
			if(m_run < m_max)
				m_run++;
			reprint(NULL, true);
		}
};

/* Print version of eix to stdout. */
void dump_version(int exit_code = -1);



#endif /* __UTILS_H__ */
