// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include "eixTk/outputstring.h"
#include "eixTk/stringlist.h"
#include "portage/version.h"

void Version::reasons_string(OutputString *s, const OutputString& skip, const OutputString& sep) const {
	bool psep(false);
	for(Reasons::const_iterator it(reasons.begin()); it != reasons.end(); ++it) {
		if(psep) {
			s->append(sep);
		} else {
			psep = true;
		}
		it->append_to_string(s, skip);
	}
}
