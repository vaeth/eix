// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include <main/main.h>

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

#define INFO printf

using namespace std;

static void update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool will_modify, const vector<string> &exclude_labels);

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
				if(fchown(fileno(database), u, g)) {
					cerr << "warning: cannot change ownership of cachefile" << endl;
				}
				if(fchmod(fileno(database), 00664)) {
					cerr << "warning: cannot change permissions for cachefile" << endl;
				}
			}
		}
};
bool Permissions::know_root, Permissions::am_root;

static void
print_help(int ret)
{
	printf( "Usage: %s [options]\n"
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
			"This program is covered by the GNU General Public License. See COPYING for\n"
			"further information.\n",
		program_name.c_str());

	exit(ret);
}

enum cli_options {
	O_DUMP = 260,
	O_DUMP_DEFAULTS,
	O_PRINT_VAR
};

static bool quiet = false,
	 show_help = false,
	 show_version = false,
	 dump_eixrc   = false,
	 dump_defaults = false;

static list<const char *> exclude_args, add_args;
static list<ArgPair> method_args;
static const char *outputname = NULL;
static const char *var_to_print = NULL;

/** Arguments and shortopts. */
static struct Option long_options[] = {

	 Option("quiet",          'q',     Option::BOOLEAN,   &quiet),
	 Option("dump",            O_DUMP, Option::BOOLEAN_T, &dump_eixrc),
	 Option("dump-defaults",O_DUMP_DEFAULTS, Option::BOOLEAN_T, &dump_defaults),
	 Option("print",        O_PRINT_VAR, Option::STRING,  &var_to_print),
	 Option("help",           'h',     Option::BOOLEAN_T, &show_help),
	 Option("version",        'V',     Option::BOOLEAN_T, &show_version),

	 Option("exclude-overlay",'x',     Option::STRINGLIST,&exclude_args),
	 Option("add-overlay",    'a',     Option::STRINGLIST,&add_args),
	 Option("method",         'm',     Option::PAIRLIST,  &method_args),
	 Option("output",         'o',     Option::STRING,    &outputname),

	 Option(0 ,                0)
};

static PercentStatus *reading_percent_status;

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

static void
add_pathnames(vector<Pathname> &add_list, const vector<string> to_add, bool must_resolve)
{
	for(vector<string>::const_iterator it = to_add.begin();
		it != to_add.end(); ++it)
		add_list.push_back(Pathname(*it, must_resolve));
}

static void
add_override(vector<Override> &override_list, EixRc &eixrc, const char *s)
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

static void
add_virtuals(vector<Override> &override_list, vector<Pathname> &add, string cachefile, string eprefix_virtual)
{
	static const string a("eix*::");
	FILE *fp = fopen(cachefile.c_str(), "rb");
	if(!fp) {
		INFO("KEEP_VIRTUALS is ignored: there is no previous %s\n", cachefile.c_str());
		return;
	}

	INFO("Adding virtual overlays from %s ..\n", cachefile.c_str());
	DBHeader header;
	bool is_current = io::read_header(fp, header);
	fclose(fp);
	if(!is_current) {
		fprintf(stderr, "Warning: KEEP_VIRTUALS ignored because database format has changed\n");
		return;
	}
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

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);

	/* Honour a wish for silence */
	if(quiet) {
		close(1);
		close(2);
	}

	/* We do not want any arguments except options */
	if(argreader.begin() != argreader.end())
		print_help(1);
	if(show_help)
		print_help(0);
	if(show_version)
		dump_version(0);

	if(var_to_print) {
		eixrc.print_var(var_to_print);
		exit(0);
	}
	if(dump_eixrc || dump_defaults) {
		eixrc.dumpDefaults(stdout, dump_defaults);
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

	/* Check for correct permissions. */
	Permissions permissions(outputfile, skip_permission_tests);
	permissions.check_db();

	INFO("Reading Portage settings ..\n");
	PortageSettings portage_settings(eixrc, false, true);

	/* Build default (overlay/method/...) lists, using environment vars */
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

	/* Modify default (overlay/method/...) lists, using command line args */
	for(list<const char*>::iterator it = exclude_args.begin();
		it != exclude_args.end(); ++it)
		excluded_list.push_back(Pathname(*it, true));
	for(list<const char*>::iterator it = add_args.begin();
		it != add_args.end(); ++it)
		add_list.push_back(Pathname(*it, true));
	for(list<ArgPair>::iterator it = method_args.begin();
		it != method_args.end(); ++it)
		override_list.push_back(Override(Pathname(it->first, true), it->second));

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

static void
error_callback(const string &str)
{
	reading_percent_status->interprint_start();
	cerr << str << endl;
	reading_percent_status->interprint_end();
}

static void
update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool will_modify, const vector<string> &exclude_labels)
{
	DBHeader dbheader;
	vector<string> *categories = portage_settings.getCategories();
	PackageTree package_tree;

	dbheader.world_sets = *(portage_settings.get_world_sets());

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
		Version::Overlay key = dbheader.addOverlay(overlay);
		cache->setKey(key);
		cache->setOverlayName(overlay.label);
		//cache->setArch(portage_settings["ARCH"]);
		cache->setErrorCallback(error_callback);

		INFO("[%u] \"%s\" %s (cache: %s)\n", PercentU(key), overlay.label.c_str(), cache->getPathHumanReadable().c_str(), cache->getType());
		reading_percent_status = new PercentStatus("     Reading ");
		if(cache->can_read_multiple_categories())
		{
			reading_percent_status->start(1);
			cache->setErrorCallback(error_callback);
			if(cache->readCategories(&package_tree, categories))
				++(*reading_percent_status);
			else
				reading_percent_status->reprint("aborted\n");
		}
		else
		{
			reading_percent_status->start(categories->size());

			/* iterator through categories */
			for(vector<string>::iterator ci = categories->begin();
				ci != categories->end();
				++ci)
			{
				// ignore return value of bad categories
				cache->readCategory(package_tree[*ci]);
				++(*reading_percent_status);
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

	INFO("Calculating hash tables ..\n");
	io::prep_header_hashs(dbheader, package_tree);

	/* And write database back to disk .. */
	INFO("Writing database file %s ..\n", outputfile);
	FILE *database_stream = fopen(outputfile, "wb");
	if (!database_stream) {
		throw ExBasic("Can't open the database file %r for writing (mode = 'wb')")
				% outputfile;
	}
	if(will_modify)
		Permissions::set_db(database_stream);

	dbheader.size = package_tree.countCategories();

	io::write_header(database_stream, dbheader);
	io::write_packagetree(database_stream, package_tree, dbheader);

	fclose(database_stream);

	INFO("Database contains %u packages in %u categories.\n",
		PercentU(package_tree.countPackages()), PercentU(dbheader.size));
}

