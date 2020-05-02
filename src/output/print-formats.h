// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_OUTPUT_PRINT_FORMATS_H_
#define SRC_OUTPUT_PRINT_FORMATS_H_ 1

#include <config.h>  // IWYU pragma: keep

#include "eixTk/attribute.h"
#include "portage/package.h"

class PrintFormats {
	public:
		virtual void start() {}
		virtual ATTRIBUTE_NONNULL_ void package(Package *) {}
		virtual void finish() {}
		virtual ~PrintFormats() {}
};

#endif  // SRC_OUTPUT_PRINT_FORMATS_H_
