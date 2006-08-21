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

#include <string>
#include <vector>

/** Compare whether two filenames are identical */
bool same_filenames(const std::string &a, const std::string &b);

inline
std::vector<std::string>::const_iterator find_filenames(const std::vector<std::string>::const_iterator start,
		const std::vector<std::string>::const_iterator end, const std::string &search)
{
	for(std::vector<std::string>::const_iterator i = start; i != end; ++i)
		if(same_filenames(*i, search))
			return i;
	return end;
}

#endif /* __FILENAMES_H__ */
