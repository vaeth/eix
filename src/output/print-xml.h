// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Bob Shaffer II <bob.shaffer.2 at gmail.com>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_OUTPUT_PRINT_XML_H_
#define SRC_OUTPUT_PRINT_XML_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/null.h"
#include "eixTk/ptr_container.h"
#include "output/print-formats.h"
#include "portage/package.h"

class EixRc;
class DBHeader;
class VarDbPkg;
class PrintFormat;
class SetStability;

class PrintXml FINAL : public PrintFormats {
	protected:
		bool started;
		bool print_overlay;
		enum { KW_NONE, KW_BOTH, KW_FULL, KW_EFF, KW_FULLS, KW_EFFS } keywords_mode;

		const DBHeader *hdr;
		VarDbPkg *var_db_pkg;
		const PrintFormat *print_format;
		const SetStability *stability;
		std::string portdir;
		std::string dateformat;

		typedef eix::ptr_container<std::vector<Package *> > PackageList;
		PackageList::size_type count;
		std::string curcat;

		void clear(EixRc *eixrc);
		void runclear();

	public:
		typedef eix::UNumber XmlVersion;
		static CONSTEXPR const XmlVersion current = 15;

		ATTRIBUTE_NONNULL_ void init(const DBHeader *header, VarDbPkg *vardb, const PrintFormat *printformat, const SetStability *set_stability, EixRc *eixrc, const std::string& port_dir) {
			hdr = header;
			var_db_pkg = vardb;
			print_format = printformat;
			stability = set_stability;
			portdir = port_dir;
			clear(eixrc);
		}

		ATTRIBUTE_NONNULL_ PrintXml(const DBHeader *header, VarDbPkg *vardb, const PrintFormat *printformat, const SetStability *set_stability, EixRc *eixrc, const std::string& port_dir) {
			init(header, vardb, printformat, set_stability, eixrc, port_dir);
		}

		PrintXml() : hdr(NULLPTR), var_db_pkg(NULLPTR), print_format(NULLPTR), stability(NULLPTR) {
			clear(NULLPTR);
		}

		void start() OVERRIDE;
		ATTRIBUTE_NONNULL_ void package(Package *pkg) OVERRIDE;
		void finish() OVERRIDE;
		static std::string escape_xmlstring(bool quoted, const std::string& s);
		static void say_xml_element(const std::string& prefix, const std::string& name, const std::string& content);

		~PrintXml() {
			finish();
		}
};

#endif  // SRC_OUTPUT_PRINT_XML_H_
