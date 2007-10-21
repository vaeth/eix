// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

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

#include <portage/conf/cascadingprofile.h>
#include <portage/conf/portagesettings.h>

#include <string>

#include <dirent.h>
#include <sys/stat.h> /* chmod(..) */
#include <signal.h>   /* signal handlers */

#define INFO(...) printf(__VA_ARGS__)

using namespace std;

char *program_name = NULL;
void sig_handler(int sig);
void update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool will_modify, const vector<string> &exclude_labels);

class Permissions {
	private:
		string cachefile;
		bool modify;
		bool testing;
		static bool know_root, am_root;
	public:
		Permissions(const string &eix_cachefile, bool never_test)
		{
			cachefile = eix_cachefile;
			if(never_test)
				testing = false;
			else
				testing = (cachefile == EIX_CACHEFILE);
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
							cerr << "You must be in the portage group to update the database.\n" <<
								"If you use NSS/LDAP, set SKIP_PERMISSION_TESTS to skip this test.\n";
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

		bool will_modify()
		{ return modify; }

		static void set_db(FILE *database)
		{
			gid_t g;
			uid_t u;
			if(get_gid_of("portage", &g) && get_uid_of("portage", &u)) {
				fchown(fileno(database), u, g);
				fchmod(fileno(database), 00664);
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
			"     --print             print the expanded value of a variable\n"
			"\n"
			" -q, --quiet             produce no output\n"
			"\n"
			" -o  --output            output to another file than "EIX_CACHEFILE"\n"
			"                         In addition, all permission checks are omitted.\n"
			" -x  --exclude-overlay   exclude matching overlays from the update-process.\n"
			"                         Note that you can also exclude PORTDIR\n"
			"                         using this option.\n"
			"\n"
			" -a  --add-overlay       add an overlay to the update-process.\n"
			"\n"
			" -m  --override-method   override cache method for matching overlays.\n"
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
	O_DUMP_DEFAULTS,
	O_PRINT_VAR
};

bool quiet = false,
	 show_help = false,
	 show_version = false,
	 dump_eixrc   = false,
	 dump_defaults = false;

list<const char *> exclude_args, add_args;
list<ArgPair> method_args;
const char *outputname = NULL;
const char *print_var = NULL;

/** Arguments and shortopts. */
static struct Option long_options[] = {

	 Option("quiet",          'q',     Option::BOOLEAN,   &quiet),
	 Option("dump",            O_DUMP, Option::BOOLEAN_T, &dump_eixrc),
	 Option("dump-defaults",O_DUMP_DEFAULTS, Option::BOOLEAN_T, &dump_defaults),
	 Option("print",        O_PRINT_VAR, Option::STRING,  &print_var),
	 Option("help",           'h',     Option::BOOLEAN_T, &show_help),
	 Option("version",        'V',     Option::BOOLEAN_T, &show_version),

	 Option("exclude-overlay",'x',     Option::STRINGLIST,&exclude_args),
	 Option("add-overlay",    'a',     Option::STRINGLIST,&add_args),
	 Option("method",         'm',     Option::PAIRLIST,  &method_args),
	 Option("output",         'o',     Option::STRING,    &outputname),

	 Option(0 ,                0)
};

class Pathname {
	private:
		string name;
		bool must_resolve;
	public:
		string resolve(PortageSettings &portage_settings)
		{ return portage_settings.resolve_overlay_name(name, must_resolve); }

		Pathname(string n, bool r) : name(n), must_resolve(r)
		{ }
};

class Override {
	public:
		Pathname name;
		string method;

		Override(Pathname n) : name(n)
		{ }

		Override(Pathname n, string m) : name(n), method(m)
		{ }
};

void add_pathnames(vector<Pathname> &add_list, const vector<string> to_add, bool must_resolve)
{
	for(vector<string>::const_iterator it = to_add.begin();
		it != to_add.end(); ++it)
		add_list.push_back(Pathname(*it, must_resolve));
}

void add_override(vector<Override> &override_list, EixRc &eixrc, const char *s)
{
	vector<string> v = split_string(eixrc[s]," \t\n\r");
	if(v.size() & 1)
	{
		fprintf(stderr, "%s must be a list of the form DIRECTORY METHOD\n", s);
		exit(1);
	}
	for(vector<string>::iterator it = v.begin(); it != v.end(); ++it)
	{
		Override o(Pathname(*it, true));
		o.method = *(++it);
		override_list.push_back(o);
	}
}

void add_virtuals(vector<Override> &override_list, vector<Pathname> &add, string cachefile, string eprefix_virtual)
{
	static const string a("eix*::");
	FILE *fp = fopen(cachefile.c_str(), "rb");
	if(!fp)
		return;

	DBHeader header;
	io::read_header(fp, header);
	fclose(fp);
	if(!header.isCurrent())
		return;
	for(Version::Overlay i = 0; i != header.countOverlays(); i++)
	{
		string overlay = eprefix_virtual + header.getOverlay(i).path;
		if(!is_virtual(overlay.c_str()))
			continue;
		Pathname name(overlay, false);
		add.push_back(name);
		override_list.push_back(Override(name, a + overlay));
	}
}

int
run_update_eix(int argc, char *argv[])
{
	/* Setup eixrc. */
	EixRc &eixrc = get_eixrc(EIX_VARS_PREFIX);
	bool skip_permission_tests;
	string eix_cachefile = eixrc["EIX_CACHEFILE"];
	string outputfile;

	vector<Override> override_list;
	add_override(override_list, eixrc, "CACHE_METHOD");
	add_override(override_list, eixrc, "ADD_CACHE_METHOD");
	vector<Pathname> excluded_list;
	add_pathnames(excluded_list, split_string(eixrc["EXCLUDE_OVERLAY"], " \t\n\r"), true);
	vector<Pathname> add_list;
	add_pathnames(add_list, split_string(eixrc["ADD_OVERLAY"], " \t\n\r"), true);

	if(eixrc.getBool("KEEP_VIRTUALS"))
		add_virtuals(override_list, add_list, eix_cachefile, eixrc["EPREFIX_VIRTUAL"]);

	add_override(override_list, eixrc, "OVERRIDE_CACHE_METHOD");
	add_override(override_list, eixrc, "ADD_OVERRIDE_CACHE_METHOD");

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);

	/* We do not want any arguments except options */
	if(argreader.begin() != argreader.end())
		print_help(1);

	if(print_var) {
		PrintVar(print_var,eixrc);
		exit(0);
	}

	/* set the outputfile */
	if(outputname) {
		skip_permission_tests = true;
		outputfile = outputname;
	}
	else {
		skip_permission_tests = eixrc.getBool("SKIP_PERMISSION_TESTS");
		outputfile = eix_cachefile;
	}
	for(list<const char*>::iterator it = exclude_args.begin();
		it != exclude_args.end(); ++it)
		excluded_list.push_back(Pathname(*it, true));
	for(list<const char*>::iterator it = add_args.begin();
		it != add_args.end(); ++it)
		add_list.push_back(Pathname(*it, true));
	for(list<ArgPair>::iterator it = method_args.begin();
		it != method_args.end(); ++it)
		override_list.push_back(Override(Pathname(it->first, true), it->second));

	Permissions permissions(outputfile, skip_permission_tests);

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
	PortageSettings portage_settings(eixrc, false);

	/* Normalize names: */

	vector<string> add_overlays;
	for(vector<Pathname>::iterator it = add_list.begin();
		it != add_list.end(); ++it)
		add_overlays.push_back(it->resolve(portage_settings));
	add_list.clear();

	vector<string> excluded_overlays;
	for(vector<Pathname>::iterator it = excluded_list.begin();
		it != excluded_list.end(); ++it)
		excluded_overlays.push_back(it->resolve(portage_settings));
	excluded_list.clear();

	map<string, string> override;
	for(vector<Override>::iterator it = override_list.begin();
		it != override_list.end(); ++it) {
		override[(it->name).resolve(portage_settings)] = it->method;
	}
	override_list.clear();

	/* Create CacheTable and fill with PORTDIR and PORTDIR_OVERLAY. */
	CacheTable table;
	{
		map<string, string> *override_ptr = (override.size() ? &override : NULL);
		if(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
				portage_settings["PORTDIR"].c_str(), true) == excluded_overlays.end())
			table.addCache(eixrc.prefix_cstr("EPREFIX_PORTAGE_CACHE"),
				NULL,
				eixrc.prefix_cstr("EPREFIX_PORTAGE_EXEC"),
				portage_settings["PORTDIR"].c_str(),
				eixrc["PORTDIR_CACHE_METHOD"],
				override_ptr);
		else
			INFO("Excluded %s\n", portage_settings["PORTDIR"].c_str());

		portage_settings.add_overlay_vector(add_overlays, false);

		for(vector<string>::size_type i = 0; i < portage_settings.overlays.size(); ++i)
		{
			if(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
					portage_settings.overlays[i].c_str(), true, false) == excluded_overlays.end())
				table.addCache(eixrc.prefix_cstr("EPREFIX_PORTAGE_CACHE"),
					eixrc.prefix_cstr("EPREFIX_ACCESS_OVERLAYS"),
					eixrc.prefix_cstr("EPREFIX_PORTAGE_EXEC"),
					portage_settings.overlays[i].c_str(),
					eixrc["OVERLAY_CACHE_METHOD"], override_ptr);
			else
				INFO("Excluded %s\n", portage_settings.overlays[i].c_str());
		}
	}

	INFO("Building database (%s) ..\n", outputfile.c_str());

	/* Update the database from scratch */
	int ret = 0;
	try {
		update(outputfile.c_str(), table, portage_settings,
			permissions.will_modify(), excluded_overlays);
	} catch(const ExBasic &e)
	{
		cerr << e << endl;
		ret = 2;
	}
	return ret;
}

void
error_callback(const char *fmt, ...)
{
	fputs("\n", stdout);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
	INFO("     Reading    %%");
}

void
update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool will_modify, const vector<string> &exclude_labels)
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
		OverlayIdent overlay(cache->getPath().c_str(), "");
		overlay.readLabel(cache->getPrefixedPath().c_str());
		if(find(exclude_labels.begin(), exclude_labels.end(), overlay.label) != exclude_labels.end()) {
			INFO("Excluding \"%s\" %s (cache: %s)\n", overlay.label.c_str(), cache->getPathHumanReadable().c_str(), cache->getType());
			continue;
		}
		short key = dbheader.addOverlay(overlay);
		cache->setKey(key);
		//cache->setArch(portage_settings["ARCH"]);
		cache->setErrorCallback(error_callback);

		INFO("[%u] \"%s\" %s (cache: %s)\n", uint(key), overlay.label.c_str(), cache->getPathHumanReadable().c_str(), cache->getType());
		INFO("     Reading ");
		if(cache->can_read_multiple_categories())
		{
			PercentStatus percent_status(1);
			cache->setErrorCallback(error_callback);
			if(cache->readCategories(&package_tree, categories)) {
				++percent_status;
			}
			else {
				INFO("\b\b\b\baborted\n");
			}
		}
		else
		{
			PercentStatus percent_status(categories->size());

			/* iterator through categories */
			for(vector<string>::iterator ci = categories->begin();
				ci != categories->end();
				++ci)
			{
				if(cache->readCategory(package_tree[*ci])) {
					++percent_status;
				}
				else {
					INFO("\b\b\b\baborted\n");
					break;
				}
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
			portage_settings.setMasks(*p);
			p->save_maskflags(Version::SAVEMASK_FILE);
		}
	}

	/* And write database back to disk .. */
	FILE *database_stream = fopen(outputfile, "wb");
	ASSERT(database_stream, "Can't open the database file %s for writing (mode = 'wb')", outputfile);
	if(will_modify)
		Permissions::set_db(database_stream);

	dbheader.size = package_tree.countCategories();

	io::write_header(database_stream, dbheader);
	io::write_packagetree(database_stream, package_tree);

	fclose(database_stream);

	INFO("Database contains %u packages in %u categories.\n",
		uint(package_tree.countPackages()), uint(dbheader.size));
}

/** On segfault: show some instructions to help us find the bug. */
void
sig_handler(int sig)
{
	if(sig == SIGSEGV)
		fprintf(stderr,
				"Received SIGSEGV - you probably found a bug in eix.\n"
				"Please proceed with the following few instructions and help us find the bug:\n"
				" * install gdb (sys-dev/gdb)\n"
				" * compile eix with FEATURES=\"nostrip\" CXXFLAGS=\"-g -ggdb3\"\n"
				" * enter gdb with \"gdb --args %s your_arguments_for_%s\"\n"
				" * type \"run\" and wait for the segfault to happen\n"
				" * type \"bt\" to get a backtrace (this helps us a lot)\n"
				" * post a bugreport and be sure to include the output from gdb ..\n"
				"\n"
				"Sorry for the inconvenience and thanks in advance!\n",
				program_name, program_name);
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
	catch(const ExBasic &e) {
		cout << e << endl;
		return 1;
	}
	return ret;
}
