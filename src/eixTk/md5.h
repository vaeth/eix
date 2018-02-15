// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_MD5_H_
#define SRC_EIXTK_MD5_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>

#include "eixTk/attribute.h"

ATTRIBUTE_NONNULL_ bool verify_md5sum(const char *file, const std::string& md5sum);

#endif  // SRC_EIXTK_MD5_H_
