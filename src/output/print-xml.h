// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Bob Shaffer II <bob.shaffer.2 at gmail.com>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_OUTPUT_PRINT_XML_H_
#define SRC_OUTPUT_PRINT_XML_H_ 1

#include <string>

#include "eixTk/eixint.h"
#include "eixTk/null.h"
#include "eixTk/ptr_list.h"
#include "portage/package.h"

class EixRc;
class DBHeader;
class VarDbPkg;
class FormbPkg;
class PrintFormat;
class SetStability;

class PrintXml {
	protected:
		bool started;
		bool print_overlay;
		enum{ KW_NONE, KW_BOTH, KW_FULL, KW_EFF, KW_FULLS, KW_EFFS } keywords_mode;

		const DBHeader *hdr;
		VarDbPkg *var_db_pkg;
		const PrintFormat *print_format;
		const SetStability *stability;
		std::string portdir;
		std::string dateformat;

		eix::ptr_list<Package>::size_type count;
		std::string curcat;

		void clear(EixRc *eixrc);
		void runclear();

	public:
		typedef eix::UNumber XmlVersion;
		static const XmlVersion current = 10;

		void init(const DBHeader *header, VarDbPkg *vardb, const PrintFormat *printformat, const SetStability *set_stability, EixRc *eixrc, const std::string &port_dir) ATTRIBUTE_NONNULL_
		{
			hdr = header;
			var_db_pkg = vardb;
			print_format = printformat;
			stability = set_stability;
			portdir = port_dir;
			clear(eixrc);
		}

		explicit PrintXml(const DBHeader *header, VarDbPkg *vardb, const PrintFormat *printformat, const SetStability *set_stability, EixRc *eixrc, const std::string &port_dir) ATTRIBUTE_NONNULL_
		{ init(header, vardb, printformat, set_stability, eixrc, port_dir); }

		PrintXml() : hdr(NULLPTR), var_db_pkg(NULLPTR), print_format(NULLPTR), stability(NULLPTR)
		{ clear(NULLPTR); }

		void start();
		void package(Package *pkg) ATTRIBUTE_NONNULL_;
		void finish();
		static std::string escape_xmlstring(const std::string &s);

		~PrintXml()
		{ finish(); }
};

#endif  // SRC_OUTPUT_PRINT_XML_H_
