// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <string>
#include <vector>

#include "eixTk/likely.h"
#include "eixTk/outputstring.h"
#include "eixTk/stringlist.h"

using std::string;
using std::vector;

void
StringListContent::append_to_string(OutputString *s, const OutputString &skip) const
{
	bool sep(false);
	for(vector<string>::const_iterator it(m_list.begin());
		likely(it != m_list.end()); ++it) {
		if(sep) {
			s->append(skip);
		} else {
			sep = true;
		}
		s->append_smart(*it);
	}
}
