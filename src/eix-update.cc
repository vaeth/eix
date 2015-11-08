// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <fnmatch.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>

#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "cache/cachetable.h"
#include "database/header.h"
#include "database/io.h"
#include "eixTk/argsreader.h"
#include "eixTk/filenames.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/percentage.h"
#include "eixTk/statusline.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "eixTk/utils.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "main/main.h"
#include "portage/conf/portagesettings.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/overlay.h"
#include "portage/packagetree.h"
#include "various/drop_permissions.h"

using std::list;
using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

inline static void INFO(const string& s) {
	cout << s;
}

class Pathname {
	private:
		string name;
		bool must_resolve;
	public:
		bool is_a_match(const string& s) const {
			if(must_resolve) {
				return !fnmatch(name.c_str(), s.c_str(), 0);
			}
			return s == name;
		}

		string resolve(PortageSettings *portage_settings) ATTRIBUTE_NONNULL_ {
			return portage_settings->resolve_overlay_name(name, must_resolve);
		}

		Pathname(string n, bool r) : name(n), must_resolve(r) {
		}
};

class Override {
	public:
		Pathname name;
		string method;

		explicit Override(Pathname n) : name(n) {
		}

		Override(Pathname n, string m) : name(n), method(m) {
		}
};

class RepoName {
	public:
		Pathname name;
		string repo_name;

		explicit RepoName(Pathname n) : name(n) {
		}

		RepoName(Pathname n, string r) : name(n), repo_name(r) {
		}
};

typedef vector<Pathname> PathVec;
typedef vector<Override> Overrides;
typedef vector<RepoName> RepoNames;

static void print_help();
static bool update(const char *outputfile, CacheTable *cache_table, PortageSettings *portage_settings, bool override_umask, const RepoNames& repo_names, const WordVec& exclude_labels, Statusline *statusline, string *errtext) ATTRIBUTE_NONNULL_;
static void error_callback(const string& str);
static void add_pathnames(PathVec *add_list, const WordVec& to_add, bool must_resolve) ATTRIBUTE_NONNULL_;
static void add_override(Overrides *override_list, EixRc *eixrc, const char *s) ATTRIBUTE_NONNULL_;
static void add_reponames(RepoNames *repo_names, EixRc *eixrc, const char *s) ATTRIBUTE_NONNULL_;
static void add_virtuals(Overrides *override_list, PathVec *add, RepoNames *repo_names, const string& cachefile, const string& eprefix_virtual) ATTRIBUTE_NONNULL_;
static void override_label(OverlayIdent *overlay, const RepoNames& repo_names) ATTRIBUTE_NONNULL_;
static bool stringstart_in_wordlist(const string& to_check, const WordVec& wordlist);

static void print_help() {
	cout << eix::format(_("Usage: %s [options]\n"
"\n"
" -h, --help              show a short help screen\n"
" -V, --version           show version-string\n"
"     --dump              show eixrc-variables\n"
"     --dump-defaults     show default eixrc-variables\n"
"     --print             print the expanded value of a variable\n"
"     --known-vars        print all variable names known to --print\n"
" -H, --nostatus          don't update status line\n"
" -n, --nocolor           don't use \"colors\" (percentage) in output\n"
"     --force-status      always output status line\n"
" -F, --force-color       force \"color\" even if output is no terminal\n"
" -v, --verbose           output used cache method for each ebuild\n"
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
" -r  --repo-name         set label for matching overlay.\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.\n")) % program_name % EIX_CACHEFILE;
}

enum cli_options {
	O_DUMP = 260,
	O_DUMP_DEFAULTS,
	O_KNOWN_VARS,
	O_PRINT_VAR,
	O_FORCE_STATUS
};

static bool
	quiet(false),
	show_help(false),
	show_version(false),
	known_vars(false),
	dump_eixrc(false),
	dump_defaults(false);

static bool use_percentage, use_status, verbose;

typedef list<const char *> ExcludeArgs;
typedef ExcludeArgs AddArgs;
static ExcludeArgs *exclude_args;
static AddArgs *add_args;

typedef list <ArgPair> MethodArgs;
typedef MethodArgs RepoArgs;
static MethodArgs *method_args;
static RepoArgs *repo_args;

static const char *outputname = NULLPTR;
static const char *var_to_print = NULLPTR;

/**
Arguments and options
**/
class EixUpdateOptionList : public OptionList {
	public:
		EixUpdateOptionList();
};

EixUpdateOptionList::EixUpdateOptionList() {
	push_back(Option("quiet",          'q',     Option::BOOLEAN,    &quiet));
	push_back(Option("dump",            O_DUMP, Option::BOOLEAN_T,  &dump_eixrc));
	push_back(Option("dump-defaults", O_DUMP_DEFAULTS, Option::BOOLEAN_T, &dump_defaults));
	push_back(Option("print",        O_PRINT_VAR,  Option::STRING,    &var_to_print));
	push_back(Option("known-vars",   O_KNOWN_VARS, Option::BOOLEAN_T, &known_vars));
	push_back(Option("help",           'h',     Option::BOOLEAN_T,  &show_help));
	push_back(Option("version",        'V',     Option::BOOLEAN_T,  &show_version));
	push_back(Option("nostatus",       'H',     Option::BOOLEAN_F,  &use_status));
	push_back(Option("nocolor",        'n',     Option::BOOLEAN_F,  &use_percentage));
	push_back(Option("force-color",    'F',     Option::BOOLEAN_T,  &use_percentage));
	push_back(Option("force-status", O_FORCE_STATUS, Option::BOOLEAN_T, &use_status));
	push_back(Option("verbose",        'v',     Option::BOOLEAN_T,  &verbose));

	push_back(Option("exclude-overlay", 'x',    Option::STRINGLIST, exclude_args));
	push_back(Option("add-overlay",    'a',     Option::STRINGLIST, add_args));
	push_back(Option("override-method", 'm',    Option::PAIRLIST,   method_args));
	push_back(Option("repo-name",      'r',     Option::PAIRLIST,   repo_args));
	push_back(Option("output",         'o',     Option::STRING,     &outputname));
}

static PercentStatus *reading_percent_status;


static void add_pathnames(PathVec *add_list, const WordVec& to_add, bool must_resolve) {
	for(WordVec::const_iterator it(to_add.begin());
		unlikely(it != to_add.end()); ++it)
		add_list->push_back(Pathname(*it, must_resolve));
}

static void add_override(Overrides *override_list, EixRc *eixrc, const char *s) {
	WordVec v;
	split_string(&v, (*eixrc)[s], true);
	if(unlikely(v.size() & 1)) {
		cerr << eix::format(_("%s must be a list of the form DIRECTORY METHOD")) % s << endl;
		exit(EXIT_FAILURE);
	}
	for(WordVec::iterator it(v.begin()); unlikely(it != v.end()); ++it) {
		Override o(Pathname(*it, true));
		o.method = *(++it);
		override_list->push_back(o);
	}
}

static void add_reponames(RepoNames *repo_names, EixRc *eixrc, const char *s) {
	WordVec v;
	split_string(&v, (*eixrc)[s], true);
	if(unlikely(v.size() & 1)) {
		cerr << eix::format(_("%s must be a list of the form DIR-PATTERN OVERLAY-LABEL")) % s << endl;
		exit(EXIT_FAILURE);
	}
	for(WordVec::const_iterator it(v.begin()); unlikely(it != v.end()); ++it) {
		RepoName r(Pathname(*it, true));
		r.repo_name = *(++it);
		repo_names->push_back(r);
	}
}

static void add_virtuals(Overrides *override_list, PathVec *add, RepoNames *repo_names, const string& cachefile, const string& eprefix_virtual) {
	Database db;
	if(unlikely(!db.openread(cachefile.c_str()))) {
		INFO(eix::format(_(
			"KEEP_VIRTUALS is ignored: there is no previous %s\n"))
			% cachefile);
		return;
	}

	INFO(eix::format(_("Adding virtual overlays from %s...\n")) % cachefile);
	DBHeader header;
	bool is_current(db.read_header(&header, NULLPTR));
	if(unlikely(!is_current)) {
		cerr << _("warning: KEEP_VIRTUALS ignored because database format has changed");
		return;
	}
	for(ExtendedVersion::Overlay i(0); likely(i != header.countOverlays()); ++i) {
		const OverlayIdent& ov(header.getOverlay(i));
		string overlay(eprefix_virtual + ov.path);
		if(!is_virtual(overlay.c_str()))
			continue;
		Pathname name(overlay, false);
		add->push_back(name);
		escape_string(&overlay, ":");
		override_list->push_back(Override(name, string("eix*::") + overlay));
		repo_names->push_back(RepoName(name, ov.label));
	}
}

static void override_label(OverlayIdent *overlay, const RepoNames& repo_names) {
	for(RepoNames::const_iterator it(repo_names.begin());
		it != repo_names.end(); ++it) {
		if(it->name.is_a_match(overlay->path)) {
			overlay->setLabel(it->repo_name);
		}
	}
}

static bool stringstart_in_wordlist(const string& to_check, const WordVec& wordlist) {
	for(WordVec::const_iterator it(wordlist.begin()); it != wordlist.end(); ++it) {
		if(to_check.compare(0, it->size(), *it) == 0) {
			return true;
		}
	}
	return false;
}

int run_eix_update(int argc, char *argv[]) {
	// Initialize static classes
	ExtendedVersion::init_static();
	PortageSettings::init_static();
	exclude_args = new ExcludeArgs;
	add_args = new AddArgs;
	method_args = new MethodArgs;
	repo_args = new RepoArgs;

	/* Setup eixrc. */
	EixRc& eixrc(get_eixrc(UPDATE_VARS_PREFIX)); {
		string errtext;
		bool success(drop_permissions(&eixrc, &errtext));
		if(!errtext.empty()) {
			cerr << errtext << endl;
		}
		if(!success) {
			return EXIT_FAILURE;
		}
	}
	Depend::use_depend = eixrc.getBool("DEP");
	string eix_cachefile(eixrc["EIX_CACHEFILE"]); {
	/* calculate defaults for use_{percentage,status} */
		bool percentage_tty(false);
		if(eixrc.getBool("NOPERCENTAGE")) {
			use_percentage = false;
		} else if(eixrc.getBool("FORCE_PERCENTAGE")) {
			use_percentage = true;
		} else {
			percentage_tty = true;
		}
		bool status_tty(false);
		if(eixrc.getBool("NOSTATUSLINE")) {
			use_status = false;
		} else if(eixrc.getBool("FORCE_STATUSLINE")) {
			use_status = true;
		} else {
			status_tty = true;
		}
		if(percentage_tty || status_tty) {
			bool is_tty(isatty(1) != 0);
			if(percentage_tty) {
				use_percentage = is_tty;
			}
			if(status_tty) {
				use_status = (is_tty &&
					stringstart_in_wordlist(eixrc["TERM"],
						split_string(eixrc["TERM_STATUSLINE"])));
			}
		}
	}

	/* other defaults */
	verbose = eixrc.getBool("UPDATE_VERBOSE");

	/* Setup ArgumentReader. */
	ArgumentReader argreader(argc, argv, EixUpdateOptionList());

	/* We do not want any arguments except options */
	if(unlikely(argreader.begin() != argreader.end())) {
		print_help();
		return EXIT_FAILURE;
	}

	if(unlikely(var_to_print != NULLPTR)) {
		if(eixrc.print_var(var_to_print)) {
			return EXIT_SUCCESS;
		}
		return EXIT_FAILURE;
	}
	if(unlikely(known_vars)) {
		eixrc.known_vars();
		return EXIT_SUCCESS;
	}
	if(unlikely(show_help)) {
		print_help();
		return EXIT_SUCCESS;
	}
	if(unlikely(show_version)) {
		dump_version();
	}

	if(unlikely(dump_eixrc || dump_defaults)) {
		eixrc.dumpDefaults(stdout, dump_defaults);
		return EXIT_SUCCESS;
	}

	/* Honour a wish for silence */
	if(unlikely(quiet)) {
		if(!freopen(DEV_NULL, "w", stdout)) {
			cerr << eix::format(_("cannot redirect to \"%s\"")) % DEV_NULL << endl;
			return EXIT_FAILURE;
		}
	}

	/* set the outputfile */
	string outputfile(eix_cachefile);
	bool override_umask(true);
	if(unlikely(outputname != NULLPTR)) {
		outputfile = outputname;
		override_umask = false;
	}

	Statusline statusline(use_status, (use_status &&
		stringstart_in_wordlist(eixrc["TERM"],
			split_string(eixrc["TERM_SOFTSTATUSLINE"]))),
		program_name, eixrc["EXIT_STATUSLINE"]);

	INFO(_("Reading Portage settings...\n"));
	PortageSettings portage_settings(&eixrc, false, true);

	/* Build default (overlay/method/...) lists, using environment vars */
	Overrides override_list;
	add_override(&override_list, &eixrc, "CACHE_METHOD");
	PathVec excluded_list;
	add_pathnames(&excluded_list, split_string(eixrc["EXCLUDE_OVERLAY"], true), true);
	PathVec add_list;
	add_pathnames(&add_list, split_string(eixrc["ADD_OVERLAY"], true), true);

	RepoNames repo_names;

	if(unlikely(eixrc.getBool("KEEP_VIRTUALS"))) {
		add_virtuals(&override_list, &add_list, &repo_names, eix_cachefile, eixrc["EPREFIX_VIRTUAL"]);
	}

	add_override(&override_list, &eixrc, "OVERRIDE_CACHE_METHOD");

	/* Modify default (overlay/method/...) lists, using command line args */
	for(ExcludeArgs::iterator it(exclude_args->begin());
		unlikely(it != exclude_args->end()); ++it)
		excluded_list.push_back(Pathname(*it, true));
	for(AddArgs::iterator it(add_args->begin());
		unlikely(it != add_args->end()); ++it)
		add_list.push_back(Pathname(*it, true));
	for(MethodArgs::iterator it(method_args->begin());
		unlikely(it != method_args->end()); ++it)
		override_list.push_back(Override(Pathname(it->first, true), it->second));

	/* For REPO_NAMES it is quite the opposite: */

	for(RepoArgs::iterator it(repo_args->begin());
		unlikely(it != repo_args->end()); ++it)
		repo_names.push_back(RepoName(Pathname(it->first, false), it->second));

	add_reponames(&repo_names, &eixrc, "REPO_NAMES");

	/* Normalize names: */

	WordVec excluded_overlays;
	for(PathVec::iterator it(excluded_list.begin());
		unlikely(it != excluded_list.end()); ++it)
		excluded_overlays.push_back(it->resolve(&portage_settings));
	excluded_list.clear();

	WordVec add_overlays;
	for(PathVec::iterator it(add_list.begin());
		unlikely(it != add_list.end()); ++it) {
		string add_name(it->resolve(&portage_settings));
		// Let exclude override added names
		if(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
			add_name.c_str(), true) == excluded_overlays.end())
			add_overlays.push_back(add_name);
	}
	add_list.clear();

	WordMap override;
	for(Overrides::iterator it(override_list.begin());
		unlikely(it != override_list.end()); ++it) {
		override[(it->name).resolve(&portage_settings)] = it->method;
	}
	override_list.clear();

	/* Calculate new PORTDIR_OVERLAY for export */

	if(likely(eixrc.getBool("EXPORT_PORTDIR_OVERLAY"))) {
		bool modified(false);
		string& ref(portage_settings["PORTDIR_OVERLAY"]);
		WordVec overlayvec;
		split_string(&overlayvec, ref, true);
		for(WordVec::const_iterator it(add_overlays.begin());
			unlikely(it != add_overlays.end()); ++it) {
			if(find_filenames(overlayvec.begin(), overlayvec.end(),
				it->c_str(), false, true) == overlayvec.end()) {
				overlayvec.push_back(*it);
				modified = true;
			}
		}
		for(WordVec::iterator it(overlayvec.begin()); unlikely(it != overlayvec.end()); ) {
			if(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
				it->c_str(), true) == excluded_overlays.end()) {
				++it;
			} else {
				it = overlayvec.erase(it);
				modified = true;
			}
		}
		if(unlikely(modified)) {
			ref.clear();
			join_to_string(&ref, overlayvec);
#ifdef HAVE_SETENV
			setenv("PORTDIR_OVERLAY", ref.c_str(), 1);
#else
			portage_settings.export_portdir_overlay = true;
#endif
		}
	}

	/* Create CacheTable and fill with PORTDIR and PORTDIR_OVERLAY. */
	CacheTable table(eixrc["CACHE_METHOD_PARSE"]); {
		WordMap *override_ptr(override.size() ? &override : NULLPTR);
		if(likely(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
				portage_settings["PORTDIR"].c_str(), true) == excluded_overlays.end())) {
			string errtext;
			if(unlikely(!table.addCache(eixrc.prefix_cstr("EPREFIX_PORTAGE_CACHE"),
				NULLPTR,
				portage_settings["PORTDIR"].c_str(),
				eixrc["PORTDIR_CACHE_METHOD"],
				override_ptr,
				&errtext))) {
				cerr << errtext << endl;
			}
		} else {
			INFO(eix::format(_("Excluded PORTDIR: %s\n"))
				% portage_settings["PORTDIR"]);
		}

		portage_settings.add_repo_vector(add_overlays, false);

		RepoList repos(portage_settings.repos);
		for(RepoList::const_iterator it(repos.second()); likely(it != repos.end()); ++it) {
			const char *path(it->path.c_str());
			if(likely(find_filenames(excluded_overlays.begin(), excluded_overlays.end(),
					path, true, false) == excluded_overlays.end())) {
				string errtext;
				if(unlikely(!table.addCache(eixrc.prefix_cstr("EPREFIX_PORTAGE_CACHE"),
					eixrc.prefix_cstr("EPREFIX_ACCESS_OVERLAYS"),
					path,
					eixrc["OVERLAY_CACHE_METHOD"], override_ptr, &errtext))) {
					cerr << errtext << endl;
				}
			} else {
				INFO(eix::format(_("Excluded overlay %s\n"))
					% it->human_readable());
			}
		}
	}

	INFO(eix::format(_("Building database (%s)...\n")) % outputfile);

	/* Update the database from scratch */
	string errtext;
	if(unlikely(!update(outputfile.c_str(), &table, &portage_settings, override_umask,
			repo_names, excluded_overlays, &statusline, &errtext))) {
		cerr << errtext << endl;
		statusline.failure();
		return EXIT_FAILURE;
	}
	statusline.success();
	return EXIT_SUCCESS;
}

static void error_callback(const string& str) {
	reading_percent_status->interprint_start();
	cerr << str << endl;
	reading_percent_status->interprint_end();
}

static bool update(const char *outputfile, CacheTable *cache_table, PortageSettings *portage_settings, bool override_umask, const RepoNames& repo_names, const WordVec& exclude_labels, Statusline *statusline, string *errtext) {
	DBHeader dbheader;
	WordVec categories;
	portage_settings->pushback_categories(&categories);
	PackageTree package_tree(categories);

	dbheader.world_sets = *(portage_settings->get_world_sets());

	/* We must first initialize all caches and erase unneeded ones,
	   because some cache methods like eixcache know about each other
	   and call each other before we can call them in a loop afterwards. */
	for(CacheTable::iterator it(cache_table->begin());
		likely(it != cache_table->end()); ) {
		BasicCache *cache(*it);
		cache->portagesettings = portage_settings;

		/* Build database from scratch. */
		OverlayIdent overlay(cache->getPath().c_str());
		override_label(&overlay, repo_names);
		overlay.readLabel(cache->getPrefixedPath().c_str());
		if(unlikely(find(exclude_labels.begin(), exclude_labels.end(), overlay.label) != exclude_labels.end())) {
			INFO(eix::format(_("Excluding \"%s\" %s (cache: %s)\n"))
				% overlay.label
				% cache->getPathHumanReadable()
				% cache->getType());
			it = cache_table->erase(it);
			continue;
		}
		ExtendedVersion::Overlay key(dbheader.addOverlay(overlay));
		cache->setKey(key);
		cache->setOverlayName(overlay.label);
		// cache->setArch((*portage_settings)["ARCH"]);
		cache->setErrorCallback(error_callback);
		if(verbose) {
			cache->setVerbose();
		}
		++it;
	}

	/* Build database from scratch. */
	for(CacheTable::iterator it(cache_table->begin());
		likely(it != cache_table->end()); ++it) {
		BasicCache *cache(*it);
		INFO(eix::format(_("[%s] \"%s\" %s (cache: %s)\n"))
			% cache->getKey()
			% cache->getOverlayName()
			% cache->getPathHumanReadable()
			% cache->getType());
		statusline->print(eix::format(P_("Statusline eix-update", "[%s] %s"))
				% cache->getKey()
				% cache->getOverlayName());
		reading_percent_status = new PercentStatus;
		if(cache->can_read_multiple_categories()) {
			reading_percent_status->init(P_("Percent",
				"     Reading Packages..."));
			cache->setErrorCallback(error_callback);
			reading_percent_status->finish(
				likely(cache->readCategories(&package_tree)) ?
				P_("Percent", "Finished") :
				P_("Percent", "ABORTED!"));
		} else {
			if(use_percentage) {
				reading_percent_status->init(P_("Percent",
					"     Reading category %s|%s (%s%%)"),
					package_tree.size());
			} else {
				reading_percent_status->init(eix::format(NP_("Percent",
					"     Reading %s category of packages...",
					"     Reading up to %s categories of packages...",
					package_tree.size()))
					% package_tree.size());
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
				} else {
					if(use_percentage) {
						reading_percent_status->next(eix::format(P_("Percent", ": %s...")) % ci->first);
					}
					is_empty = false;
					if(!cache->readCategory(ci->second)) {
						aborted = true;
					}
				}
				cache->readCategoryFinalize();
			}
			string msg(unlikely(is_empty) ? P_("Percent", "EMPTY!") :
				(unlikely(aborted) ? P_("Percent", "ABORTED!") :
					P_("Percent", "Finished")));
			if(use_percentage) {
				msg.insert(string::size_type(0), 1, ' ');
			}
			reading_percent_status->finish(msg);
		}
		delete reading_percent_status;
	}
	statusline->print(P_("Statusline eix-update", "Analyzing"));

	/* Now apply all masks... */
	INFO(_("Applying masks...\n"));
	for(PackageTree::iterator c(package_tree.begin());
		likely(c != package_tree.end()); ++c) {
		Category *ci = c->second;
		for(Category::iterator p(ci->begin());
			likely(p != ci->end()); ++p) {
			portage_settings->setMasks(*p);
			p->save_maskflags(Version::SAVEMASK_FILE);
		}
	}

	INFO(_("Calculating hash tables...\n"));
	Database::prep_header_hashs(&dbheader, package_tree);

	/* And write database back to disk... */
	statusline->print(eix::format(P_("Statusline eix-update", "Creating %s")) % outputfile);
	INFO(eix::format(_("Writing database file %s...\n")) % outputfile);
	mode_t old_umask;
	if(override_umask) {
		old_umask = umask(2);
	}
	Database db;
	bool ok(db.openwrite(outputfile));
	if(override_umask) {
		umask(old_umask);
	}
	if(unlikely(!ok)) {
		*errtext = eix::format(_("cannot open database file %s for writing (mode = 'wb')")) % outputfile;
		return false;
	}

	dbheader.size = package_tree.countCategories();

	if(!(likely(db.write_header(dbheader, errtext)) &&
		likely(db.write_packagetree(package_tree, dbheader, errtext)))) {
		return false;
	}

	INFO(eix::format(N_(
		"Database contains %s packages in %s category.\n",
		"Database contains %s packages in %s categories.\n",
		dbheader.size))
		% package_tree.countPackages() % dbheader.size);
	return true;
}
