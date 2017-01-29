// vim:set noet cinoptions=g0,t0,(0 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "portage/instversion.h"
#include <config.h>

#include "eixTk/eixint.h"
#include "eixTk/likely.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"

eix::SignedBool InstVersion::compare(const InstVersion& left, const InstVersion& right) {
	if(likely(left.know_overlay && right.know_overlay && (!left.overlay_failed) && (!right.overlay_failed))) {
		return ExtendedVersion::compare(left, right);
	} else {
		return BasicVersion::compare(left, right);
	}
}

eix::SignedBool InstVersion::compare(const InstVersion& left, const ExtendedVersion& right) {
	if(likely(left.know_overlay && !left.overlay_failed)) {
		return ExtendedVersion::compare(left, right);
	} else {
		return BasicVersion::compare(left, right);
	}
}

eix::SignedBool InstVersion::compare(const ExtendedVersion& left, const InstVersion& right) {
	if(likely(right.know_overlay && !right.overlay_failed)) {
		return ExtendedVersion::compare(left, right);
	} else {
		return BasicVersion::compare(left, right);
	}
}
