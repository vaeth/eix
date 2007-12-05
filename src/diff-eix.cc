// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

/*
 * Everyone wanted something like esync from esearch .. so here it is!
 */

#include <config.h>

#include <global.h>
#include <eixTk/argsreader.h>
#include <eixTk/stringutils.h>
#include <eixTk/exceptions.h>
#include <eixTk/sysutils.h>
#include <eixTk/filenames.h>
#include <eixTk/utils.h>

#include <database/header.h>
#include <portage/packagetree.h>
#include <portage/set_stability.h>

#include <portage/vardbpkg.h>
#include <portage/conf/cascadingprofile.h>
#include <portage/conf/portagesettings.h>

#include <output/formatstring.h>
#include <output/formatstring-print.h>

#include <string>
#include <sstream>

#include <dirent.h>
#include <sys/stat.h> /* chmod(..) */
#include <signal.h>   /* signal handlers */

#define VAR_DB_PKG "/var/db/pkg/"


#define INFO(...) printf(__VA_ARGS__)

using namespace std;

void   signal_handler(int sig);

PortageSettings *portagesettings;
SetStability   *set_stability_old, *set_stability_new;
PrintFormat     format_for_new(get_package_property, print_package_property);
PrintFormat     format_for_old;
VarDbPkg       *varpkg_db;
DBHeader        old_header, new_header;
Node           *format_new, *format_delete, *format_changed;

static void
print_help(int ret)
{
	printf(
		"diff-eix [options] old-cache [new-cache]\n"
		"\n"
		" -Q, --quick (toggle)    do (not) read unguessable slots of installed packages\n"
		"     --care              always read slots of installed packages\n"
		" -q, --quiet (toggle)    (no) output\n"
		" -n, --nocolor           don't use colors in output\n"
		" -F, --force-color       force colors on things that are not a terminal\n"
		"     --dump              dump variables to stdout\n"
		"     --dump-defaults     dump default values of variables\n"
		"     --print             print the expanded value of a variable\n"
		"\n"
		" -h, --help              show a short help screen\n"
		" -V, --version           show version-string\n"
		"\n"
		"If you omit the new-cache parameter, we'll use the\n"
		"default cache-file "EIX_CACHEFILE".\n"
		"\n"
		"You can contact the developers in #eix on irc.freenode.net or on\n"
		"the sourceforge-page "PACKAGE_BUGREPORT".\n"
		"There is also a wiki at "EIX_WIKI".\n"
		"This program is covered by the GNU General Public License. See COPYING for\n"
		"further information.\n"
		);

	if(ret != -1) {
		exit(ret);
	}
}

bool cli_show_help    = false,
	 cli_show_version = false,
	 cli_dump_eixrc   = false,
	 cli_dump_defaults = false,
	 cli_quick,
	 cli_care,
	 cli_quiet;

const char *var_to_print = NULL;

enum cli_options {
	O_DUMP = 300,
	O_DUMP_DEFAULTS,
	O_PRINT_VAR,
	O_CARE,
	O_FORCE_COLOR
};

/** Arguments and shortopts. */
static struct Option long_options[] = {

	Option("help",         'h',    Option::BOOLEAN_T, &cli_show_help), /* show a short help screen */
	Option("version",      'V',    Option::BOOLEAN_T, &cli_show_version),
	Option("dump",         O_DUMP, Option::BOOLEAN_T, &cli_dump_eixrc),
	Option("dump-deafults",O_DUMP_DEFAULTS, Option::BOOLEAN_T, &cli_dump_defaults),
	Option("print",        O_PRINT_VAR, Option::STRING,&var_to_print),
	Option("nocolor",      'n',    Option::BOOLEAN_T, &(format_for_new.no_color)),
	Option("force-color",  'F',    Option::BOOLEAN_F, &(format_for_new.no_color)),
	Option("quick",        'Q',    Option::BOOLEAN,   &cli_quick),
	Option("care",         O_CARE, Option::BOOLEAN_T, &cli_care),
	Option("quiet",        'q',    Option::BOOLEAN,   &cli_quiet),

	Option(0, 0)
};

void
load_db(const char *file, DBHeader *header, PackageTree *body)
{
	FILE *fp = fopen(file, "rb");

	if(fp == NULL) {
		throw(ExBasic("Can't open the database file %s for reading (mode = 'rb')\n", file));
	}

	io::read_header(fp, *header);
	if(!header->isCurrent()) {
		fprintf(stderr, "%s uses an obsolete database format (%u, current is %u).\n",
				file, uint(header->version), uint(DBHeader::current));
		exit(1);
	}

	io::read_packagetree(fp, header->size, *body);
	fclose(fp);
}

void
set_virtual(PrintFormat *fmt, const DBHeader &header, const string &eprefix_virtual)
{
	if(!header.countOverlays())
		return;
	fmt->clear_virtual(header.countOverlays());
	for(Version::Overlay i = 1; i != header.countOverlays(); i++)
		fmt->set_as_virtual(i, is_virtual((eprefix_virtual + header.getOverlay(i).path).c_str()));
}

class DiffTrees
{
	public:
		typedef void (*lost_func) (Package *p);
		typedef void (*found_func) (Package *p);
		typedef void (*changed_func) (Package *p1, Package *p2);

		lost_func lost_package;
		found_func found_package;
		changed_func changed_package;

		DiffTrees(VarDbPkg *vardbpkg, bool only_installed, bool compare_slots) :
			m_vardbpkg(vardbpkg), m_only_installed(only_installed), m_slots(compare_slots)
		{ }

		/// Diff the trees and run callbacks
		void diff(PackageTree &old_tree, PackageTree &new_tree)
		{
			// Diff every category from the old tree with the category from the new tree
			for(PackageTree::iterator old_cat = old_tree.begin();
				old_cat != old_tree.end();
				++old_cat)
			{
				diff_category(**old_cat, new_tree[old_cat->name()]);
			}

			// Know we've only new package in the new_tree
			for(PackageTree::iterator new_cat = new_tree.begin();
				new_cat != new_tree.end();
				++new_cat)
			{
				// Print packages as new
				for_each(new_cat->begin(), new_cat->end(), found_package);
			}
		}
	private:
		VarDbPkg *m_vardbpkg;
		bool m_only_installed, m_slots;

		bool best_differs(const Package *new_pkg, const Package *old_pkg)
		{ return new_pkg->differ(*old_pkg, m_vardbpkg, true, m_only_installed, m_slots); }

		/// Diff two categories and run callbacks.
		// Remove already diffed packages.
		void diff_category(Category &old_cat, Category &new_cat)
		{
			Category::iterator old_pkg = old_cat.begin();

			while(old_pkg != old_cat.end())
			{
				Category::iterator new_pkg = new_cat.find(old_pkg->name);

				// Lost a package
				if(new_pkg == new_cat.end())
					lost_package(*old_pkg);

				// Best version differs
				else if(best_differs(*new_pkg, *old_pkg))
					changed_package(*old_pkg, *new_pkg);

				// Remove the new package
				if(new_pkg != new_cat.end())
				{
					delete *new_pkg;
					new_cat.erase(new_pkg);
				}

				// Remove the old packages
				delete *old_pkg;
				old_pkg = old_cat.erase(old_pkg);
			}
		}
};

/* Diff everything from old-tree with the according package from new-tree.
 * They diff if
 * a) the package does not exist in the new tree :) or
 * b) the new package has a different best-match than the old. */

void
print_changed_package(Package *op, Package *np)
{
	Package *p[2];
	p[0] = op;
	p[1] = np;
	format_for_new.print(p, print_diff_package_property, get_diff_package_property, format_changed, &new_header, varpkg_db, portagesettings, set_stability_new);
}

void
print_found_package(Package *p)
{
	format_for_new.print(p, format_new, &new_header, varpkg_db, portagesettings, set_stability_new);
}

void
print_lost_package(Package *p)
{
	format_for_old.print(p, format_delete, &old_header, varpkg_db, portagesettings, set_stability_old);
}


int
run_diff_eix(int argc, char *argv[])
{
	string old_file, new_file;

	format_for_new.no_color   = (isatty(1) != 1);

	EixRc &eixrc = get_eixrc(DIFF_EIX_VARS_PREFIX);

	cli_quick = eixrc.getBool("QUICKMODE");
	cli_care  = eixrc.getBool("CAREMODE");
	cli_quiet = eixrc.getBool("QUIETMODE");
	Package::upgrade_to_best = eixrc.getBool("UPGRADE_TO_HIGHEST_SLOT");

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);
	ArgumentReader::iterator current_param = argreader.begin();

	if(cli_show_help)
		print_help(0);

	if(cli_show_version)
		dump_version(0);

	if(var_to_print) {
		eixrc.print_var(var_to_print);
		exit(0);
	}


	if(cli_dump_eixrc || cli_dump_defaults) {
		eixrc.dumpDefaults(stdout, cli_dump_defaults);
		exit(0);
	}

	if(cli_quiet)
		close(1);

	if(eixrc.getBool("FORCE_USECOLORS")) {
		format_for_new.no_color = false;
	}

	if(current_param == argreader.end() || current_param->type != Parameter::ARGUMENT) {
		print_help(1);
		throw(ExBasic("Missing cache-file."));
	}

	old_file = current_param->m_argument;
	++current_param;
	if(current_param == argreader.end() || current_param->type != Parameter::ARGUMENT) {
		new_file = eixrc["EIX_CACHEFILE"];
	}
	else {
		new_file = current_param->m_argument;
	}

	const char *varname;
	try {
		varname = "DIFF_FORMAT_NEW";
		format_new = format_for_new.parseFormat(eixrc[varname].c_str());

		varname = "DIFF_FORMAT_DELETE";
		format_delete = format_for_new.parseFormat(eixrc[varname].c_str());

		varname = "DIFF_FORMAT_CHANGED";
		format_changed = format_for_new.parseFormat(eixrc[varname].c_str());
	}
	catch(const ExBasic &e) {
		cout << "Problems while parsing " << varname << ".\n"
			<< e.getMessage() << endl;
		exit(1);
	}

	format_for_new.dateFormat       = eixrc["FORMAT_INSTALLATION_DATE"];
	format_for_new.dateFormatShort  = eixrc["FORMAT_SHORT_INSTALLATION_DATE"];
	format_for_new.color_masked     = eixrc["COLOR_MASKED"];
	format_for_new.color_unstable   = eixrc["COLOR_UNSTABLE"];
	format_for_new.color_stable     = eixrc["COLOR_STABLE"];
	format_for_new.color_overlaykey = eixrc["COLOR_OVERLAYKEY"];
	format_for_new.color_virtualkey = eixrc["COLOR_VIRTUALKEY"];
	format_for_new.color_slots      = eixrc["COLOR_SLOTS"];
	format_for_new.color_fetch      = eixrc["COLOR_FETCH"];
	format_for_new.color_mirror     = eixrc["COLOR_MIRROR"];
	format_for_new.mark_installed   = eixrc["MARK_INSTALLED"];
	format_for_new.show_slots       = eixrc.getBool("PRINT_SLOTS");
	format_for_new.colon_slots      = eixrc.getBool("COLON_SLOTS");
	format_for_new.colored_slots    = eixrc.getBool("COLORED_SLOTS");
	format_for_new.color_original   = eixrc.getBool("COLOR_ORIGINAL");
	format_for_new.color_local_mask = eixrc.getBool("COLOR_LOCAL_MASK");

	format_for_new.alpha_use        = eixrc.getBool("SORT_INST_USE_ALPHA");
	format_for_new.print_restrictions = !eixrc.getBool("NO_RESTRICTIONS");

	format_for_new.print_iuse       = eixrc.getBool("PRINT_IUSE");
	format_for_new.before_iuse      = eixrc["FORMAT_BEFORE_IUSE"];
	format_for_new.after_iuse       = eixrc["FORMAT_AFTER_IUSE"];
	format_for_new.before_coll_iuse = eixrc["FORMAT_BEFORE_COLL_IUSE"];
	format_for_new.after_coll_iuse  = eixrc["FORMAT_AFTER_COLL_IUSE"];
	format_for_new.before_slot_iuse = eixrc["FORMAT_BEFORE_SLOT_IUSE"];
	format_for_new.after_slot_iuse  = eixrc["FORMAT_AFTER_SLOT_IUSE"];

	format_for_new.tag_fetch        = eixrc["TAG_FETCH"];
	format_for_new.tag_mirror       = eixrc["TAG_MIRROR"];

	format_for_new.setupColors();

	format_for_new.tag_for_profile            = eixrc["TAG_FOR_PROFILE"];
	format_for_new.tag_for_masked             = eixrc["TAG_FOR_MASKED"];
	format_for_new.tag_for_ex_profile         = eixrc["TAG_FOR_EX_PROFILE"];
	format_for_new.tag_for_ex_masked          = eixrc["TAG_FOR_EX_MASKED"];
	format_for_new.tag_for_locally_masked     = eixrc["TAG_FOR_LOCALLY_MASKED"];
	format_for_new.tag_for_stable             = eixrc["TAG_FOR_STABLE"];
	format_for_new.tag_for_unstable           = eixrc["TAG_FOR_UNSTABLE"];
	format_for_new.tag_for_minus_asterisk     = eixrc["TAG_FOR_MINUS_ASTERISK"];
	format_for_new.tag_for_minus_keyword      = eixrc["TAG_FOR_MINUS_KEYWORD"];
	format_for_new.tag_for_alien_stable       = eixrc["TAG_FOR_ALIEN_STABLE"];
	format_for_new.tag_for_alien_unstable     = eixrc["TAG_FOR_ALIEN_UNSTABLE"];
	format_for_new.tag_for_missing_keyword    = eixrc["TAG_FOR_MISSING_KEYWORD"];
	format_for_new.tag_for_ex_unstable        = eixrc["TAG_FOR_EX_UNSTABLE"];
	format_for_new.tag_for_ex_minus_asterisk  = eixrc["TAG_FOR_EX_MINUS_ASTERISK"];
	format_for_new.tag_for_ex_minus_keyword   = eixrc["TAG_FOR_EX_MINUS_KEYWORD"];
	format_for_new.tag_for_ex_alien_stable    = eixrc["TAG_FOR_EX_ALIEN_STABLE"];
	format_for_new.tag_for_ex_alien_unstable  = eixrc["TAG_FOR_EX_ALIEN_UNSTABLE"];
	format_for_new.tag_for_ex_missing_keyword = eixrc["TAG_FOR_EX_MISSING_KEYWORD"];

	portagesettings = new PortageSettings(eixrc, true);

	varpkg_db = new VarDbPkg(eixrc["EPREFIX_INSTALLED"] + VAR_DB_PKG, !cli_quick, cli_care,
		eixrc.getBool("RESTRICT_INSTALLED"), eixrc.getBool("CARE_RESTRICT_INSTALLED"));
	varpkg_db->check_installed_overlays = eixrc.getBoolText("CHECK_INSTALLED_OVERLAYS", "repository");

	bool local_settings = eixrc.getBool("LOCAL_PORTAGE_CONFIG");
	bool always_accept_keywords = eixrc.getBool("ALWAYS_ACCEPT_KEYWORDS");
	set_stability_old = new SetStability(portagesettings, local_settings, true, always_accept_keywords);
	set_stability_new = new SetStability(portagesettings, local_settings, false, always_accept_keywords);
	format_for_new.recommend_mode = eixrc.getLocalMode("RECOMMEND_LOCAL_MODE");

	PackageTree old_tree;
	load_db(old_file.c_str(), &old_header, &old_tree);
	set_stability_old->set_stability(old_tree);

	PackageTree new_tree;
	load_db(new_file.c_str(), &new_header, &new_tree);
	set_stability_new->set_stability(new_tree);

	format_for_new.set_overlay_translations(NULL);

	format_for_old = format_for_new;

	string eprefix_virtual = eixrc["EPREFIX_VIRTUAL"];
	set_virtual(&format_for_old, old_header, eprefix_virtual);
	set_virtual(&format_for_new, new_header, eprefix_virtual);

	DiffTrees differ(varpkg_db,
		eixrc.getBool("DIFF_ONLY_INSTALLED"),
		!eixrc.getBool("DIFF_NO_SLOTS"));
	INFO("Diffing databases (%u - %u packages)\n", uint(old_tree.countPackages()), uint(new_tree.countPackages()));

	differ.lost_package    = print_lost_package;
	differ.found_package   = print_found_package;
	differ.changed_package = print_changed_package;

	differ.diff(old_tree, new_tree);

	delete varpkg_db;
	delete portagesettings;
	return 0;
}

/** On segfault: show some instructions to help us find the bug. */
void
signal_handler(int sig)
{
	if(sig == SIGSEGV)
		fprintf(stderr,
				"Received SIGSEGV - you probably found a bug in eix.\n"
				"Please post the output of eix -V along with your bugreport.\n"
				"Sorry for the inconvenience.\n");
	exit(1);
}

int
main(int argc, char** argv)
{
	/* Install signal handler for segfaults */
	signal(SIGSEGV, signal_handler);

	int ret = 0;
	try {
		ret = run_diff_eix(argc, argv);
	}
	catch(const ExBasic &e) {
		cout << e << endl;
		return 1;
	}
	return ret;
}
