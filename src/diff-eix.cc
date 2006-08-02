/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/*
 * Everyone wanted something like esync from esearch .. so here it is!
 */

#include "../config.h"

#include <global.h>
#include <eixTk/argsreader.h>
#include <eixTk/stringutils.h>
#include <eixTk/exceptions.h>
#include <eixTk/sysutils.h>
#include <eixTk/utils.h>

#include <database/header.h>
#include <portage/packagetree.h>

#include <portage/vardbpkg.h>
#include <portage/conf/portagesettings.h>

#include <output/formatstring.h>
#include <output/formatstring-print.h>

#include <string>
#include <sstream>

#include <dirent.h>
#include <sys/stat.h> /* chmod(..) */
#include <signal.h>   /* signal handlers */

#define INFO(...) fprintf(stderr, __VA_ARGS__)

using namespace std;

void   signal_handler(int sig);

PortageSettings portagesettings;
PrintFormat     format_string(get_package_property, print_package_property);
VarDbPkg        varpkg_db("/var/db/pkg/");
Node           *format_new, *format_delete, *format_changed;

static void
print_help(int ret)
{
	printf(
		"diff-eix [options] old-cache [new-cache]\n"
		"\n"
		" -n, --nocolor           don't use colors in output\n"
		" -F, --force-color       force colors on things that are not a terminal\n"
		"     --dump              dump variables to stdout\n"
		"\n"
		" -h, --help              show a short help screen\n"
		" -V, --version           show version-string\n"
		"\n"
		"If you omit the new-cache parameter, we'll use the default cache-file ("EIX_CACHEFILE").\n"
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
	 cli_dump_eixrc   = false;

enum cli_options {
	O_DUMP = 300,
	O_FORCE_COLOR
};

/** Arguments and shortopts. */
static struct Option long_options[] = {

	Option("help",         'h',    Option::BOOLEAN_T, &cli_show_help), /* show a short help screen */
	Option("version",      'V',    Option::BOOLEAN_T, &cli_show_version),
	Option("dump",         O_DUMP, Option::BOOLEAN_T, &cli_dump_eixrc),
	Option("nocolor",      'n',    Option::BOOLEAN_T, &(format_string.no_color)),
	Option("force-color",  'F',    Option::BOOLEAN_F, &(format_string.no_color)),

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
		fprintf(stderr, "%s uses an obsolete database format (%i, current is %i).\n",
				file, header->version, DBHeader::current);
		exit(1);
	}

	io::read_packagetree(fp, header->size, *body);
	fclose(fp);
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

		DiffTrees(const PortageSettings &psettings, bool local_portage_config)
		{
			portagesettings = &psettings;
			ignore_etc_portage = !local_portage_config;
			if(local_portage_config)
				accepted_keywords = psettings.getAcceptKeywords();
			else
				accepted_keywords = Keywords::KEY_STABLE;
		}

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
		Keywords accepted_keywords;
		const PortageSettings *portagesettings;
		bool ignore_etc_portage;

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
				else if(best_differs(**old_pkg, **new_pkg))
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

		/// Return true if the best versions of both packages differ.
		bool best_differs(Package &p1, Package &p2)
		{
			if(!ignore_etc_portage)
			{
				portagesettings->user_config->setMasks(&p1);
				portagesettings->user_config->setStability(&p1, accepted_keywords);
				portagesettings->user_config->setMasks(&p2);
				portagesettings->user_config->setStability(&p2, accepted_keywords);
			}
			else
			{
				portagesettings->setStability(&p1, accepted_keywords);
				portagesettings->setStability(&p2, accepted_keywords);
			}
			Version *old_best = p1.best();
			Version *new_best = p2.best();

			return (old_best == NULL && new_best != NULL)
			    || (old_best != NULL && new_best == NULL)
			    || (old_best != NULL && new_best != NULL && *old_best != *new_best);
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
	op->installed_versions = np->installed_versions
		= varpkg_db.getInstalledString(*np);

	format_string.print(p, print_diff_package_property, get_diff_package_property, format_changed, &varpkg_db);
}

void
print_found_package(Package *p)
{
	p->installed_versions = varpkg_db.getInstalledString(*p);
	format_string.print(p, format_new, &varpkg_db);
}

void
print_lost_package(Package *p)
{
	p->installed_versions = varpkg_db.getInstalledString(*p);
	format_string.print(p, format_delete, &varpkg_db);
}


int
run_diff_eix(int argc, char *argv[])
{
	string old_file, new_file;

	if(isatty(1) != 1) {
		format_string.no_color = true;
	}

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);
	ArgumentReader::iterator current_param = argreader.begin();

	if(cli_show_help)
		print_help(0);

	if(cli_show_version)
		dump_version(0);

	EixRc &eixrc = get_eixrc();

	if(cli_dump_eixrc) {
		eixrc.dumpDefaults(stdout);
		exit(0);
	}

	if(eixrc.getBool("FORCE_USECOLORS")) {
		format_string.no_color = false;
	}

	if(current_param == argreader.end() || current_param->type != Parameter::ARGUMENT) {
		print_help(1);
		throw(ExBasic("Missing cache-file."));
	}

	old_file = current_param->m_argument;
	++current_param;
	if(current_param == argreader.end() || current_param->type != Parameter::ARGUMENT) {
		new_file = EIX_CACHEFILE;
	}
	else {
		new_file = current_param->m_argument;
	}

	string varname;
	try {
		varname = "DIFF_FORMAT_NEW";
		format_new = format_string.parseFormat(eixrc["DIFF_FORMAT_NEW"].c_str());

		varname = "DIFF_FORMAT_DELETE";
		format_delete = format_string.parseFormat(eixrc["DIFF_FORMAT_DELETE"].c_str());

		varname = "DIFF_FORMAT_CHANGED";
		format_changed = format_string.parseFormat(eixrc["DIFF_FORMAT_CHANGED"].c_str());
	}
	catch(ExBasic e) {
		cout << "Problems while parsing " << varname << "." << endl
			<< e.getMessage() << endl;
		exit(1);
	}

	format_string.color_masked     = eixrc["COLOR_MASKED"];
	format_string.color_unstable   = eixrc["COLOR_UNSTABLE"];
	format_string.color_stable     = eixrc["COLOR_STABLE"];
	format_string.color_overlaykey = eixrc["COLOR_OVERLAYKEY"];
	format_string.mark_installed   = eixrc["MARK_INSTALLED"];

	format_string.setupColors();

	DBHeader old_header;
	PackageTree old_tree;
	load_db(old_file.c_str(), &old_header, &old_tree);

	DBHeader new_header;
	PackageTree new_tree;
	load_db(new_file.c_str(), &new_header, &new_tree);


	INFO("Diffing databases (%i - %i packages)\n", old_tree.countPackages(), new_tree.countPackages());

	DiffTrees differ(portagesettings, eixrc.getBool("DIFF_LOCAL_PORTAGE_CONFIG"));
	differ.lost_package    = print_lost_package;
	differ.found_package   = print_found_package;
	differ.changed_package = print_changed_package;

	differ.diff(old_tree, new_tree);

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
	catch(ExBasic e) {
		cout << e << endl;
		return 1;
	}
	return ret;
}
