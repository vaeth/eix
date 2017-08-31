// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <unistd.h>

#include <cstdlib>

#include <string>
#include <vector>

#include "database/header.h"
#include "database/io.h"
#include "database/package_reader.h"
#include "eixTk/ansicolor.h"
#include "eixTk/argsreader.h"
#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/filenames.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/parseerror.h"
#include "eixTk/utils.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "main/main.h"
#include "output/formatstring-print.h"
#include "output/formatstring.h"
#include "portage/conf/portagesettings.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/package.h"
#include "portage/packagetree.h"
#include "portage/set_stability.h"
#include "portage/vardbpkg.h"
#include "various/drop_permissions.h"

#define VAR_DB_PKG "/var/db/pkg/"

using std::string;
using std::vector;

static void print_help();
ATTRIBUTE_NONNULL_ static void init_db(const char *file, Database *db, DBHeader *header, PackageReader **reader, PortageSettings *ps);
ATTRIBUTE_NONNULL_ static void set_virtual(PrintFormat *fmt, const DBHeader& header, const string& eprefix_virtual);
ATTRIBUTE_NONNULL_ static void print_changed_package(Package *op, Package *np);
ATTRIBUTE_NONNULL_ static void print_found_package(Package *p);
ATTRIBUTE_NONNULL_ static void print_lost_package(Package *p);
ATTRIBUTE_NONNULL_ static void parseFormat(Node **format, const char *varname, EixRc *rc);

static PortageSettings *portagesettings;
static SetStability   *set_stability_old, *set_stability_new;
static PrintFormat    *format_for_new, *format_for_old;
static VarDbPkg       *varpkg_db;
static DBHeader       *old_header, *new_header;
static PackageReader  *old_reader, *new_reader;
static Node           *format_new, *format_delete, *format_changed;

static void print_help() {
	eix::say(_("Usage: %s [options] old-cache [new-cache]\n"
"\n"
" -Q, --quick (toggle)    do (not) read unguessable slots of installed packages\n"
"     --care              always read slots of installed packages\n"
"     --deps-installed    always read deps of installed packages\n"
" -q, --quiet (toggle)    (no) output\n"
"     --ansi              reset the ansi 256 color palette\n"
" -n, --nocolor           don't use colors in output\n"
" -F, --force-color       force colors on things that are not a terminal\n"
"     --dump              dump variables to stdout\n"
"     --dump-defaults     dump default values of variables\n"
"     --print             print the expanded value of a variable\n"
"     --known-vars        print all variable names known to --print\n"
"\n"
" -h, --help              show a short help screen\n"
" -V, --version           show version-string\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.")) % program_name;
}

bool cli_show_help(false),
	cli_show_version(false),
	cli_dump_eixrc(false),
	cli_dump_defaults(false),
	cli_known_vars(false),
	cli_ansi(false),
	cli_quick,
	cli_care,
	cli_deps_installed,
	cli_quiet;

const char *var_to_print(NULLPTR);

enum cli_options {
	O_DUMP = 300,
	O_DUMP_DEFAULTS,
	O_KNOWN_VARS,
	O_PRINT_VAR,
	O_CARE,
	O_DEPS_INSTALLED,
	O_ANSI,
	O_FORCE_COLOR
};


/**
Arguments and options
**/
class EixDiffOptionList : public OptionList {
	public:
		EixDiffOptionList();
};

EixDiffOptionList::EixDiffOptionList() {
	EMPLACE_BACK(Option, ("help",         'h',    Option::BOOLEAN_T, &cli_show_help)); /* show a short help screen */
	EMPLACE_BACK(Option, ("version",      'V',    Option::BOOLEAN_T, &cli_show_version));
	EMPLACE_BACK(Option, ("dump",         O_DUMP, Option::BOOLEAN_T, &cli_dump_eixrc));
	EMPLACE_BACK(Option, ("dump-defaults", O_DUMP_DEFAULTS, Option::BOOLEAN_T, &cli_dump_defaults));
	EMPLACE_BACK(Option, ("print",        O_PRINT_VAR, Option::STRING, &var_to_print));
	EMPLACE_BACK(Option, ("known-vars",   O_KNOWN_VARS, Option::BOOLEAN_T, &cli_known_vars));
	EMPLACE_BACK(Option, ("nocolor",      'n',    Option::BOOLEAN_T, &(format_for_new->no_color)));
	EMPLACE_BACK(Option, ("force-color",  'F',    Option::BOOLEAN_F, &(format_for_new->no_color)));
	EMPLACE_BACK(Option, ("quick",        'Q',    Option::BOOLEAN,   &cli_quick));
	EMPLACE_BACK(Option, ("care",         O_CARE, Option::BOOLEAN_T, &cli_care));
	EMPLACE_BACK(Option, ("deps_installed", O_DEPS_INSTALLED, Option::BOOLEAN_T, &cli_deps_installed));
	EMPLACE_BACK(Option, ("quiet",        'q',    Option::BOOLEAN,   &cli_quiet));
	EMPLACE_BACK(Option, ("ansi",         O_ANSI, Option::BOOLEAN_T, &cli_ansi));
}

static void init_db(const char *file, Database *db, DBHeader *header, PackageReader **reader, PortageSettings *ps) {
	if(likely(db->openread(file))) {
		string errtext;
		if(likely(db->read_header(header, &errtext, 37))) {
			*reader = new PackageReader(db, *header, ps);
			header->set_priorities(ps);
			ps->store_world_sets(&(header->world_sets));
			return;
		}
		eix::say_error(_("error in database file %s: %s")) % file % errtext;
	} else {
		eix::say_error(_("cannot open database file %s for reading (mode = 'rb')")) % file;
	}
	std::exit(EXIT_FAILURE);
}

static void set_virtual(PrintFormat *fmt, const DBHeader& header, const string& eprefix_virtual) {
	if(header.countOverlays() == 0) {
		return;
	}
	fmt->clear_virtual(header.countOverlays());
	for(ExtendedVersion::Overlay i(1); i != header.countOverlays(); ++i)
		fmt->set_as_virtual(i, is_virtual((eprefix_virtual + header.getOverlay(i).path).c_str()));
}

class DiffReaders {
	public:
		ATTRIBUTE_NONNULL_ typedef void (*lost_func) (Package *p);
		ATTRIBUTE_NONNULL_ typedef void (*found_func) (Package *p);
		ATTRIBUTE_NONNULL_ typedef void (*changed_func) (Package *p1, Package *p2);

		lost_func lost_package;
		found_func found_package;
		changed_func changed_package;

		ATTRIBUTE_NONNULL_ DiffReaders(VarDbPkg *vardbpkg, PortageSettings *portage_settings, bool only_installed, bool compare_slots, bool separate_deleted) :
			m_vardbpkg(vardbpkg), m_portage_settings(portage_settings), m_only_installed(only_installed),
			m_slots(compare_slots), m_separate_deleted(separate_deleted) {
		}

		/**
		Diff the trees and run callbacks
		**/
		int diff() {
			old_read = new_read = true;
			int prev_compare(0);
			for(;;) {
				if(likely(prev_compare <= 0)) {
					if(unlikely(!doread_old())) {
						break;
					}
				}
				if(likely(prev_compare >= 0)) {
					if(unlikely(!doread_new())) {
						prev_compare = 1;
						break;
					}
				}
				prev_compare = old_pkg->compare_catname(*new_pkg);
				if(likely(prev_compare == 0)) {
					handle_equal_packages();
				} else if(prev_compare < 0) {
					handle_old_package();
				} else {
					handle_new_package();
				}
			}
			if(unlikely(prev_compare != 0)) {
				if(prev_compare < 0) {
					handle_new_package();
				} else {
					handle_old_package();
				}
			}
			if(m_separate_deleted) {
				for(vector<Package *>::iterator it(lost_list.begin());
					likely(it != lost_list.end()); ++it) {
					lost_package(*it);
					delete(*it);
				}
				lost_list.clear();
			}
			while(doread_old()) {
				lost_package(old_pkg);
				delete(old_pkg);
			}
			if(m_separate_deleted) {
				for(vector<Package *>::iterator it(found_list.begin());
					likely(it != found_list.end()); ++it) {
					found_package(*it);
					delete(*it);
				}
				found_list.clear();
			}
			while(doread_new()) {
				found_package(new_pkg);
				delete(new_pkg);
			}
			const char *err_cstr(old_reader->get_errtext());
			if(likely(err_cstr == NULLPTR)) {
				err_cstr = new_reader->get_errtext();
			}
			bool ret(EXIT_SUCCESS);
			if(unlikely(err_cstr != NULLPTR)) {
				eix::say_error() % err_cstr;
				ret = EXIT_FAILURE;
			}
			delete old_reader;
			delete new_reader;
			return ret;
		}

	private:
		VarDbPkg *m_vardbpkg;
		PortageSettings *m_portage_settings;
		bool m_only_installed, m_slots, m_separate_deleted;

		// These are actually local variables to diff() but used for the subsequent functions
		bool old_read, new_read;
		vector<Package *> lost_list, found_list;
		Package *old_pkg, *new_pkg;

		bool doread_old() {
			if(likely(old_read)) {
				if(likely(old_reader->next())) {
					if(likely((old_pkg = old_reader->release()) != NULLPTR)) {
						set_stability_old->set_stability(old_pkg);
						return true;
					}
				}
				old_read = false;
			}
			return false;
		}

		bool doread_new() {
			if(likely(new_read)) {
				if(likely(new_reader->next())) {
					if(likely((new_pkg = new_reader->release()) != NULLPTR)) {
						set_stability_new->set_stability(new_pkg);
						return true;
					}
				}
				new_read = false;
			}
			return false;
		}

		bool best_differs() {
			return new_pkg->differ(*old_pkg, m_vardbpkg, m_portage_settings, true, m_only_installed, m_slots);
		}

		void handle_equal_packages() {
			if(unlikely(best_differs())) {
				changed_package(old_pkg, new_pkg);
			}
			delete old_pkg;
			delete new_pkg;
		}

		void handle_old_package() {
			if(m_separate_deleted) {
				lost_list.PUSH_BACK(old_pkg);
			} else {
				lost_package(old_pkg);
				delete old_pkg;
			}
		}

		void handle_new_package() {
			if(m_separate_deleted) {
				found_list.PUSH_BACK(new_pkg);
			} else {
				found_package(new_pkg);
				delete new_pkg;
			}
		}
};

/*
 * Diff everything from old-tree with the according package from new-tree.
 * They diff if
 * a) the package does not exist in the new tree :) or
 * b) the new package has a different best-match than the old. */

static void print_changed_package(Package *op, Package *np) {
	Package *p[2] = { op, np };
	format_for_new->print(p, get_diff_package_property, format_changed, new_header, varpkg_db, portagesettings, set_stability_new);
}

static void print_found_package(Package *p) {
	format_for_new->print(p, format_new, new_header, varpkg_db, portagesettings, set_stability_new);
}

static void print_lost_package(Package *p) {
	format_for_old->print(p, format_delete, old_header, varpkg_db, portagesettings, set_stability_old);
}

static void parseFormat(Node **format, const char *varname, EixRc *rc) {
	string errtext;
	if(likely((format_for_new->parseFormat(format, (*rc)[varname].c_str(), &errtext)))) {
		return;
	}
	eix::say_error(_("problems while parsing %s: %s"))
			% varname % errtext;
	std::exit(EXIT_FAILURE);
}

int run_eix_diff(int argc, char *argv[]) {
	// Initialize static classes
	Eapi::init_static();
	Category::init_static();
	ExtendedVersion::init_static();
	PortageSettings::init_static();
	PrintFormat::init_static();
	format_for_new = new PrintFormat(get_package_property);

	string old_file, new_file;

	EixRc& rc(get_eixrc(DIFF_VARS_PREFIX)); {
		string errtext;
		bool success(drop_permissions(&rc, &errtext));
		if(!errtext.empty()) {
			eix::say_error() % errtext;
		}
		if(!success) {
			return EXIT_FAILURE;
		}
	}

	Depend::use_depend = rc.getBool("DEP");
	Version::use_required_use = rc.getBool("REQUIRED_USE");

	cli_quick = rc.getBool("QUICKMODE");
	cli_care  = rc.getBool("CAREMODE");
	cli_deps_installed = rc.getBool("DEPS_INSTALLED");
	cli_quiet = rc.getBool("QUIETMODE");

	format_for_new->no_color = (rc.getBool("NOCOLORS") ? true :
		(rc.getBool("FORCE_COLORS") ? false : (isatty(1) == 0)));

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, EixDiffOptionList());
	ArgumentReader::iterator current_param(argreader.begin());

	if(unlikely(cli_ansi)) {
		AnsiColor::AnsiPalette();
	}

	if(unlikely(var_to_print != NULLPTR)) {
		if(rc.print_var(var_to_print)) {
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;
	}

	if(unlikely(cli_show_help)) {
		print_help();
		return EXIT_SUCCESS;
	}

	if(unlikely(cli_show_version)) {
		dump_version();
	}

	if(unlikely(cli_known_vars)) {
		rc.known_vars();
		return EXIT_SUCCESS;
	}

	if(unlikely(cli_dump_eixrc || cli_dump_defaults)) {
		rc.dumpDefaults(stdout, cli_dump_defaults);
		return EXIT_SUCCESS;
	}

	if(unlikely(cli_quiet)) {
		if(!freopen(DEV_NULL, "w", stdout)) {
			eix::say_error(_("cannot redirect to \"%s\"")) % DEV_NULL;
			std::exit(EXIT_FAILURE);
		}
	}

	bool have_new(false);
	if(unlikely((current_param != argreader.end()) && (current_param->type == Parameter::ARGUMENT))) {
		old_file = current_param->m_argument;
		++current_param;
		if(unlikely((current_param != argreader.end()) && (current_param->type == Parameter::ARGUMENT))) {
			new_file = current_param->m_argument;
			have_new = true;
		}
	} else {
		old_file = rc["EIX_PREVIOUS"];
	}
	if(!have_new) {
		new_file = rc["EIX_CACHEFILE"];
	}

	format_for_new->setupResources(&rc);
	format_for_new->slot_sorted = false;
	format_for_new->style_version_lines = false;
	format_for_new->setupColors();

	parseFormat(&format_new, "DIFF_FORMAT_NEW", &rc);
	parseFormat(&format_delete, "DIFF_FORMAT_DELETE", &rc);
	parseFormat(&format_changed, "DIFF_FORMAT_CHANGED", &rc);

	ParseError parse_error;
	portagesettings = new PortageSettings(&rc, &parse_error, true, false);

	varpkg_db = new VarDbPkg(rc["EPREFIX_INSTALLED"] + VAR_DB_PKG,
		!cli_quick, cli_care, cli_deps_installed,
		rc.getBool("RESTRICT_INSTALLED"), rc.getBool("CARE_RESTRICT_INSTALLED"),
		rc.getBool("USE_BUILD_TIME"));
	varpkg_db->check_installed_overlays = rc.getBoolText("CHECK_INSTALLED_OVERLAYS", "repository");

	bool local_settings(rc.getBool("LOCAL_PORTAGE_CONFIG"));
	bool always_accept_keywords(rc.getBool("ALWAYS_ACCEPT_KEYWORDS"));
	set_stability_old = new SetStability(portagesettings, local_settings, true, always_accept_keywords);
	set_stability_new = new SetStability(portagesettings, local_settings, false, always_accept_keywords);
	format_for_new->recommend_mode = rc.getLocalMode("RECOMMEND_LOCAL_MODE");

	Database new_db;
	new_header = new DBHeader;
	init_db(new_file.c_str(), &new_db, new_header, &new_reader, portagesettings);

	Database old_db;
	old_header = new DBHeader;
	init_db(old_file.c_str(), &old_db, old_header, &old_reader, portagesettings);

	format_for_new->set_overlay_translations(NULLPTR);

	format_for_old = new PrintFormat(*format_for_new);

	string eprefix_virtual(rc["EPREFIX_VIRTUAL"]);
	set_virtual(format_for_old, *old_header, eprefix_virtual);
	set_virtual(format_for_new, *new_header, eprefix_virtual);

	DiffReaders differ(varpkg_db, portagesettings,
		rc.getBool("DIFF_ONLY_INSTALLED"),
		!rc.getBool("DIFF_NO_SLOTS"),
		rc.getBool("DIFF_SEPARATE_DELETED"));

	differ.lost_package    = print_lost_package;
	differ.found_package   = print_found_package;
	differ.changed_package = print_changed_package;

	int ret(differ.diff());
	eix::print() % format_for_new->color_end;

	delete varpkg_db;
	delete portagesettings;
	return ret;
}
