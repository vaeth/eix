/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
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

#ifndef __FILENAMES_H__
#define __FILENAMES_H__

#include "../../config.h"
#include <eixTk/sysutils.h>

#include <string>
#include <vector>

/** canonicalize_file_name() if possible or some substitute */
std::string normalize_path(const char *path, bool resolve = true);

/** Compare whether two (normalized) filenames are identical */
bool same_filenames(const char *mask, const char *name, bool glob = false, bool resolve_mask = true);

/** Return first match in a list of filenames/patterns. */
inline
std::vector<std::string>::const_iterator find_filenames(const std::vector<std::string>::const_iterator start,
		const std::vector<std::string>::const_iterator end, const char *search,
		bool list_of_patterns = false, bool resolve_list = false)
{
	for(std::vector<std::string>::const_iterator i = start; i != end; ++i)
		if(same_filenames(i->c_str(), search, list_of_patterns, resolve_list))
			return i;
	return end;
}

/** Test whether filename appears to be a "virtual" overlay */
inline
bool is_virtual(const char *name)
{
	if(*name != '/')
		return true;
	return !is_dir(name);
}

#endif /* __FILENAMES_H__ */
