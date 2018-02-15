// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "database/header.h"
#include <config.h>  // IWYU pragma: keep

#include "eixTk/likely.h"
#include "portage/conf/portagesettings.h"
#include "portage/overlay.h"

void DBHeader::set_priorities(PortageSettings *ps) {
	for(OverlayVec::iterator it(overlays.begin());
		likely(it != overlays.end()); ++it) {
		ps->repos.set_priority(&(*it));
	}
}
