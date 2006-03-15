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

#ifndef __UTILS_H__
#define __UTILS_H__


#include <map>
#include <vector>
#include <string>

using namespace std;

/** push_back every line of file into v. */
bool pushback_lines(const char *file, vector<string> *v, bool removed_empty = true);

/** List of files in directory.
 * Pushed names of file in directory into string-vector if the don't match any
 * char * in given exlude list.
 * @param dir_path Path to directory
 * @param into pointer to vector of strings .. files get append here (with full path)
 * @param exlude list of char * that don't need to be put into vector
 * @return false if everything is ok */
bool pushback_files(string &dir_path, vector<string> &into, const char *exclude[]);


/** Cycle through map using it, until it is it_end, append all values from it
 * to the value with the same key in append_to. */
void join_map(map<string,string> *append_to, map<string,string>::iterator it, map<string,string>::iterator it_end);

/** A percent status for stdout.
 * Only shows a status */
class PercentStatus {
	public:
		/** Calculacte step-width.
		 * @param max number of steps you want to do. */
		PercentStatus(unsigned int max = 0) {
			if(max != 0) {
				_max = max;
				_run = 0;
				_step_size = 100.0/max;
				printf("  0%%");
			}
		}

		/** Start status. */
		void start(unsigned int max) {
			_max = max;
			_run = 0;
			_step_size = 100.0/max;
			printf("  0%%");
		}

		/** Print next step. */
		void operator ++() {
			++_run;
			if(_run == _max) {
				puts("\b\b\b\b" "100%");
				fflush(stdout);
				return;
			}

			printf("\b\b\b\b" "%.3i%%", (int) (_run * _step_size));
			fflush(stdout);
		}

	protected:
		unsigned int _max, /**<Number of steps. */
					 _run;
		float _step_size;  /**< Step-size. */
};

/* Print version of eix to stdout. */
void dump_version(int exit_code = -1);

#endif /* __UTILS_H__ */
