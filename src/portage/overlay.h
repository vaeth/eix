// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)                                                         
//   Wolfgang Frisch <xororand@users.sourceforge.net>                    
//   Emil Beinroth <emilbeinroth@gmx.net>                                
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     

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
