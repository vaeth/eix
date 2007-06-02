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

#include "keywords.h"

const Keywords::Redundant
	Keywords::RED_NOTHING       = 0x00000,
	Keywords::RED_DOUBLE        = 0x00001,
	Keywords::RED_DOUBLE_LINE   = 0x00002,
	Keywords::RED_MIXED         = 0x00004,
	Keywords::RED_WEAKER        = 0x00008,
	Keywords::RED_STRANGE       = 0x00010,
	Keywords::RED_NO_CHANGE     = 0x00020,
	Keywords::RED_MINUSASTERISK = 0x00040,
	Keywords::RED_IN_KEYWORDS   = 0x00080,
	Keywords::RED_ALL_KEYWORDS  = RED_DOUBLE|RED_DOUBLE_LINE|RED_MIXED|RED_WEAKER|RED_STRANGE|RED_NO_CHANGE|RED_MINUSASTERISK|RED_IN_KEYWORDS,
	Keywords::RED_MASK          = 0x00100,
	Keywords::RED_DOUBLE_MASK   = 0x00200,
	Keywords::RED_IN_MASK       = 0x00400,
	Keywords::RED_UNMASK        = 0x00800,
	Keywords::RED_DOUBLE_UNMASK = 0x01000,
	Keywords::RED_IN_UNMASK     = 0x02000,
	Keywords::RED_ALL_MASK      = RED_MASK|RED_DOUBLE_MASK|RED_IN_MASK,
	Keywords::RED_ALL_UNMASK    = RED_UNMASK|RED_DOUBLE_UNMASK|RED_IN_UNMASK,
	Keywords::RED_ALL_MASKSTUFF = RED_ALL_MASK|RED_ALL_UNMASK,
	Keywords::RED_DOUBLE_USE    = 0x04000,
	Keywords::RED_IN_USE        = 0x08000,
	Keywords::RED_ALL_USE       = RED_DOUBLE_USE|RED_IN_USE,
	Keywords::RED_DOUBLE_CFLAGS = 0x10000,
	Keywords::RED_IN_CFLAGS     = 0x20000,
	Keywords::RED_ALL_CFLAGS    = RED_DOUBLE_CFLAGS|RED_IN_CFLAGS;

KeywordsFlags::Type
KeywordsFlags::get_type(std::string arch, std::string keywords)
{
	Type mask = KEY_EMPTY;
	std::vector<std::string> arr_keywords = split_string(keywords);
	for(std::vector<std::string>::iterator it = arr_keywords.begin(); it != arr_keywords.end(); ++it) {
		switch((*it)[0]) {
			case '~':
				if(it->substr(1) == arch) {
					mask |= KEY_UNSTABLE;
				}
				else {
					mask |= KEY_ALIENUNSTABLE;
				}
				break;
			case '-':
				if(it->substr(1) == "*") {
					mask |= KEY_MINUSASTERISK;
				}
				else if(it->substr(1) == arch) {
					mask |= KEY_MINUSKEYWORD;
				}
				break;
			default:
				if(*it == arch) {
					mask |= KEY_STABLE;
				}
				else {
					mask |= KEY_ALIENSTABLE;
				}
				break;
		}
	}
	return mask;
}

