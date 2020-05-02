// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#ifndef SRC_OUTPUT_PRINT_PROTO_H_
#define SRC_OUTPUT_PRINT_PROTO_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/null.h"
#include "output/print-formats.h"
#include "portage/package.h"

class DBHeader;
class VarDbPkg;
class PrintFormat;
class SetStability;
class EixOutput;

class PrintProto : public PrintFormats {
	protected:
		const DBHeader *hdr;
		VarDbPkg *var_db_pkg;
		const PrintFormat *print_format;
		const SetStability *stability;
		std::string portdir;
		EixOutput *eix_output;

	public:
		ATTRIBUTE_NONNULL_ PrintProto(const DBHeader *header, VarDbPkg *vardb, const PrintFormat *printformat, const SetStability *set_stability, const std::string& port_dir) :
			hdr(header), var_db_pkg(vardb), print_format(printformat), stability(set_stability), portdir(port_dir), eix_output(NULLPTR) {}

		PrintProto() : hdr(NULLPTR), var_db_pkg(NULLPTR), print_format(NULLPTR), stability(NULLPTR), eix_output(NULLPTR) {}

		void start() OVERRIDE;

		ATTRIBUTE_NONNULL_ void package(Package *pkg) OVERRIDE;

		void finish() OVERRIDE;

		~PrintProto() {
			finish();
		}
};

#endif  // SRC_OUTPUT_PRINT_PROTO_H_
