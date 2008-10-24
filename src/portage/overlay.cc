// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "overlay.h"

#include <eixTk/utils.h>

using namespace std;

void
OverlayIdent::readLabel(const char *Path)
{
	vector<string> lines;
	pushback_lines((string(Path) + "/profiles/repo_name").c_str(), &lines, true, false, false);
	for(vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
		if(i->empty())
			continue;
		label = *i;
		return;
	}
	label = "";
}

string
OverlayIdent::human_readable() const
{
	if(label.empty())
		return path;
	return string("\"") + label + "\" " + path;
}
