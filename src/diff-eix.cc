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
#include <database/database.h>

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
#define ERROR(ret, ...) do { fprintf(stderr, __VA_ARGS__); fputc('\n', stdout); exit(ret); } while(0)

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

	{"help",         'h',           Option::BOOLEAN_T, (void *) &cli_show_help }, /* show a short help screen */
	{"version",      'V',           Option::BOOLEAN_T, (void *) &cli_show_version},
	{"dump",         O_DUMP,        Option::BOOLEAN_T, (void *) &cli_dump_eixrc },
	{"nocolor",      'n',           Option::BOOLEAN_T, (void *) &(format_string.no_color) },
	{"force-color",  'F',           Option::BOOLEAN_F, (void *) &(format_string.no_color) },
	{0,              0,             Option::NONE,      NULL }
};

void
load_db(const char *file, DBHeader *header, PackageDatabase *body)
{
	FILE *stream = fopen(file, "rb");
	if(!stream) {
		throw(ExBasic("Can't open the database file %s for reading (mode = 'rb')\n",
					file));
	}

	header->read(stream);
	if(!header->isCurrent()) {
		fprintf(stderr, "%s uses an obsolete database format (%i, current is %i).\n", file, header->version, DBHeader::current);
		exit(1);
	}
	body->read(header, stream);
}


/* Diff everything from old-tree with the according package from new-tree.
 * They diff if
 * a) the package does not exist in the new tree :) or
 * b) the new package has a different best-match than the old. */
void
diff_and_remove(PackageDatabase *db1, PackageDatabase *db2, PortageSettings &psettings, void (*callback)(Package *p1, Package *p2))
{
	Keywords accepted_keywords = psettings.getAcceptKeywords();
	for(PackageDatabase::category_iterator cat = db1->begin();
		cat != db1->end();
		++cat)
	{
		PackageDatabase::package_iterator pkg1 = cat->second.begin();
		while(pkg1 != cat->second.end())
		{
			Package *pkg2 = db2->findPackage((*pkg1)->category, (*pkg1)->name);
			psettings.user_config->setMasks(*pkg1);
			psettings.user_config->setStability(*pkg1, accepted_keywords);
			if(pkg2 == NULL) {
				callback(*pkg1, NULL);
			}
			else {
				psettings.user_config->setMasks(pkg2);
				psettings.user_config->setStability(pkg2, accepted_keywords);
				Version *old_best = (*pkg1)->best();
				Version *new_best = pkg2->best();
				if( ( (old_best == NULL
								&& new_best != NULL)
							|| (old_best != NULL
								&& new_best == NULL) )
						|| (old_best != NULL
							&& new_best != NULL
							&& *old_best != *new_best))
				{
					callback(*pkg1, pkg2);
				}
				db2->deletePackage(pkg2->category, pkg2->name);
			}
			db1->deletePackage((*pkg1)->category, (*pkg1)->name);
		}
	}
}

void
print_diff_new_old(Package *np, Package *op)
{
	if(np == NULL) {
		op->installed_versions = varpkg_db.getInstalledString(op);
		format_string.print(op, format_delete);
	}
	else if(op == NULL) {
		np->installed_versions = varpkg_db.getInstalledString(np);
		format_string.print(np, format_new);
	}
	else {
		Package *p[2];
		p[0] = op;
		p[1] = np;
		op->installed_versions = np->installed_versions = varpkg_db.getInstalledString(np);
		format_string.print(p, print_diff_package_property, get_diff_package_property, format_changed);
	}
}

void
print_diff_old_new(Package *op, Package *np)
{
	print_diff_new_old(np, op);
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
	vector<Parameter>::iterator current_param = argreader.begin();

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
		ERROR(1, "Missing cache-file.\n");
	}
	old_file = current_param->arg;
	++current_param;
	if(current_param == argreader.end() || current_param->type != Parameter::ARGUMENT) {
		new_file = EIX_CACHEFILE;
	}
	else {
		new_file = current_param->arg;
	}

	string varname;
	try {
		varname = "DIFF_FORMAT_NEW";
		format_new     = format_string.parseFormat(eixrc["DIFF_FORMAT_NEW"].c_str());
		varname = "DIFF_FORMAT_DELETE";
		format_delete  = format_string.parseFormat(eixrc["DIFF_FORMAT_DELETE"].c_str());
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

	format_string.setupColors();

	DBHeader old_header;
	PackageDatabase old_tree;
	load_db(old_file.c_str(), &old_header, &old_tree);

	DBHeader new_header;
	PackageDatabase new_tree;
	load_db(new_file.c_str(), &new_header, &new_tree);


	INFO("Diffing databases (%i - %i packages)\n", old_tree.countPackages(), new_tree.countPackages());

	diff_and_remove(&old_tree, &new_tree, portagesettings, print_diff_old_new);
	diff_and_remove(&new_tree, &old_tree, portagesettings, print_diff_new_old);

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
