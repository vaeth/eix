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

using namespace std;

const MaskFlags::MaskType
	MaskFlags::MASK_NONE,
	MaskFlags::MASK_PACKAGE,
	MaskFlags::MASK_PROFILE,
	MaskFlags::MASK_HARD,
	MaskFlags::MASK_SYSTEM;

const KeywordsFlags::KeyType
	KeywordsFlags::KEY_EMPTY,
	KeywordsFlags::KEY_STABLE,
	KeywordsFlags::KEY_ARCHSTABLE,
	KeywordsFlags::KEY_ARCHUNSTABLE,
	KeywordsFlags::KEY_ALIENSTABLE,
	KeywordsFlags::KEY_ALIENUNSTABLE,
	KeywordsFlags::KEY_MINUSKEYWORD,
	KeywordsFlags::KEY_MINUSASTERISK,
	KeywordsFlags::KEY_SOMESTABLE,
	KeywordsFlags::KEY_SOMEUNSTABLE,
	KeywordsFlags::KEY_TILDESTARMATCH;

KeywordsFlags::KeyType
KeywordsFlags::get_keyflags(const std::set<string> &accepted_keywords, const string &keywords, bool obsolete_minus)
{
	KeyType m = KEY_EMPTY;
	std::set<string> keywords_set;
	make_set<string>(keywords_set, split_string(keywords));
	for(std::set<string>::const_iterator it = keywords_set.begin(); it != keywords_set.end(); ++it) {
		bool found = false;
		if((*it)[0] == '-') {
			if(*it == "-*")
				m |= KEY_MINUSASTERISK;
			else if(accepted_keywords.find(it->substr(1)) != accepted_keywords.end())
				m |= KEY_MINUSKEYWORD;
			if(!obsolete_minus)
				continue;
		}
		if(accepted_keywords.find(*it) != accepted_keywords.end()) {
			found = true;
			m |= KEY_STABLE;
		}
		if((*it)[0] == '~') {
			if(found)
				m |= KEY_ARCHUNSTABLE;
			else {
				if(accepted_keywords.find(it->substr(1)) != accepted_keywords.end())
					m |= KEY_ARCHUNSTABLE;
				else
					m |= KEY_ALIENUNSTABLE;
			}
		}
		else {
			if((*it)[0] != '-')
				m |= found ? KEY_ARCHSTABLE : KEY_ALIENSTABLE;
		}
	}
	if(m & KEY_STABLE)
		return m;
	if(accepted_keywords.find("**") != accepted_keywords.end())
		return (m | KEY_STABLE);
	if(m & KEY_SOMESTABLE) {
		if(accepted_keywords.find("*") != accepted_keywords.end())
			return (m | KEY_STABLE);
	}
	if(m & KEY_TILDESTARMATCH) {
		if(accepted_keywords.find("~*") != accepted_keywords.end())
			return (m | KEY_STABLE);
	}
	return m;
}

const Keywords::Redundant
	Keywords::RED_NOTHING,
	Keywords::RED_DOUBLE,
	Keywords::RED_DOUBLE_LINE,
	Keywords::RED_MIXED,
	Keywords::RED_WEAKER,
	Keywords::RED_STRANGE,
	Keywords::RED_NO_CHANGE,
	Keywords::RED_MINUSASTERISK,
	Keywords::RED_IN_KEYWORDS,
	Keywords::RED_ALL_KEYWORDS,
	Keywords::RED_MASK,
	Keywords::RED_DOUBLE_MASK,
	Keywords::RED_IN_MASK,
	Keywords::RED_UNMASK,
	Keywords::RED_DOUBLE_UNMASK,
	Keywords::RED_IN_UNMASK,
	Keywords::RED_ALL_MASK,
	Keywords::RED_ALL_UNMASK,
	Keywords::RED_ALL_MASKSTUFF,
	Keywords::RED_DOUBLE_USE,
	Keywords::RED_IN_USE,
	Keywords::RED_ALL_USE,
	Keywords::RED_DOUBLE_CFLAGS,
	Keywords::RED_IN_CFLAGS,
	Keywords::RED_ALL_CFLAGS;
