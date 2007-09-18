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
			if(!found)
				m |= KEY_ALIENSTABLE;
			else if((*it)[0] != '-')
				m |= KEY_ARCHSTABLE;
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

