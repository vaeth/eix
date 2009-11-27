// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <cache/cachetable.h>
#include <config.h>
#include <database/header.h>
#include <database/io.h>
#include <eixTk/argsreader.h>
#include <eixTk/exceptions.h>
#include <eixTk/filenames.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/percentage.h>
#include <eixTk/stringutils.h>
#include <eixTk/sysutils.h>
#include <eixTk/unused.h>
#include <eixTk/utils.h>
#include <eixrc/eixrc.h>
#include <eixrc/global.h>
#include <main/main.h>
#include <portage/conf/portagesettings.h>
#include <portage/packagetree.h>

#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

inline static void
INFO(const string &s)
{ cout << s; }

static void update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool will_modify, const vector<string> &exclude_labels) throw(ExBasic);
static void print_help(int ret) ATTRIBUTE_NORETURN;

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
			if(likely(know_root))
				return am_root;
			know_root = true;
			am_root = (my_geteuid() == 0);
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
						cerr << eix::format(_(
							"%s must be writable by group portage."))
							% cachefile << endl;
						exit(1);
					}
					if(!am_i_root())
					{
						if(!user_in_group("portage")) {
							cerr << _(
								"You must be in the portage group to update the database.\n"
								"Set SKIP_PERMISSION_TESTS=true to skip this test (e.g. if you use NSS/LDAP).")
								<< endl;
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
						cerr << eix::format(_(
							"User 'root' is needed to initially generate the database.\n"
							"Alternatively, you can generate a dummy cachefile %r\n"
							"with write permissions for group portage.\n"
							"Set SKIP_PERMISSION_TESTS=true to skip this test."))
							% cachefile << endl;
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
					cerr << _("warning: cannot change ownership of cachefile") << endl;
				}
				if(fchmod(fileno(database), 00664)) {
					cerr << _("warning: cannot change permissions for cachefile") << endl;
				}
			}
		}
};
bool Permissions::know_root, Permissions::am_root;


static void
print_help(int ret)
{
	printf(_("Usage: %s [options]\n"
			"\n"
			" -h, --help              show a short help screen\n"
			" -V, --version           show version-string\n"
			"     --dump              show eixrc-variables\n"
			"     --dump-defaults     show default eixrc-variables\n"
			"     --print             print the expanded value of a variable\n"
			" -n, --nocolor           don't use \"colors\" (percentage) in output\n"
			" -F, --force-color       force \"color\" on things that are not a terminal\n"
			"\n"
			" -q, --quiet             produce no output\n"
			"\n"
			" -o  --output            output to another file than %s\n"
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
			"further information.\n"),
		program_name.c_str(), EIX_CACHEFILE);

	exit(ret);
}

enum cli_options {
	O_DUMP = 260,
	O_DUMP_DEFAULTS,
	O_PRINT_VAR
};

static bool
	quiet(false),
	show_help(false),
	show_version(false),
	dump_eixrc(false),
	dump_defaults(false);

static bool use_percentage;

static list<const char *> exclude_args, add_args;
static list<ArgPair> method_args;
static const char *outputname(NULL);
static const char *var_to_print(NULL);

/** Arguments and shortopts. */
static struct Option long_options[] = {

	 Option("quiet",          'q',     Option::BOOLEAN,   &quiet),
	 Option("dump",            O_DUMP, Option::BOOLEAN_T, &dump_eixrc),
	 Option("dump-defaults",O_DUMP_DEFAULTS, Option::BOOLEAN_T, &dump_defaults),
	 Option("print",        O_PRINT_VAR, Option::STRING,  &var_to_print),
	 Option("help",           'h',     Option::BOOLEAN_T, &show_help),
	 Option("version",        'V',     Option::BOOLEAN_T, &show_version),
	 Option("nocolor",        'n',     Option::BOOLEAN_F, &use_percentage),
	 Option("force-color",    'F',     Option::BOOLEAN_T, &use_percentage),

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
	for(vector<string>::const_iterator it(to_add.begin());
		unlikely(it != to_add.end()); ++it)
		add_list.push_back(Pathname(*it, must_resolve));
}

static void
add_override(vector<Override> &override_list, EixRc &eixrc, const char *s)
{
	vector<string> v;
	split_string(v, eixrc[s], true);
	if(unlikely(v.size() & 1)) {
		cerr << eix::format(_("%s must be a list of the form DIRECTORY METHOD\n")) % s << endl;
		exit(1);
	}
	for(vector<string>::iterator it(v.begin()); unlikely(it != v.end()); ++it) {
		Override o(Pathname(*it, true));
		o.method = *(++it);
		override_list.push_back(o);
	}
}

static void
add_virtuals(vector<Override> &override_list, vector<Pathname> &add, string cachefile, string eprefix_virtual)
{
	static const string a("eix*::");
	FILE *fp(fopen(cachefile.c_str(), "rb"));
	if(!fp) {
		INFO(eix::format(_(
			"KEEP_VIRTUALS is ignored: there is no previous %s\n"))
			% cachefile);
		return;
	}

	INFO(eix::format(_("Adding virtual overlays from %s ..\n")) % cachefile);
	DBHeader header;
	bool is_current(io::read_header(fp, header));
	fclose(fp);
	if(unlikely(!is_current)) {
		cerr << _("Warning: KEEP_VIRTUALS ignored because database format has changed");
		return;
	}
	for(Version::Overlay i(0); likely(i != header.countOverlays()); ++i) {
		string overlay(eprefix_virtual + header.getOverlay(i).path);
		if(!is_virtual(overlay.c_str()))
			continue;
		Pathname name(overlay, false);
		add.push_back(name);
		escape_string(overlay, ":");
		override_list.push_back(Override(name, a + overlay));
	}
}

int
run_eix_update(int argc, char *argv[])
{
	/* Setup eixrc. */
	EixRc &eixrc(get_eixrc(EIX_VARS_PREFIX));
	bool skip_permission_tests;
	string eix_cachefile(eixrc["EIX_CACHEFILE"]);
	string outputfile;

	use_percentage = (eixrc.getBool("FORCE_PERCENTAGE") || isatty(1));

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, long_options);

	/* Honour a wish for silence */
	if(unlikely(quiet)) {
		if(!freopen(DEV_NULL, "w", stdout)) {
			cerr << eix::format(_("cannot redirect to %r")) % DEV_NULL << endl;
			exit(1);
		}
	}

	/* We do not want any arguments except options */
	if(unlikely(argreader.begin() != argreader.end()))
		print_help(1);
	if(unlikely(show_help))
		print_help(0);
	if(unlikely(show_version))
		dump_version(0);

	if(unlikely(var_to_print != NULL)) {
		eixrc.print_var(var_to_print);
		exit(0);
	}
	if(unlikely(dump_eixrc || dump_defaults)) {
		eixrc.dumpDefaults(stdout, dump_defaults);
		exit(0);
	}

	/* set the outputfile */
	if(unlikely(outputname != NULL)) {
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

	INFO(_("Reading Portage settings ..\n"));
	PortageSettings portage_settings(eixrc, false, true);

	/* Build default (overlay/method/...) lists, using environment vars */
	vector<Override> override_list;
	add_override(override_list, eixrc, "CACHE_METHOD");
	vector<Pathname> excluded_list;
	add_pathnames(excluded_list, split_string(eixrc["EXCLUDE_OVERLAY"], true), true);
	vector<Pathname> add_list;
	add_pathnames(add_list, split_string(eixrc["ADD_OVERLAY"], true), true);

	if(unlikely(eixrc.getBool("KEEP_VIRTUALS")))
		add_virtuals(override_list, add_list, eix_cachefile, eixrc["EPREFIX_VIRTUAL"]);

	add_override(override_list, eixrc, "OVERRIDE_CACHE_METHOD");

	/* Modify default (overlay/method/...) lists, using command line args */
	for(list<const char*>::iterator it(exclude_args.begin());
		unlikely(it != exclude_args.end()); ++it)
		excluded_list.push_back(Pathname(*it, true));
	for(list<const char*>::iterator it(add_args.begin());
		unlikely(it != add_args.end()); ++it)
		add_list.push_back(Pathname(*it, true));
	for(list<ArgPair>::iterator it(method_args.begin());
		unlikely(it != method_args.end()); ++it)
		override_list.push_back(Override(Pathname(it->first, true), it->second));

	/* Normalize names: */

	vector<string> excluded_overlays;
	for(vector<Pathname>::iterator it(excluded_list.begin());
		unlikely(it != excluded_list.end()); ++it)
		excluded_overlays.push_back(it->resolve(portage_settings));
	excluded_list.clear();

	vector<string> add_overlays;
	for(vector<Pathname>::iterator it(add_list.begin());
		unlikely(it != add_list.end()); ++it) {
		string add_name(it->resolve(portage_settings));
		// Let exclude override added names
		if(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
			add_name.c_str(), true) == excluded_overlays.end())
			add_overlays.push_back(add_name);
	}
	add_list.clear();

	map<string, string> override;
	for(vector<Override>::iterator it(override_list.begin());
		unlikely(it != override_list.end()); ++it) {
		override[(it->name).resolve(portage_settings)] = it->method;
	}
	override_list.clear();

	/* Calculate new PORTDIR_OVERLAY for export */

	if(likely(eixrc.getBool("EXPORT_PORTDIR_OVERLAY"))) {
		bool modified(false);
		string &ref(portage_settings["PORTDIR_OVERLAY"]);
		vector<string> overlayvec;
		split_string(overlayvec, ref, true);
		for(vector<string>::const_iterator it(add_overlays.begin());
			unlikely(it != add_overlays.end()); ++it) {
			if(find_filenames(overlayvec.begin(), overlayvec.end(),
				it->c_str(), false, true) == overlayvec.end()) {
				overlayvec.push_back(*it);
				modified = true;
			}
		}
		for(vector<string>::iterator it(overlayvec.begin());
			unlikely(it != overlayvec.end()); ) {
			if(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
				it->c_str(), true) == excluded_overlays.end()) {
				++it;
			}
			else {
				it = overlayvec.erase(it);
				modified = true;
			}
		}
		if(unlikely(modified)) {
			ref.clear();
			join_to_string(ref, overlayvec);
#ifdef HAVE_SETENV
			setenv("PORTDIR_OVERLAY", ref.c_str(), 1);
#else
			portage_settings.export_portdir_overlay = true;
#endif
		}
	}

	/* Create CacheTable and fill with PORTDIR and PORTDIR_OVERLAY. */
	CacheTable table(eixrc["CACHE_METHOD_PARSE"]);
	{
		map<string, string> *override_ptr(override.size() ? &override : NULL);
		if(likely(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
				portage_settings["PORTDIR"].c_str(), true) == excluded_overlays.end())) {
			table.addCache(eixrc.prefix_cstr("EPREFIX_PORTAGE_CACHE"),
				NULL,
				portage_settings["PORTDIR"].c_str(),
				eixrc["PORTDIR_CACHE_METHOD"],
				override_ptr);
		}
		else {
			INFO(eix::format(_("Excluded PORTDIR: %s\n"))
				% portage_settings["PORTDIR"]);
		}

		portage_settings.add_overlay_vector(add_overlays, false);

		for(vector<string>::size_type i(0); likely(i < portage_settings.overlays.size()); ++i) {
			if(likely(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
					portage_settings.overlays[i].c_str(), true, false) == excluded_overlays.end())) {
				table.addCache(eixrc.prefix_cstr("EPREFIX_PORTAGE_CACHE"),
					eixrc.prefix_cstr("EPREFIX_ACCESS_OVERLAYS"),
					portage_settings.overlays[i].c_str(),
					eixrc["OVERLAY_CACHE_METHOD"], override_ptr);
			}
			else {
				INFO(eix::format(_("Excluded overlay %s\n"))
					% portage_settings.overlays[i]);
			}
		}
	}

	INFO(eix::format(_("Building database (%s) ..\n")) % outputfile);

	/* Update the database from scratch */
	try {
		update(outputfile.c_str(), table, portage_settings,
			permissions.will_modify(), excluded_overlays);
	} catch(const ExBasic &e) {
		cerr << e << endl;
		return 2;
	}
	return 0;
}

static void
error_callback(const string &str)
{
	reading_percent_status->interprint_start();
	cerr << str << endl;
	reading_percent_status->interprint_end();
}

static void
update(const char *outputfile, CacheTable &cache_table, PortageSettings &portage_settings, bool will_modify, const vector<string> &exclude_labels) throw(ExBasic)
{
	DBHeader dbheader;
	vector<string> categories;
	portage_settings.pushback_categories(categories);
	PackageTree package_tree(categories);

	dbheader.world_sets = *(portage_settings.get_world_sets());

	for(eix::ptr_list<BasicCache>::iterator it(cache_table.begin());
		likely(it != cache_table.end()); ++it) {
		BasicCache *cache(*it);
		cache->portagesettings = &portage_settings;

		/* Build database from scratch. */
		OverlayIdent overlay(cache->getPath().c_str(), "");
		overlay.readLabel(cache->getPrefixedPath().c_str());
		if(unlikely(find(exclude_labels.begin(), exclude_labels.end(), overlay.label) != exclude_labels.end())) {
			INFO(eix::format(_("Excluding \"%s\" %s (cache: %s)\n"))
				% overlay.label
				% cache->getPathHumanReadable()
				% cache->getType());
			continue;
		}
		Version::Overlay key(dbheader.addOverlay(overlay));
		cache->setKey(key);
		cache->setOverlayName(overlay.label);
		//cache->setArch(portage_settings["ARCH"]);
		cache->setErrorCallback(error_callback);

		INFO(eix::format(_("[%s] \"%s\" %s (cache: %s)\n"))
			% key
			% overlay.label
			% cache->getPathHumanReadable()
			% cache->getType());
		reading_percent_status = new PercentStatus;
		if(cache->can_read_multiple_categories()) {
			reading_percent_status->init(_("     Reading Packages .. "));
			cache->setErrorCallback(error_callback);
			reading_percent_status->finish(
				likely(cache->readCategories(&package_tree)) ?
				_("Finished") : _("ABORTED!"));
		}
		else {
			if(use_percentage) {
				reading_percent_status->init(
					_("     Reading category %s|%s (%s%%)"),
					package_tree.size());
			}
			else {
				reading_percent_status->init(eix::format(
					_("     Reading up to %s categories of packages .. ")) %
					package_tree.size());
			}

			/* iterator through categories */
			bool aborted(false);
			bool is_empty(true);
			for(PackageTree::const_iterator ci(package_tree.begin());
				unlikely(ci != package_tree.end()); ++ci) {
				if(!cache->readCategoryPrepare(ci->first.c_str())) {
					if(use_percentage) {
						reading_percent_status->next();
					}
				}
				else {
					if(use_percentage) {
						reading_percent_status->next(eix::format(_(": %s ..")) % ci->first);
					}
					is_empty = false;
					if(!cache->readCategory(*(ci->second))) {
						aborted = true;
					}
				}
				cache->readCategoryFinalize();
			}
			string msg(unlikely(is_empty) ? _("EMPTY!") :
				(unlikely(aborted) ? _("ABORTED!") :
					_("Finished")));
			if(use_percentage)
				msg.insert(0, 1, ' ');
			reading_percent_status->finish(msg);
		}
		delete reading_percent_status;
	}

	/* Now apply all masks .. */
	INFO(_("Applying masks ..\n"));
	for(PackageTree::iterator c(package_tree.begin());
		likely(c != package_tree.end()); ++c) {
		Category *ci = c->second;
		for(Category::iterator p(ci->begin());
			likely(p != ci->end()); ++p) {
			portage_settings.setMasks(*p);
			p->save_maskflags(Version::SAVEMASK_FILE);
		}
	}

	INFO(_("Calculating hash tables ..\n"));
	io::prep_header_hashs(dbheader, package_tree);

	/* And write database back to disk .. */
	INFO(eix::format(_("Writing database file %s ..\n")) % outputfile);
	FILE *database_stream(fopen(outputfile, "wb"));
	if(unlikely(database_stream == NULL)) {
		throw ExBasic(_("Can't open the database file %r for writing (mode = 'wb')"))
			% outputfile;
	}
	if(unlikely(will_modify))
		Permissions::set_db(database_stream);

	dbheader.size = package_tree.countCategories();

	io::write_header(database_stream, dbheader);
	io::write_packagetree(database_stream, package_tree, dbheader);

	fclose(database_stream);

	INFO(eix::format(_("Database contains %s packages in %s categories.\n"))
		% package_tree.countPackages() % dbheader.size);
}

