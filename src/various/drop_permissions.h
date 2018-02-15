// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_VARIOUS_DROP_PERMISSIONS_H_
#define SRC_VARIOUS_DROP_PERMISSIONS_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>

#include "eixTk/attribute.h"

class EixRc;

ATTRIBUTE_NONNULL_ bool drop_permissions(EixRc *eix, std::string *errtext);

#endif  // SRC_VARIOUS_DROP_PERMISSIONS_H_
