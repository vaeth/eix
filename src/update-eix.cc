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

#include "../config.h"

#include <global.h>
#include <eixTk/argsreader.h>
#include <eixTk/stringutils.h>
#include <eixTk/exceptions.h>
#include <eixTk/sysutils.h>
#include <eixTk/utils.h>
#include <eixTk/filenames.h>

#include <database/header.h>
#include <database/io.h>
#include <portage/packagetree.h>

#include <cachetable.h>

#include <portage/conf/portagesettings.h>

#include <string>

#include <dirent.h>
#include <sys/stat.h> /* chmod(..) */
#include <signal.h>   /* signal handlers */

#define INFO(...) printf(__VA_ARGS__)

using namespace std;

char *program_name = NULL;
void sig_handler(int sig);
void update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool small);

class Permissions {
	private:
		string cachefile;
		bool modify;
		bool testing;
		static bool know_root, am_root;
	public:
		Permissions(const char *other_cachefile)
		{
			if(other_cachefile)
			{
				testing = false;
				cachefile = other_cachefile;
			}
			else
			{
				testing = true;
				cachefile = EIX_CACHEFILE;
			}
			know_root = false;
		}

		bool am_i_root()
		{
			if(know_root)
				return am_root;
			know_root = true;
			am_root = (getuid() == 0);
			return am_root;
		}

		bool test_writable()
		{
			if(am_i_root())
				return true;
			else
				return is_writable(cachefile.c_str());
		}

		void check_db()
		{
			bool must_modify;
			if(get_mtime(cachefile.c_str()) == 0)
			{
				modify = true;
				must_modify = testing;
			}
			else
			{
				modify = testing;
				must_modify = false;
				if(testing)
				{
					if(!test_writable())
					{
						cerr << cachefile << " must be writable by group portage.\n";
						exit(1);
					}
					if(!am_i_root())
					{
						if(!user_in_group("portage")) {
							cerr << "You must be in the portage-group to update the database.\n";
							exit(1);
						}
					}
				}
			}
			if(modify)
			{
				if(!am_i_root())
				{
					if(must_modify) {
						cerr << "User 'root' is needed to initially generate the database.\n";
						exit(1);
					}
					modify = false;
				}
				return;
			}
		}

		void set_db()
		{
			if(!modify)
				return;
			gid_t g;
			uid_t u;
			if(get_gid_of("portage", &g) && get_uid_of("portage", &u)) {
				chown(cachefile.c_str(), u, g);
				chmod(cachefile.c_str(), 00664);
			}
		}
};
bool Permissions::know_root, Permissions::am_root;

static void
print_help(int ret)
{
	printf( "%s [options]\n"
			"\n"
			" -h, --help              show a short help screen\n"
			" -V, --version           show version-string\n"
			"     --dump              show eixrc-variables\n"
			"     --dump-defaults     show default eixrc-variables\n"
			"\n"
			" -q, --quiet             produce no output\n"
			"\n"
			" -o  --output            output to another file than"EIX_CACHEFILE"\n"
			"                         In addition, all permission checks are omitted.\n"
			" -x  --exclude-overlay   exclude an overlay from the update-process.\n"
			"                         Note that you can also exclude PORTDIR\n"
			"                         using this option.\n"
			"\n"
			" -a  --add-overlay       add an overlay to the update-process.\n"
			"\n"
			" -m  --override-method   override cache method for a specified overlay.\n"
			"\n"
			"You can contact the developers in #eix on irc.freenode.net or on\n"
			"the sourceforge-page "PACKAGE_BUGREPORT".\n"
			"There is also a wiki at "EIX_WIKI".\n"
			"This program is covered by the GNU General Public License. See COPYING for\n"
			"further information.\n",
		program_name);

	exit(ret);
}

enum cli_options {
	O_DUMP = 260,
	O_DUMP_DEFAULTS
};

bool quiet = false,
	 show_help = false,
	 show_version = false,
	 dump_eixrc   = false,
	 dump_defaults = false;

list<const char *> exclude_args, add_args;
list<ArgPair> method_args;
const char *outputname = NULL;

/** Arguments and shortopts. */
static struct Option long_options[] = {

	 Option("quiet",          'q',     Option::BOOLEAN,   &quiet),
	 Option("dump",            O_DUMP, Option::BOOLEAN_T, &dump_eixrc),
	 Option("dump-defaults",O_DUMP_DEFAULTS, Option::BOOLEAN_T, &dump_defaults),
	 Option("help",           'h',     Option::BOOLEAN_T, &show_help),
	 Option("version",        'V',     Option::BOOLEAN_T, &show_version),

	 Option("exclude-overlay",'x',     Option::STRINGLIST,&exclude_args),
	 Option("add-overlay",    'a',     Option::STRINGLIST,&add_args),
	 Option("method",         'm',     Option::PAIRLIST,  &method_args),
	 Option("output",         'o',     Option::STRING,    &outputname),

	 Option(0 ,                0)
};

inline
void add_override(map<string, string> &override, EixRc &eixrc, const char *s)
{
	vector<string> v = split_string(eixrc[s]," \t\n\r");
	if(v.size() & 1)
	{
		fprintf(stderr, "%s must be a list of the form DIRECTORY METHOD\n", s);
		exit(1);
	}
	for(vector<string>::iterator it = v.begin(); it != v.end(); ++it)
	{
		const char *path = it->c_str();
		++it;
		override[path] = *it;
	}
}

void add_virtuals(map<string, string> &override, vector<string> &add)
{
	static const string a = "eix*::";
	FILE *fp = fopen(EIX_CACHEFILE, "rb");
	if(!fp)
		return;

	DBHeader header;
	io::read_header(fp, header);
	fclose(fp);
	if(!header.isCurrent())
		return;
	for(Version::Overlay i = 0; i != header.countOverlays(); i++)
	{
		const char *overlay = (header.getOverlay(i)).c_str();
		if(!is_virtual(overlay))
			continue;
		add.push_back(overlay);
		override[overlay] = a + overlay;
	}
}

int
run_update_eix(int argc, char *argv[])
{
	/* Setup eixrc. */
	EixRc &eixrc = get_eixrc();
	map<string, string> override;
	bool have_output;
	string outputfile;

	add_override(override, eixrc, "CACHE_METHOD");
	add_override(override, eixrc, "ADD_CACHE_METHOD");
	vector<string> excluded_overlays = split_string(eixrc["EXCLUDE_OVERLAY"], " \t\n\r");
	vector<string> add_overlays = split_string(eixrc["ADD_OVERLAY"], " \t\n\r");

	if(eixrc.getBool("KEEP_VIRTUALS"))
		add_virtuals(override, add_overlays);

	add_override(override, eixrc, "OVERRIDE_CACHE_METHOD");
	add_override(override, eixrc, "ADD_OVERRIDE_CACHE_METHOD");

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);

	/* We do not want any arguments except options */
	if(argreader.begin() != argreader.end())
		print_help(1);

	/* Interpret the string arguments (do not trust that content of const char * remains) */
	if(outputname)
	{
		have_output = true;
		outputfile = outputname;
	}
	else
	{
		have_output = false;
		outputfile = EIX_CACHEFILE;
	}
	for(list<const char*>::iterator it = exclude_args.begin();
		it != exclude_args.end(); ++it)
		excluded_overlays.push_back(*it);
	for(list<const char*>::iterator it = add_args.begin();
		it != add_args.end(); ++it)
		add_overlays.push_back(*it);
	for(list<ArgPair>::iterator it = method_args.begin();
		it != method_args.end(); ++it)
		override[it->first] = it->second;

	Permissions permissions(have_output ? outputfile.c_str() : NULL);

	if(show_help)
		print_help(0);

	if(show_version) {
		dump_version(0);
	}

	/* Honor a wish for silence */
	if(quiet) {
		close(1);
		close(2);
	}

	if(dump_eixrc || dump_defaults) {
		eixrc.dumpDefaults(stdout, dump_defaults);
		exit(0);
	}

	/* Check for correct permissions. */
	permissions.check_db();

	INFO("Reading Portage settings ..\n");
	PortageSettings portage_settings;

	/* Create CacheTable and fill with PORTDIR and PORTDIR_OVERLAY. */
	CacheTable table;
	{
		map<string, string> *override_ptr = (override.size() ? &override : NULL);
		if( find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
				portage_settings["PORTDIR"]) == excluded_overlays.end())
			table.addCache(portage_settings["PORTDIR"], eixrc["PORTDIR_CACHE_METHOD"].c_str(), override_ptr);
		else
			INFO("Not reading %s\n", portage_settings["PORTDIR"].c_str());

		for(vector<string>::iterator it = add_overlays.begin();
			it != add_overlays.end(); ++it)
		{
			// Don't add double overlays
			if(find_filenames(portage_settings.overlays.begin(), portage_settings.overlays.end(),
					*it) != portage_settings.overlays.end())
				continue;
			// Don't add PORTDIR
			if(same_filenames(portage_settings["PORTDIR"], *it))
				continue;
			portage_settings.overlays.push_back(*it);
		}

		for(unsigned int i = 0; i<portage_settings.overlays.size(); ++i)
		{
			if( find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
					portage_settings.overlays[i]) == excluded_overlays.end())
				table.addCache(portage_settings.overlays[i], eixrc["OVERLAY_CACHE_METHOD"].c_str(), override_ptr);
			else
				INFO("Not reading %s\n", portage_settings.overlays[i].c_str());
		}
	}

	INFO("Building database (%s) ..\n", outputfile.c_str());

	/* Update the database from scratch */
	int ret = 0;
	try {
		update(outputfile.c_str(), table, portage_settings,
			eixrc.getBool("SMALL_EIX_DATABASE"));
	} catch(ExBasic &e)
	{
		cerr << e << endl;
		ret = 2;
	}
	permissions.set_db();
	return ret;
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
update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool small)
{
	DBHeader dbheader;
	vector<string> *categories = portage_settings.getCategories();
	PackageTree package_tree;

	for(eix::ptr_list<BasicCache>::iterator it = cache_table.begin();
		it != cache_table.end();
		++it)
	{
		BasicCache *cache = *it;
		cache->portagesettings = &portage_settings;

		/* Build database from scratch. */
		short key = dbheader.addOverlay(cache->getPath());
		cache->setKey(key);
		cache->setArch(portage_settings["ARCH"]);
		cache->setErrorCallback(error_callback);

		INFO("[%i] %s (cache: %s)\n", key, cache->getPath().c_str(), cache->getType());
		INFO("     Reading ");
		if(cache->can_read_multiple_categories())
		{
			cache->readCategories(&package_tree, categories);
			INFO("100%\n");
		}
		else
		{

			PercentStatus percent_status;
			percent_status.start(categories->size());

			/* iterator through categories */
			for(vector<string>::iterator ci = categories->begin();
				ci != categories->end();
				++ci)
			{
				++percent_status;
				cache->readCategory(package_tree[*ci]);
			}
		}
	}

	/* Now apply all masks .. */
	INFO("Applying masks ..\n");
	for(PackageTree::iterator c = package_tree.begin();
		c != package_tree.end();
		++c)
	{
		for(Category::iterator p = c->begin();
			p != c->end();
			++p)
		{
			portage_settings.profile->getAllowedPackages()->applyMasks(*p);
			portage_settings.profile->getSystemPackages()->applyMasks(*p);
			portage_settings.getMasks()->applyMasks(*p);
		}
	}

	/* And write database back to disk .. */
	FILE *database_stream = fopen(outputfile, "wb");
	ASSERT(database_stream, "Can't open the database file %s for writing (mode = 'wb')", EIX_CACHEFILE);

	dbheader.size = package_tree.countCategories();

	io::write_header(database_stream, dbheader);
	io::write_packagetree(database_stream, package_tree, small);

	fclose(database_stream);

	INFO("Database contains %i packages in %i categories.\n",
		package_tree.countPackages(), dbheader.size);
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
