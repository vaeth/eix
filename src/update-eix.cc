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

#include "../config.h"

#include <global.h>
#include <eixTk/argsreader.h>
#include <eixTk/stringutils.h>
#include <eixTk/exceptions.h>
#include <eixTk/sysutils.h>
#include <eixTk/utils.h>

#include <database/header.h>
#include <database/database.h>

#include <cachetable.h>

#include <portage/conf/portagesettings.h>

#include <string>

#include <dirent.h>
#include <sys/stat.h> /* chmod(..) */
#include <signal.h>   /* signal handlers */

#define INFO(...) printf(__VA_ARGS__)

char *program_name = NULL;
void sig_handler(int sig);
void update(CacheTable &cache_table, PortageSettings &portage_settings);

static void
check_db_permissions()
{
	/* Check if the permissions are correct and we are in the right group */
	if(getuid() != 0) {
		if(get_mtime(EIX_CACHEFILE) == 0) {
			fprintf(stderr, "User 'root' is needed to initially generate the database.\n");
			exit(1);
		}
		if(!user_in_group("portage")) {
			fprintf(stderr, "You must be in the portage-group to update the database.\n");
			exit(1);
		}
		if(!is_writable(EIX_CACHEFILE)) {
			fprintf(stderr, EIX_CACHEFILE" must be writable by group portage.\n");
			exit(1);
		}
	}

	gid_t g;
	uid_t u;

	if(get_gid_of("portage", &g) && get_uid_of("portage", &u)) {
		chown(EIX_CACHEFILE, u, g);
		chmod(EIX_CACHEFILE, 00664);
	}
}

static void
print_help(void)
{
	printf( "%s [options]\n"
			"\n"
			" -h, --help              show a short help screen\n"
			" -V, --version           show version-string\n"
			"     --dump              show eixrc-variables\n"
			"\n"
			" -q, --quiet             produce no output\n"
			"\n"
			"     --exclude-overlay   exclude a overlay from the update-process.\n"
			"\n"
#if 0
			"     --print-masks       print masks (if you are no developer, you probably won't need this)\n" 
			"\n"
#endif
			"You can contact the developers in #eix on irc.freenode.net or on\n"
			"the sourceforge-page "PACKAGE_BUGREPORT".\n"
			"There is also a wiki at "EIX_WIKI".\n"
			"This program is covered by the GNU General Public License. See COPYING for\n"
			"further information.\n",
		program_name);

	exit(0);
}

#define O_DUMP          260
#define EXCLUDE_OVERLAY 266
#define ONLY_NEEDED     267
#define O_PRINT_MASKS   268

bool quiet = false,
	 show_help = false,
	 show_version = false,
	 dump_eixrc = false,
	 print_masks_only = false;

/** Arguments and shortopts. */
static struct Option long_options[] = {

	{"quiet",          'q',              Option::BOOLEAN,   (void *) &quiet }, /* produce no output */
	{"dump",           O_DUMP,           Option::BOOLEAN_T, (void *) &dump_eixrc },
	{"help",           'h',              Option::BOOLEAN_T, (void *) &show_help }, /* show a short help screen */
	{"version",        'V',              Option::BOOLEAN_T, (void *) &show_version},
	{"exclude-overlay", EXCLUDE_OVERLAY, Option::NONE,       NULL }, /* exclude a overlay from the update-process. */
	{"print-masks",     O_PRINT_MASKS,   Option::BOOLEAN_T, (void *) &print_masks_only },
	{ 0 ,              0 ,               Option::NONE,       NULL }
};

int
run_update_eix(int argc, char *argv[])
{
	/* Setup eixrc. */
	EixRc &eixrc = get_eixrc();

	vector<string>excluded_overlays = split_string(eixrc["EXCLUDE_OVERLAY"], " \t\n\r");

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);
	vector<Parameter>::iterator current_param = argreader.begin();

	/* Read options. */
	while(current_param != argreader.end())
	{
		switch(current_param->opt)
		{
			case EXCLUDE_OVERLAY:
				++current_param;
				if(! (current_param != argreader.end()
					  && current_param->type == Parameter::ARGUMENT)) {
					fprintf(stderr, "Arg %i: --exclude-overlay is missing an argument.\n",
							distance(argreader.begin(), current_param));
					exit(1);
				}
				excluded_overlays.push_back(current_param->arg);
				break;
		}
		++current_param;
	}

	/* Check for correct permissions. */
	check_db_permissions();

	if(show_help)
		print_help();

	if(show_version) {
		dump_version(0);
	}

	/* Honor a wish for silence */
	if(quiet) {
		close(1);
		close(2);
	}

	if(dump_eixrc) {
		eixrc.dumpDefaults(stdout);
		exit(0);
	}

	INFO("Reading Portage settings ..\n");
	PortageSettings portage_settings;

	if(print_masks_only) {
		cout << *(portage_settings.profile->getAllowedPackages());
		cout << *(portage_settings.profile->getSystemPackages());
		cout << *(portage_settings.getMasks());
		exit(0);
	}

	/* Create CacheTable and fill with PORTDIR and PORTDIR_OVERLAY. */
	CacheTable table;
	if( find(excluded_overlays.begin(), excluded_overlays.end(),
				portage_settings["PORTDIR"]) == excluded_overlays.end())
		table.addCache(portage_settings["PORTDIR"], eixrc["PORTDIR_CACHE_METHOD"].c_str());
	else
		INFO("Not reading %s\n", portage_settings["PORTDIR"].c_str());

	for(unsigned int i = 0; i<portage_settings.overlays.size(); ++i)
	{
		if( find(excluded_overlays.begin(), excluded_overlays.end(),
					portage_settings.overlays[i]) == excluded_overlays.end())
			table.addCache(portage_settings.overlays[i], eixrc["OVERLAY_CACHE_METHOD"].c_str());
		else
			INFO("Not reading %s\n", portage_settings.overlays[i].c_str());
	}

	INFO("Building database (%s) from scratch ..\n", EIX_CACHEFILE);

	/* Update the database from scratch */
	try {
		update(table, portage_settings);
	} catch(ExBasic &e)
	{
		cerr << e << endl;
		return 1;
	}
	return 0;
}

void
error_callback(const char *fmt, ...)
{
	fputs("\n", stdout);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fputs("\n", stdout);
	INFO("     Reading    %%");
}

void
update(CacheTable &cache_table, PortageSettings &portage_settings)
{
	DBHeader dbheader;
	PackageDatabase package_tree(portage_settings.getCategories());

	dbheader.arch = portage_settings["ARCH"];

	for(vector<BasicCache*>::iterator it = cache_table.begin();
			it != cache_table.end(); ++it)
	{
		BasicCache *cache = *it;
		/* Build database from scratch. */
		short key = dbheader.addOverlay(cache->getScheme());
		cache->setKey(key);
		cache->setArch(portage_settings["ARCH"]);
		cache->setErrorCallback(error_callback);

		INFO("[%i] %s (cache: %s)\n", key, cache->getScheme().c_str(), cache->getType());
		INFO("     Reading ");

		PercentStatus percent_status;
		percent_status.start(package_tree.countCategories());

		/* iterator through categories */
		for(PackageDatabase::mapiter m = package_tree.begin(); m != package_tree.end(); ++m) {
			++percent_status;
			cache->readCategory(m->second, m->first);
		}
	}

	/* Now apply all masks .. */
	INFO("Applying masks ..\n");
	package_tree.checkMasks(portage_settings.profile->getAllowedPackages());
	package_tree.checkMasks(portage_settings.profile->getSystemPackages());
	package_tree.checkMasks(portage_settings.getMasks());

	/* And write database back to disk .. */
	FILE *database_stream = fopen(EIX_CACHEFILE, "wb");
	ASSERT(database_stream, "Can't open the database file %s for writing (mode = 'wb')", EIX_CACHEFILE);

	dbheader.numpackages   = package_tree.countPackages();
	dbheader.numcategories = package_tree.countCategories();
	dbheader.write(database_stream);
	package_tree.write(database_stream);
	fclose(database_stream);

	INFO("Database contains %i packages in %i categories.\n", dbheader.numpackages, dbheader.numcategories);
}

/** On segfault: show some instructions to help us find the bug. */
void
sig_handler(int sig)
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
	program_name = argv[0];

	/* Install signal handler for segfaults */
	signal(SIGSEGV, sig_handler);

	int ret = 0;
	try {
		ret = run_update_eix(argc, argv);
	}
	catch(ExBasic e) {
		cout << e << endl;
		return 1;
	}
	return ret;
}
