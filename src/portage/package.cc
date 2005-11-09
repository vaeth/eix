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

#include "package.h"

#include <database/basicio.h>
#include <database/database.h>

#include <fstream>
#include <dirent.h>
#include <unistd.h>

/** Constructor */
Package::Package(FILE *is)
{
	stream = is;
	defaults();
}

/** Deconstructor */
Package::~Package()
{
	for(size_type i = 0; i < size(); ++i) {
		delete (*this)[i];
	}
}

/** Read needed values from db.  */
void Package::readNeeded(InputStatus we_want)
{
	if(have >= we_want) /* Already got this one. */
		return;

	switch(have) {
		case Package::NONE:
			name = io::read_string(stream);
			if(we_want == Package::NAME)
				break;
		case Package::NAME:
			desc = io::read_string(stream);
			if(we_want == Package::DESCRIPTION)
				break;
		case Package::DESCRIPTION:
			provide = io::read_string(stream);
			if(we_want == Package::PROVIDE)
				break;
		case Package::PROVIDE:
			homepage = io::read_string( stream);
			if(we_want == Package::HOMEPAGE)
				break;
		case Package::HOMEPAGE:
			licenses = io::read_string( stream);
			if(we_want == Package::LICENSE)
				break;
		case Package::LICENSE:
			size_type n;
			n = io::read<size_type>(stream);
			for(size_type i = 0; i<n; i++ ) {
				addVersion(new Version( stream ));
			}
		case Package::ALL:
			break;
	}
	have = we_want;
}

/** Comparator for version-pointers. */
inline
bool
versionComparator(BasicVersion *v1, BasicVersion *v2)
{
	return (*v1 < *v2);
}

/** Write the package to an eix db */
void Package::write( FILE *output ) throw(ExBasic)
{
	/* I didn't like the size-calculation, so now we are:
	 * - writing the package
	 * - seeking back to the position where the offset should be written 
	 * - write the offset (we already know where the package ends!) 
	 * - seek forward to the end of the package. 
	 *
	 * I think this needs to prove itself .. so I'll keep an eye on it.
	 *   -- Emil Beinroth */

	sort(begin(), end(), versionComparator);
	off_t offset_position = ftello(output);
	fseek(output, sizeof(offset_type), SEEK_CUR);

	io::write_string(output, name);
	io::write_string(output, desc);
	io::write_string(output, provide);
	io::write_string(output, homepage);
	io::write_string(output, licenses);

	// write all version entries
	size_type n = size();
	io::write<size_type>(output, n);

	vector<Version*>::iterator vi = begin();
	while(vi != end())
		(*vi++)->write(output);

	off_t pkg_end = ftello(output);
	fseek(output, offset_position, SEEK_SET);
	off_t v = (pkg_end - offset_position);
	io::write<offset_type>(output, v);
	fseek(output, pkg_end, SEEK_SET);
}

/** Check if a package has duplicated versions. */
bool Package::checkDuplicates(Version *version)
{
	if(version == NULL) {
		for(unsigned int i = 0; i<size(); ++i)
			for(unsigned int j = 0; j<size(); ++j)
				if(i != j && *((*this)[i]) == *((*this)[j])) {
					return true;
				}
	}
	else {
		for(unsigned int i = 0; i<size(); ++i)
			if( *((*this)[i]) == *version ) {
				return true;
			}
	}
	return false;
}

/** Adds a version to "the versions" vector. */
void Package::addVersion(Version *version)
{
	/* if the same version is in various places it should be shown.
	   possible thanks to the new [overlay] marker. */
	if(!have_duplicate_versions) {
		have_duplicate_versions = checkDuplicates(version);
	}

	/* This should remain with two if .. so we can guarante that
	 * versions.size() == 0 in the else. */
	if(size() != 0) {
		if(overlay_key != version->overlay_key)
			have_same_overlay_key = false;
		
		if(is_system_package) {
			is_system_package = version->isSystem();
		}
	}
	else {
		overlay_key       = version->overlay_key;
		is_system_package = version->isSystem();
	}

	push_back(version);
}
