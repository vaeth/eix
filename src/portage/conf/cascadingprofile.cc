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

#include "cascadingprofile.h"

#include <eixTk/utils.h>
#include <varsreader.h>

#include <fstream>

#include <unistd.h>

/* Path to symlink to profile */
#define PROFILE_LINK "/etc/make.profile"
#define PROFILE_LINK_DIRECTORY "/etc/"

/* Buffer size for the readlink-call */
#define READLINK_BUFFER 128

/** Exclude this files from listing of files in profile. */
static const char *profile_exclude[] = { "parent", "..", "." , NULL };

#if 0
#define MASKMAP_REMOVE(m) do { map<string, vector<Mask*> >::iterator it = (m).begin(); \
	for( ;it != (m).end(); ++it) { \
		for(unsigned int i = 0; i<it->second.size(); ++i) { \
			delete it->second[i]; \
		} \
	} \
} while(0);
#endif

/** Look for parent profile of the profile pointed to by path_buffer. Write
 * the path for the new profile into path_buffer and return true; Return false
 * if there is no parent profile. */
bool CascadingProfile::getParentProfile(string &path_buffer)
{
	string _buf;

	/* Open stream and check if it's open */
	ifstream ifstr((path_buffer + "parent").c_str());
	if(! ifstr.is_open())
		return false;

	/* while there are lines in the file */
	while(getline(ifstr, _buf))
	{
		_buf = trim(_buf, "\t\n\r ");
		/* If it's a comment or a empty line continue with the next line */
		if(_buf.size() == 0 || _buf[0] == '#')
			continue;

		path_buffer.append(_buf);
		path_buffer.append("/");
		return true;
	}
	return false;
}

void CascadingProfile::collectLines(const char* file, vector<string> *lines)
{
	/* Get all lines */
	for(unsigned int i = 0; i<_profile_files.size(); i++)
		if( strcmp(strrchr(_profile_files[i].c_str(), '/'), file) == 0)
			(void) pushback_lines(_profile_files[i].c_str(), lines);
}

void CascadingProfile::readPackages()
{
	vector<string> lines;
	collectLines("/packages", &lines);

	/* Cycle through and get rid of comments ..
	 * lines beginning with '*' are system-packages
	 * all others are masked by profile .. if they don't match :) */
	Mask *m = NULL;
	for(unsigned int i = 0; i<lines.size(); i++) {
		switch(lines[i][0]) {
			case '*': 
				try {
					m = new Mask(lines[i].substr(1), Mask::maskInSystem) ;
				}
				catch(ExBasic e) {
					cerr << e << endl;
					continue;
				}
				system.add(m);
				break;
			default:
				try {
					m = new Mask(lines[i], Mask::maskAllowedByProfile);
				}
				catch(ExBasic e) {
					cerr << e << endl;
					continue;
				}
				system_allowed.add(m);
				break;
		}
	}
}

/** Key that should accumelate their content rathern then replace. */
static const char *default_accumulating_keys[] = {
	"USE",
	"CONFIG_*",
	"FEATURES",
	"ACCEPT_KEYWORDS",
	NULL
};

/** Read all "make.defaults" files found in profile. */
void CascadingProfile::readMakeDefaults()
{
	for(unsigned int i = 0; i<_profile_files.size(); i++) {
		if( strcmp(strrchr(_profile_files[i].c_str(), '/'), "/make.defaults") == 0) {
			VarsReader parser(VarsReader::SUBST_VARS|VarsReader::INTO_MAP|VarsReader::APPEND_VALUES);
			parser.accumulatingKeys(default_accumulating_keys); // use defaults
			parser.useMap(_make_defaults);
			parser.read(_profile_files[i].c_str());
		}
	}
}

void CascadingProfile::readPackageMasks()
{
	vector<string> lines;
	collectLines("/package.mask", &lines);
	for(unsigned int i = 0; i<lines.size(); i++) {
		Mask *m = NULL;
		try {
			m = new Mask(lines[i], Mask::maskMask);
		}
		catch(ExBasic e) {
			cerr << e << endl;
			continue;
		}
		package_masks.add(m);
	}
}

/** Cycle through profile and put path to files into this->_profile_files. */
void CascadingProfile::listProfile(void) throw(ExBasic)
{
	char readlink_buffer[READLINK_BUFFER];
	int len = readlink(PROFILE_LINK, readlink_buffer, READLINK_BUFFER - 1);
	if(len == -1) {
		throw( ExBasic("readlink("PROFILE_LINK") failed: %s", strerror(errno)) );
	}
	readlink_buffer[len] = '\0';

	string path_buffer(readlink_buffer);
	/* If it's a relative path prepend the dirname of PROFILE_LINK_DIRECTORY */
	if( path_buffer[0] != '/' )
		path_buffer.insert(0, PROFILE_LINK_DIRECTORY);
	path_buffer.append("/"); /* append "/" */

	do {
		/* Don't care about errors when reading profile. */
		(void) pushback_files(path_buffer, _profile_files, profile_exclude);
	} while( getParentProfile(path_buffer) );

}
