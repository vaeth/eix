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

#ifndef __OVERLAY_H__
#define __OVERLAY_H__

#include <string>
#include <vector>
#include <eixTk/utils.h>

class OverlayIdent {
	public:
		std::string path, label;

		OverlayIdent(const char *Path, const char *Label)
		{ path = Path; label = Label; }

		void
		readLabel(const char *Path)
		{
			std::vector<std::string> lines;
			pushback_lines((std::string(Path) + "/profiles/repo_name").c_str(), &lines, true, false, false);
			for(std::vector<std::string>::const_iterator i = lines.begin();
				i != lines.end(); ++i) {
				if(i->empty())
					continue;
				label = *i;
				return;
			}
			label = "";
		}

		std::string
		human_readable() const
		{
			if(label.empty())
				return path;
			return std::string("\"") + label + "\" " + path;
		}
};


#endif /* __OVERLAY_H__ */
