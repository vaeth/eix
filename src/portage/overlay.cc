// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/overlay.h"
#include <config.h>  // IWYU pragma: keep

#include "eixTk/null.h"

void OverlayIdent::init(const char *patharg, const char *labelarag) {
	if(patharg == NULLPTR) {
		know_path = false;
	} else {
		path = patharg;
		know_path = true;
	}
	if(labelarag == NULLPTR) {
		know_label = false;
		return;
	}
	label = labelarag;
	know_label = true;
}
