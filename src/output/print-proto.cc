// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "output/print-proto.h"
#include <config.h>  // IWYU pragma: keep

#ifndef WITH_PROTOBUF
#include <cstdlib>
#endif

#ifdef WITH_PROTOBUF
#include "output/eix.pb.h"
#endif

#ifndef WITH_PROTOBUF
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#else
#include "eixTk/null.h"
#endif

#ifdef WITH_PROTOBUF

void PrintProto::start() {
	eix_output = new EixOutput();
}

void PrintProto::package(Package *package) {
	if (eix_output == NULLPTR) {
		return;
	}
	// TOOD: build eix_output
}

void PrintProto::finish() {
	if (eix_output == NULLPTR) {
		return;
	}
	// TODO: serialize eix_output
	delete eix_output;
	eix_output = NULLPTR;
}

#else

void PrintProto::start() {}
void PrintProto::finish() {}
void PrintProto::package(Package *) {
	eix::say_error(_("protobuf format is not compiled in"));
	std::exit(EXIT_FAILURE);
}

#endif
