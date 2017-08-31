// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/conf/portagesettings.h"
#include <config.h>

#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <string>
#include <vector>

#include "eixTk/assert.h"
#include "eixTk/attribute.h"
#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/filenames.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/parseerror.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "eixTk/unordered_map.h"
#include "eixTk/utils.h"
#include "eixTk/varsreader.h"
#include "eixrc/eixrc.h"
#include "portage/basicversion.h"
#include "portage/conf/cascadingprofile.h"
#include "portage/keywords.h"
#include "portage/mask.h"
#include "portage/mask_list.h"
#include "portage/overlay.h"
#include "portage/package.h"
#include "portage/packagesets.h"
#include "portage/version.h"

using std::string;
using std::vector;

static string *emptystring = NULLPTR;

typedef char ArchUsed;

ATTRIBUTE_NONNULL_ static ArchUsed apply_keyword(const string& key, const WordUnorderedSet& keywords_set, KeywordsFlags kf, const WordSet *arch_set, Keywords::Redundant *redundant, Keywords::Redundant check, bool shortcut);
ATTRIBUTE_NONNULL_ inline static void increase(char *s);

const string& PortageSettings::operator[](const string& var) const {
	const_iterator it(find(var));
	if(it == end()) {
		return *emptystring;
	}
	return it->second;
}

const char *PortageSettings::cstr(const string& var) const {
	const_iterator it(find(var));
	if(it == end()) {
		return NULLPTR;
	}
	return it->second.c_str();
}

bool PortageSettings::grab_setmasks(const char *file, SetsIndex i, WordVec *contains_set, bool recursive) {
	LineVec lines;
	if(!pushback_lines(file, &lines, recursive)) {
		return false;
	}
	for(LineVec::iterator it(lines.begin()); likely(it < lines.end()); ++it) {
		if(it->empty()) {
			continue;
		}
		string errtext;
		SetMask m(i);
		BasicVersion::ParseResult r(m.parseMask(it->c_str(), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			parse_error->output(file, lines.begin(), it, errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			if(m.is_set()) {
				contains_set->PUSH_BACK(m.getName());
			} else {
				m_package_sets.add(m);
			}
		}
	}
	return true;
}


/**
Keys that should accumulate their content rathern then replace
**/
static CONSTEXPR const char *default_accumulating_keys[] = {
	"USE",
	"USE_EXPAND*",
	"CONFIG_*",
	"FEATURES",
	"ACCEPT_KEYWORDS",
	NULLPTR
};

/**
Environment variables which should take effect before reading profiles
**/
static CONSTEXPR const char *test_in_env_early[] = {
	"PORTAGE_PROFILE",
	"PORTAGE_REPOSITORIES",
	"PORTDIR",
	"PORTDIR_OVERLAY",
	NULLPTR
};

/**
Environment variables which should add/override all other settings
**/
static CONSTEXPR const char *test_in_env_late[] = {
	"USE",
	"USE_EXPAND",
	"USE_EXPAND_HIDDEN",
	"CONFIG_PROTECT",
	"CONFIG_PROTECT_MASK",
	"FEATURES",
	"ARCH",
	"ACCEPT_KEYWORDS",
	NULLPTR
};

void PortageSettings::override_by_env(const char *const *vars) {
	for(const char *var(*vars); likely(var != NULLPTR); var = *(++vars)) {
		const char *e(std::getenv(var));
		if(e == NULLPTR)
			continue;
		if(!match_list(default_accumulating_keys, var)) {
			(*this)[var] = e;
			continue;
		}
		string& ref((*this)[var]);
		if(ref.empty())
			ref = e;
		else
			ref.append(string("\n") + e);
	}
}

void PortageSettings::read_config(const string& name, const string& prefix) {
	(*this)["EPREFIX"] = m_eprefix;
	VarsReader configfile(VarsReader::SUBST_VARS|VarsReader::INTO_MAP|VarsReader::APPEND_VALUES|VarsReader::ALLOW_SOURCE|VarsReader::PORTAGE_ESCAPES|VarsReader::RECURSE);
	configfile.accumulatingKeys(default_accumulating_keys);
	configfile.useMap(this);
	configfile.setPrefix(prefix);
	string errtext;
	if(unlikely(!configfile.read(name.c_str(), &errtext, true))) {
		eix::say_error() % errtext;
	}
}

string PortageSettings::resolve_overlay_name(const string& path, bool resolve) {
	if(resolve) {
		string full(m_eprefixoverlays);
		full.append(path);
		return normalize_path(full.c_str(), true);
	}
	return normalize_path(path.c_str(), false);
}

void PortageSettings::add_repo(const string& path, bool resolve, const char *label, OverlayIdent::Priority priority, bool is_main) {
	OverlayIdent o(resolve_overlay_name(path, resolve).c_str(), label, priority, is_main);
	repos.push_back(o, true);
}

void PortageSettings::add_repo_vector(const WordVec& v, bool resolve) {
	for(WordVec::const_iterator it(v.begin()); likely(it != v.end()); ++it) {
		add_repo(*it, resolve);
	}
}

/**
Read make.globals and make.conf
**/
void PortageSettings::init(EixRc *eixrc, const ParseError *e, bool getlocal, bool init_world, bool print_profile_paths) {
	settings_rc = eixrc;
	parse_error = e;
#ifndef HAVE_SETENV
	export_portdir_overlay = false;
#endif
	know_upgrade_policy = know_expands = false;
	m_recurse_sets    = eixrc->getBool("RECURSIVE_SETS");
	m_eprefixconf     = eixrc->m_eprefixconf;
	m_eprefix         = (*eixrc)["EPREFIX"];
	m_eprefixprofile  = (*eixrc)["EPREFIX_PORTAGE_PROFILE"];
	m_eprefixportdir  = (*eixrc)["EPREFIX_PORTDIR"];
	m_eprefixoverlays = (*eixrc)["EPREFIX_OVERLAYS"];
	m_eprefixaccessoverlays = (*eixrc)["EPREFIX_ACCESS_OVERLAYS"];
	(*this)["ARCH"]   = (*eixrc)["DEFAULT_ARCH"];

	const string& eprefixsource((*eixrc)["EPREFIX_SOURCE"]);
	const string& make_globals((*eixrc)["MAKE_GLOBALS"]);
	if(is_file(make_globals.c_str())) {
		read_config(make_globals, eprefixsource);
	} else {
		read_config(m_eprefixconf + MAKE_GLOBALS_FILE, eprefixsource);
	}
	read_config(m_eprefixconf + MAKE_CONF_FILE, eprefixsource);
	read_config(m_eprefixconf + MAKE_CONF_FILE_NEW, eprefixsource);
	string *use_make_conf; {
		iterator it(find("USE"));
		if(likely(it != end())) {
			use_make_conf = new string(it->second);
			erase(it);
		} else {
			use_make_conf = NULLPTR;
		}
	}

	override_by_env(test_in_env_early); {
		/* Read repos.conf */
		bool have_repos;
		VarsReader reposfile(VarsReader::SUBST_VARS|VarsReader::PORTAGE_SECTIONS|VarsReader::RECURSE);
		reposfile.setPrefix(eprefixsource);
		string &repositories = (*this)["PORTAGE_REPOSITORIES"];
		if(!repositories.empty()) {
			const char *s = repositories.c_str();
			have_repos = reposfile.readmem(s, s + repositories.size(), NULLPTR);
		} else {
			have_repos = reposfile.read((*eixrc)["PORTAGE_REPOS_CONF"].c_str(), NULLPTR, true);
			if(reposfile.read((m_eprefixconf + USER_REPOS_CONF).c_str(), NULLPTR, true)) {
				have_repos = true;
			}
		}
		const string *main_repo(NULLPTR);
		OverlayIdent::Priority priority(-1000);
		string& portdirref((*this)["PORTDIR"]);
		if(likely(have_repos)) {
			if(portdirref.empty()) {
				main_repo = reposfile.find("main-repo");
				const string *p(NULLPTR);
				const string *prio(NULLPTR);
				if(main_repo == NULLPTR) {
					p = reposfile.find("location");
					prio = reposfile.find("priority");
				} else {
					p = reposfile.find(string("location:") + *main_repo);
					prio = reposfile.find(string("priority:") + *main_repo);
				}
				if(prio != NULLPTR) {
					priority = my_atos(prio->c_str());
				}
				if(p != NULLPTR) {
					portdirref = *p;
				}
			}
		}
		if(portdirref.empty()) {
			portdirref = "/usr/portage";
		}
		/* Normalize "PORTDIR": */
		portdirref = normalize_path((m_eprefixportdir + portdirref).c_str(), true, true);
		add_repo(portdirref, false, (main_repo == NULLPTR) ? NULLPTR : main_repo->c_str(), priority, true);
		/* Normalize overlays and erase duplicates */
		string& ref((*this)["PORTDIR_OVERLAY"]);
		WordVec overlayvec;
		split_string(&overlayvec, ref, true);
		add_repo_vector(overlayvec, true);
		overlayvec.clear();
		if(likely(have_repos)) {
			string main_repo_check;
			if(main_repo != NULLPTR) {
				main_repo_check.assign(*main_repo);
			}
			for(VarsReader::const_iterator it(reposfile.begin());
				likely(it != reposfile.end()); ++it) {
				if(std::strncmp(it->first.c_str(), "location:", 9)) {
					continue;
				}
				string label(it->first.substr(9));
				if(unlikely(label == main_repo_check)) {
					continue;
				}
				priority = 0;
				const string *prio(reposfile.find(string("priority:") + label));
				if(prio != NULLPTR) {
					priority = my_atos(prio->c_str());
				}
				add_repo(it->second, true, label.c_str(), priority, false);
			}
		}
		repos.sort();
		for(RepoList::const_iterator it(repos.second());
			likely(it != repos.end()); ++it) {
			overlayvec.PUSH_BACK(it->path);
		}
		ref.clear();
		join_to_string(&ref, overlayvec, "\n");
	}

	user_config = NULLPTR;
	profile = new CascadingProfile(this, init_world);
	if(unlikely(print_profile_paths)) {
		profile->print_profile_paths = true;
		string& s(profile->profile_paths_append);
		s = (*eixrc)["PRINT_APPEND"];
		unescape_string(&s);
	}
	store_world_sets(NULLPTR);
	bool read_world(false);
	if(init_world) {
		if(eixrc->getBool("SAVE_WORLD"))
			read_world = true;
	} else {
		if(eixrc->getBool("CURRENT_WORLD"))
			read_world = true;
	}
	if(read_world) {
		if(profile->m_world.add_file((*eixrc)["EIX_WORLD"].c_str(), Mask::maskInWorld, false, parse_error)) {
			profile->use_world = true;
		}
		profile->m_world.finalize();
		read_world_sets((*eixrc)["EIX_WORLD_SETS"].c_str());
	}

	string& my_path((*this)["PORTDIR"]);
	profile->listaddFile(my_path + PORTDIR_MASK_FILE, 0, false);
	profile->listaddFile(my_path + PORTDIR_UNMASK_FILE, 0, false);
	profile->listaddProfile();
	if(unlikely(print_profile_paths)) {
		return;
	}
	profile->readMakeDefaults();
	profile->readremoveFiles();
	CascadingProfile *local_profile(NULLPTR);
	if(getlocal) {
		local_profile = new CascadingProfile(*profile);
	}
	addOverlayProfiles(profile);
	if(getlocal) {
		local_profile->listaddProfile((m_eprefixconf + USER_PROFILE_DIR).c_str());
		addOverlayProfiles(local_profile);
		local_profile->readMakeDefaults();
		if(!local_profile->readremoveFiles()) {
			// local_profile does not differ; we do not need it
			delete local_profile;
			local_profile = NULLPTR;
		}
		profile->readremoveFiles();
	} else {
		profile->readMakeDefaults();
		profile->readremoveFiles();
	} {
		const char *useflags(cstr("USE"));
		if(likely(useflags != NULLPTR)) {
			(*this)["USE.profile"] = useflags;
		}
		if(likely(use_make_conf != NULLPTR)) {
			(*this)["USE"] = *use_make_conf;
			delete use_make_conf;
		} else if(likely(useflags != NULLPTR)) {
			erase("USE");
		}
	}

	override_by_env(test_in_env_late);

	m_accepted_keywords.clear();
	split_string(&m_accepted_keywords, (*this)["ARCH"]);
	m_arch_set.clear();
	resolve_plus_minus(&m_arch_set, m_accepted_keywords);
	split_string(&m_accepted_keywords, (*this)["ACCEPT_KEYWORDS"]);
	m_accepted_keywords_set = m_arch_set;
	resolve_plus_minus(&m_accepted_keywords_set, m_accepted_keywords);
	m_accepted_keywords = WordVec(m_accepted_keywords_set.begin(), m_accepted_keywords_set.end());
	eix::SignedBool as_arch(eixrc->getBoolText("ACCEPT_KEYWORDS_AS_ARCH", "full"));
	if(as_arch != 0) {
		m_plain_accepted_keywords_set.clear();
		for(WordSet::const_iterator it(m_accepted_keywords_set.begin());
			unlikely(it != m_accepted_keywords_set.end()); ++it) {
			if(std::strchr("-~", (*it)[0]) == NULLPTR) {
				m_plain_accepted_keywords_set.INSERT(*it);
			} else {
				m_plain_accepted_keywords_set.INSERT(it->substr(1));
			}
		}
		m_local_arch_set = &m_plain_accepted_keywords_set;
		if(as_arch < 0) {
			m_auto_arch_set = &m_plain_accepted_keywords_set;
		} else {
			m_auto_arch_set = &m_arch_set;
		}
	} else {
		m_local_arch_set = m_auto_arch_set = &m_arch_set;
	} {
		// Calculate m_raised_arch by prepending ~ to every token
		WordSet archset;
		for(WordSet::const_iterator it(m_arch_set.begin());
			unlikely(it != m_arch_set.end()); ++it) {
			if(std::strchr("-~", (*it)[0]) == NULLPTR) {
				archset.INSERT(string("~") + *it);
			}
		}
		join_to_string(&m_raised_arch, archset);
	}

	// Finalize global and local cascading profile and create user_config
	profile->finalize();
	if(getlocal) {
		if(local_profile != NULLPTR) {
			local_profile->finalize();
		}
		user_config = new PortageUserConfig(this, local_profile);
	}

	WordVec sets_dirs;
	split_string(&sets_dirs, (*eixrc)["EIX_LOCAL_SETS"], true);
	// Expand relative pathnames and the magic "*" of EIX_LOCAL_SETS:
	for(WordVec::size_type i(0); likely(i != sets_dirs.size()); ) {
		switch(sets_dirs[i][0]) {
			case '/':  // absolute path: Nothing special
				break;
			case '*': {  // the magic "*"
					string app;
					if(sets_dirs[i].size() > 1) {
						app.assign(sets_dirs[i], 1, string::npos);
					}
GCC_DIAG_OFF(sign-conversion)
					sets_dirs.erase(sets_dirs.begin() + i);
GCC_DIAG_ON(sign-conversion)
					WordVec::size_type j(i);
					for(RepoList::const_iterator it(repos.second()); likely(it != repos.end()); ++it) {
						if(it->know_path) {
GCC_DIAG_OFF(sign-conversion)
							sets_dirs.insert(sets_dirs.begin() + j, 1, (it->path) + app);
GCC_DIAG_ON(sign-conversion)
							++i;
						}
					}
				}
				// skip ++i:
				continue;
			default:  // relative pathname is relative to $PORTDIR:
				sets_dirs[i].insert(0, (*this)["PORTDIR"]);
				break;
		}
		++i;
	}
	// Now finally read the local sets:
	read_local_sets(sets_dirs);
}

PortageSettings::~PortageSettings() {
	delete profile;
	delete user_config;
}

void PortageSettings::read_world_sets(const char *file) {
	LineVec lines;
	if(!pushback_lines(file, &lines, false, false)) {
		return;
	}
	WordVec the_sets;
	for(WordVec::const_iterator it(lines.begin());
		likely(it != lines.end()); ++it) {
		string s;
		if((*it)[0] == '@') {
			s.assign(*it, 1, string::npos);
		} else {
			s = *it;
		}
		if(s.empty()) {
			continue;
		}
		the_sets.PUSH_BACK(MOVE(s));
	}
	store_world_sets(&the_sets, true);
}

void PortageSettings::store_world_sets(const WordVec *s_world_sets, bool override) {
	if(!s_world_sets) {
		// set defaults:
		know_world_sets = false;
		world_setslist_up_to_date = false;
		world_sets.clear();
		return;
	}
	if((!override) && know_world_sets) {
		return;
	}
	know_world_sets = true;
	world_setslist_up_to_date = false;
	world_sets.clear();
	for(WordVec::const_iterator it(s_world_sets->begin());
		likely(it != s_world_sets->end()); ++it) {
		if(it->empty()) {
			continue;
		}
		world_sets.push_back(*it);
	}
}

void PortageSettings::add_name(SetsList *l, const string& s, bool recurse) const {
	if(s == "system") {
		l->add_system();
		return;
	}
	for(SetsIndex i(0); likely(i != set_names.size()); ++i) {
		if(s == set_names[i]) {
			l->add(i);
			if(recurse) {
				l->add(children_sets[i]);
			}
			return;
		}
	}
}

void PortageSettings::update_world_setslist() {
	world_setslist_up_to_date = true;
	world_setslist.clear();
	for(WordVec::const_iterator it(world_sets.begin());
		likely(it != world_sets.end()); ++it) {
		add_name(&world_setslist, *it, m_recurse_sets);
	}
}

void PortageSettings::calc_world_sets(Package *p) {
	if(!world_setslist_up_to_date)
		update_world_setslist();
	for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
		if(world_setslist.has_system()) {
			if(it->maskflags.isSystem()) {
				it->maskflags.setbits(MaskFlags::MASK_WORLD_SETS);
				continue;
			}
		}
		bool world(false);
		for(Version::SetsIndizes::const_iterator sit(it->sets_indizes.begin());
			sit != it->sets_indizes.end(); ++sit) {
			if(std::find(world_setslist.begin(), world_setslist.end(), *sit) != world_setslist.end()) {
				world = true;
				break;
			}
		}
		if(world) {
			it->maskflags.setbits(MaskFlags::MASK_WORLD_SETS);
		}
	}
}

void PortageSettings::get_setnames(WordSet *names, const Package *p, bool also_nonlocal) const {
	names->clear();
	for(Package::const_iterator it(p->begin()); likely(it != p->end()); ++it) {
		for(Version::SetsIndizes::const_iterator sit(it->sets_indizes.begin());
			likely(sit != it->sets_indizes.end()); ++sit) {
			names->INSERT(set_names[*sit]);
		}
	}
	if(also_nonlocal) {
		if(unlikely(p->is_system_package())) {
			names->INSERT("system");
		}
		if(unlikely(p->is_profile_package())) {
			names->INSERT("profile");
		}
	}
}

string PortageSettings::get_setnames(const Package *p, bool also_nonlocal) const {
	WordSet names;
	get_setnames(&names, p, also_nonlocal);
	return join_to_string(names);
}


static CONSTEXPR const char *sets_exclude[] = { "..", "." , "system", "world", NULLPTR };

void PortageSettings::read_local_sets(const WordVec& dir_list) {
	world_setslist_up_to_date = false;
	set_names.clear();

	// Pushback all set names into set_names, setting dir_size appropriately.
	vector<WordVec::size_type> dir_size(dir_list.size());
	/**
	all_set_names is used as a cache to find duplicate set names faster
	**/
	WordUnorderedSet all_set_names;
	for(WordVec::size_type i(0); likely(i != dir_list.size()); ++i) {
		WordVec temporary_set_names;
		pushback_files(dir_list[i], &temporary_set_names, sets_exclude, 0, true, false);
		WordVec::size_type s(set_names.size());
		// Avoid duplicate sets
		for(WordVec::const_iterator it(temporary_set_names.begin());
			likely(it != temporary_set_names.end()); ++it) {
			if(all_set_names.count(*it) != 0) {
				continue;
			}
			all_set_names.INSERT(*it);
			set_names.PUSH_BACK(MOVE(*it));
		}
		dir_size[i] = set_names.size() - s;
	}
	vector<WordVec> child_names(set_names.size());
	SetsIndex c(0);
	for(WordVec::size_type i(0); likely(i != dir_list.size()); ++i) {
		string dir_slash(dir_list[i]);
		optional_append(&dir_slash, '/');
		for(WordVec::size_type j(0); likely(j != dir_size[i]); ++j) {
			grab_setmasks((dir_slash + set_names[c]).c_str(), c, &(child_names[c]));
			++c;
		}
	}
	m_package_sets.finalize();

	// calculate children_sets and parent_sets:

	if(!m_recurse_sets) {  // Shortcut if we do not need the data
		return;
	}

	children_sets.assign(set_names.size(), SetsList());
	parent_sets.assign(set_names.size(), SetsList());

	// First, we resolve all the set names:
	for(SetsIndex i(0); likely(i != set_names.size()); ++i) {
		for(WordVec::const_iterator it(child_names[i].begin());
			likely(it != child_names[i].end()); ++it) {
			add_name(&(children_sets[i]), *it, m_recurse_sets);
		}
	}

	// Now we recurse:
	for(bool startnew(true); likely(startnew); ) {
		startnew = false;
		for(SetsIndex i(0); likely(i != set_names.size()); ++i) {
			for(SetsList::const_iterator it(children_sets[i].begin());
				it != children_sets[i].end(); ++it) {
				parent_sets[*it].add(i);
				if(children_sets[i].add(children_sets[*it])) {
					startnew = true;
					break;
				}
			}
		}
	}
}

void PortageSettings::calc_recursive_sets(Package *p) const {
	for(Package::iterator vi(p->begin()); likely(vi != p->end()); ++vi) {
		SetsList will_add;
		for(Version::SetsIndizes::const_iterator it(vi->sets_indizes.begin());
			it != vi->sets_indizes.end(); ++it)
			will_add.add(parent_sets[*it]);
		for(SetsList::const_iterator it(will_add.begin());
			it != will_add.end(); ++it)
			vi->add_to_set(*it);
	}
}

bool PortageSettings::calc_allow_upgrade_slots(const Package *p) const {
	if(unlikely(!know_upgrade_policy)) {
		upgrade_policy = settings_rc->getBool("UPGRADE_TO_HIGHEST_SLOT");
		upgrade_policy_exceptions.clear();
		WordVec exceptions;
		split_string(&exceptions, ((*settings_rc)[upgrade_policy ?
				"SLOT_UPGRADE_FORBID" : "SLOT_UPGRADE_ALLOW"]), true);
		for(WordVec::const_iterator it(exceptions.begin());
			likely(it != exceptions.end()); ++it) {
			upgrade_policy_exceptions.add_file(it->c_str(), Mask::maskTypeNone, true, parse_error);
		}
		upgrade_policy_exceptions.finalize();
	}
	if(unlikely(!upgrade_policy_exceptions.empty()) && unlikely(upgrade_policy_exceptions.match_name(p)))
		return !upgrade_policy;
	return upgrade_policy;
}

/**
Verify whether categories in vec, starting at index for file name are ok.
Give an error if not and throw out inappropriate ones.
@return index for next push_back
**/
WordVec::size_type PortageSettings::verify_categories(const string &name, WordVec *vec, WordVec::size_type index) const {
	LineVec::size_type line(0);
	while(index < vec->size()) {
		++line;
		string &s = (*vec)[index];
		// Remove trailing and leading /, see https://github.com/vaeth/eix/issues/8
		while(likely(!s.empty())) {
			if(unlikely(s[s.size() - 1] == '/')) {
				s.erase(s.size() - 1, 1);
			} else if(unlikely(s[0] == '/')) {
				s.erase(0, 1);
			} else {
				break;
			}
		}
		// Do not allow other bad symbols:
		if(likely(!s.empty())) {
			if(likely((*first_not_alnum_or_ok(s.c_str(), "+-_./,;!?*")) == '\0')) {
				++index;
				continue;
			} else {
				parse_error->output(name, line, s, (_("bad category name")));
			}
		}
GCC_DIAG_OFF(sign-conversion)
		vec->erase((vec->begin()) + index);
GCC_DIAG_ON(sign-conversion)
	}
	return index;
}

/**
pushback categories from profiles to vec. Categories may be duplicate.
Result is not cashed, i.e. this should be called only once
**/
void PortageSettings::pushback_categories(WordVec *vec) {
	WordVec::size_type index(vec->size());
	/**
	Merge categories from /etc/portage/categories and
	portdir/profile/categories
	**/
	string filename(m_eprefixconf + USER_CATEGORIES_FILE);
	pushback_lines(filename.c_str(), vec);
	index = verify_categories(filename, vec, index);

	for(RepoList::const_iterator i(repos.begin()); likely(i != repos.end()); ++i) {
		string errtext;
		if(!i->know_path) {
			continue;
		}
		filename.assign(m_eprefixaccessoverlays + (i->path) + "/" + PORTDIR_CATEGORIES_FILE);
		if(!pushback_lines(filename.c_str(), vec, false, true, 0, &errtext)) {
			if(i == repos.begin()) {
				eix::say_error() % errtext;
			}
		}
		index = verify_categories(filename, vec, index);
	}
}

void PortageSettings::addOverlayProfiles(CascadingProfile *p) const {
	RepoList::size_type j(1);
	for(RepoList::const_iterator i(repos.second()); likely(i != repos.end()); ++i, ++j) {
		if(!(i->know_path)) {
			continue;
		}
		string path(m_eprefixaccessoverlays);
		path.append(i->path);
		path.append(1, '/');
		p->listaddFile(path + PORTDIR_MASK_FILE, j, true);
		p->listaddFile(path + PORTDIR_UNMASK_FILE, j, true);
		p->listaddFile(path + PORTDIR_MAKE_DEFAULTS, j, false);
	}
}

PortageUserConfig::PortageUserConfig(PortageSettings *psettings, CascadingProfile *local_profile) {
	m_settings = psettings;
	profile    = local_profile;
	read_use = read_env = read_license = read_restrict = read_cflags = false;
	readKeywords();
	readMasks();
}

PortageUserConfig::~PortageUserConfig() {
	delete profile;
}

bool PortageUserConfig::readMasks() {
	const ParseError *parse_error(m_settings->parse_error);
	bool added(m_localmasks.add_file(((m_settings->m_eprefixconf) + USER_MASK_FILE).c_str(), Mask::maskMask, true, true, parse_error));
	if(m_localmasks.add_file(((m_settings->m_eprefixconf) + USER_UNMASK_FILE).c_str(), Mask::maskUnmask, true, parse_error)) {
		added = true;
	}
	m_localmasks.finalize();
	return added;
}

bool PortageUserConfig::readKeywords() {
	PreList pre_list;
	bool added(false);
	LineVec lines;
	const string& path(m_settings->m_eprefixconf);
	string file(path + USER_KEYWORDS_FILE1);
	if(pushback_lines(file.c_str(), &lines, true)) {
		added = pre_list.handle_file(lines, file, NULLPTR, true, false, false);
		lines.clear();
	}
	file = (path + USER_KEYWORDS_FILE2);
	if(pushback_lines(file.c_str(), &lines, true)) {
		added |= pre_list.handle_file(lines, file, NULLPTR, true, false, false);
	}
	if(!added) {
		return false;
	}
	pre_list.initialize(&m_accept_keywords, m_settings->m_raised_arch, m_settings->parse_error);
	return true;
}

void PortageUserConfig::ReadVersionFile(const char *file, MaskList<KeywordMask> *list) const {
	LineVec lines;
	pushback_lines(file, &lines, true);
	for(LineVec::iterator i(lines.begin());
		likely(i != lines.end()); ++i) {
		if(i->empty())
			continue;
		KeywordMask m;
		string::size_type n(i->find_first_of("\t "));
		string errtext;
		BasicVersion::ParseResult r(m.parseMask(((n == string::npos) ? i->c_str() : i->substr(0, n).c_str()), &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			m_settings->parse_error->output(file, lines.begin(), i, errtext);
		}
		if(likely(r != BasicVersion::parsedError)) {
			if(n != string::npos) {
				m.keywords = "1";  // i->substr(n + 1);
			}
			list->add(m);
		}
	}
}

inline static void increase(char *s) {
	if(*s != 2) {
		++(*s);
	}
}

/**
@return true if some mask from list applied
**/
bool PortageUserConfig::CheckList(Package *p, const MaskList<KeywordMask> *list, Keywords::Redundant flag_double, Keywords::Redundant flag_in) const {
	bool rvalue(false);
	UNORDERED_MAP<Version*, char> counter;
	MaskList<KeywordMask>::Get *keyword_masks(list->get(p));
	if(keyword_masks != NULLPTR) {
		for(MaskList<KeywordMask>::Get::const_iterator it(keyword_masks->begin());
			likely(it != keyword_masks->end()); ++it) {
			rvalue = true;
			Mask::Matches matches;
			it->match(&matches, p);
			for(Mask::Matches::iterator v(matches.begin());
				likely(v != matches.end()); ++v) {
				if(!it->keywords.empty()) {
					increase(&(counter[*v]));
				}
			}
		}
		delete keyword_masks;
	}
	// Also apply the set entries:
	for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
		if(v->sets_indizes.empty()) {
			// Shortcut for the most frequent case
			continue;
		}
		for(Version::SetsIndizes::const_iterator it(v->sets_indizes.begin());
			unlikely(it != v->sets_indizes.end()); ++it) {
			MaskList<KeywordMask>::Get *key_masks(list->get_setname(m_settings->set_names[*it]));
			if(key_masks == NULLPTR) {
				continue;
			}
			rvalue = true;
			char& c(counter[*v]);
			for(MaskList<KeywordMask>::Get::const_iterator m(key_masks->begin());
				likely(m != key_masks->end()); ++m) {
				if(!m->keywords.empty()) {
					increase(&c);
				}
			}
			delete key_masks;
		}
	}
	if(!rvalue) {
		// Shortcut for the most frequent case
		return false;
	}

	for(Package::iterator i(p->begin()); likely(i != p->end()); ++i) {
		char s(counter[*i]);
		if(s == 0) {
			continue;
		}
		Keywords::Redundant redundant(flag_in | i->get_redundant());
		if(s == 2) {
			redundant |= flag_double;
		}
		i->set_redundant(redundant);
	}
	return true;
}

bool PortageUserConfig::CheckFile(Package *p, const char *file, MaskList<KeywordMask> *list, bool *readfile, Keywords::Redundant flag_double, Keywords::Redundant flag_in) const {
	if(!(*readfile)) {
		ReadVersionFile(((m_settings->m_eprefixconf) + file).c_str(), list);
		*readfile = true;
	}
	return CheckList(p, list, flag_double, flag_in);
}

static CONSTEXPR const ArchUsed
	ARCH_NOTHING        = 0,
	ARCH_STABLE         = 1,
	ARCH_UNSTABLE       = 2,
	ARCH_ALIENSTABLE    = 3,
	ARCH_ALIENUNSTABLE  = 4,
	ARCH_EVERYTHING     = 5,
	ARCH_MINUSASTERISK  = 6;  // -* always matches -T WEAKER because it is higher than arch_needed default

static ArchUsed apply_keyword(const string& key, const WordUnorderedSet& keywords_set, KeywordsFlags kf, const WordSet *arch_set, Keywords::Redundant *redundant, Keywords::Redundant check, bool shortcut) {
	if(key[0] == '-') {
		*redundant |= (check & Keywords::RED_STRANGE);
		return ARCH_NOTHING;
	}
	if((keywords_set.count(key) == 0) &&
		((key[0] == '*') || (keywords_set.count("*") == 0)) &&
		((key[0] != '~') || (keywords_set.count("~*") == 0))) {
		// Not found:
		if(key == "**")
			return ARCH_EVERYTHING;
		if(key == "*") {
			if(kf.havesome(KeywordsFlags::KEY_SOMESTABLE))
				return ARCH_ALIENSTABLE;
		}
		if(key == "~*") {
			if(kf.havesome(KeywordsFlags::KEY_TILDESTARMATCH))
				return ARCH_ALIENUNSTABLE;
			*redundant |= (check & Keywords::RED_STRANGE);
			return ARCH_NOTHING;
		}

		// Let us now check whether we trigger RED_STRANGE.
		// Since this test takes time, we check first whether the
		// result is required at all. Otherwise, we are done already:
		if(!(check & Keywords::RED_STRANGE))
			return ARCH_NOTHING;

		// Let s point to the "blank" keyword (without -/~)
		// have_searched is the "flag" which we have already tested.
		const string *s;
		string r;
		char have_searched(key[0]);
		if((have_searched == '-') || (have_searched == '~')) {
			r.assign(key, 1, string::npos);
			s = &r;
		} else {
			s = &key;
			have_searched = '\0';
		}

		// Is the "blank" keyword in arch_set (possibly with -/~)?
		if(arch_set->count(*s) != 0) {
			return ARCH_NOTHING;
		}
		if(arch_set->count(string("~") + *s) != 0) {
			return ARCH_NOTHING;
		}
		if(arch_set->count(string("-") + *s) != 0) {
			return ARCH_NOTHING;
		}

		// Is the "blank" keyword in KEYWORDS (possibly with -/~)?
		// (We can avoid the test which already has failed...)
		if(have_searched != '\0') {
			if(keywords_set.count(*s) != 0) {
				return ARCH_NOTHING;
			}
		}
		if(have_searched != '~') {
			if(keywords_set.count(string("~") + *s) != 0) {
				return ARCH_NOTHING;
			}
		}
		if(have_searched != '-') {
			if(keywords_set.count(string("-") + *s) != 0) {
				return ARCH_NOTHING;
			}
		}

		// None of the above tests succeeded, so have a strange key:
		*redundant |= Keywords::RED_STRANGE;
		return ARCH_NOTHING;
	}
	// Found:
	if(shortcut) {
		// We do not care what stabilized it, so we speed things up:
		return ARCH_STABLE;
	}
	if(key[0] == '~') {
		// Usually, we have ARCH_UNSTABLE, but there are exceptions.
		// First, test special case:
		if(key == "~*") {
			return ARCH_ALIENUNSTABLE;
		}
		// We have an ARCH_UNSTABLE if key is in arch (with or without ~)
		if(arch_set->count(key) != 0) {
			return ARCH_UNSTABLE;
		}
		if(arch_set->count(key.substr(1)) != 0) {
			return ARCH_UNSTABLE;
		}
		return ARCH_ALIENUNSTABLE;
	}
	// Usually, we have ARCH_STABLE, but there are exceptions.
	// First, test special cases:
	if(key[0] == '-') {
		return ARCH_MINUSASTERISK;
	}
	if(key == "*") {
		return ARCH_ALIENSTABLE;
	}
	if(key == "**") {
		return ARCH_EVERYTHING;
	}
	// We have an ARCH_STABLE if key is in arch (with or without ~)
	if(arch_set->count(key) != 0) {
		return ARCH_STABLE;
	}
	if(arch_set->count(string("~") + key) != 0) {
		return ARCH_STABLE;
	}
	return ARCH_ALIENSTABLE;
}

/**
@return true if something from /etc/portage/package.* applied and check involves keywords
**/
bool PortageUserConfig::setKeyflags(Package *p, Keywords::Redundant check) const {
	if((check & Keywords::RED_ALL_KEYWORDS) == Keywords::RED_NOTHING) {
		if(p->restore_keyflags(Version::SAVEKEY_USER))
			return false;
	}
	m_settings->get_effective_keywords_userprofile(p);

	UNORDERED_MAP<Version*, WordVec> sorted_by_versions;

	const MaskList<KeywordMask>::Get *keyword_masks(m_accept_keywords.get(p));
	if(keyword_masks != NULLPTR) {
		for(MaskList<KeywordMask>::Get::const_iterator it(keyword_masks->begin());
			likely(it != keyword_masks->end()); ++it) {
			Mask::Matches matches;
			it->match(&matches, p);
			for(Mask::Matches::iterator v(matches.begin());
				likely(v != matches.end()); ++v) {
				split_string(&(sorted_by_versions[*v]), it->keywords);
				// Set RED_DOUBLE_LINE depending on locally_double
				if(it->locally_double) {
					if(check & Keywords::RED_DOUBLE_LINE)
						v->set_redundant((v->get_redundant()) |
							Keywords::RED_DOUBLE_LINE);
				}
			}
		}
		delete keyword_masks;
	}

	bool rvalue(!sorted_by_versions.empty());

	bool shortcut(!(check & (Keywords::RED_MIXED | Keywords::RED_WEAKER)));
	for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
		// Calculate state of ACCEPT_KEYWORDS, taking into account
		// package.accept_keywords and sets from package.keywords

		Keywords::Redundant redundant(it->get_redundant());
		WordVec kv(m_settings->m_accepted_keywords);
		WordVec::size_type kvsize(kv.size());
		KeywordsFlags kf;
		if(!it->m_accepted_keywords.empty()) {
			split_string(&kv, it->m_accepted_keywords);
		}
		if(!it->sets_indizes.empty()) {
			pushback_set_accepted_keywords(&kv, *it);
		}
		if(kv.size() == kvsize) {
			// Nothing has changed. In this case, we take defaults:
			kf.set_keyflags(it->get_keyflags(m_settings->m_accepted_keywords_set));
			it->keyflags = kf;
			it->save_keyflags(Version::SAVEKEY_ACCEPT);
		} else {
			// We must recalculate:
			WordSet s;
			resolve_plus_minus(&s, kv);
			kv = WordVec(s.begin(), s.end());
			kf.set_keyflags(it->get_keyflags(s));
			kvsize = kv.size();
		}
		bool ori_is_stable(kf.havesome(KeywordsFlags::KEY_STABLE));

		// Were keywords added from /etc/portage/package.accept_keywords?
		// In this case, the "default" accept_keywords are taken from kv_set_nofile.
		// Otherwise, kv_set_nofile remains NULLPTR.
		WordSet *kv_set_nofile(NULLPTR);

		WordVec& kvfile(sorted_by_versions[*it]);
		bool calc_lkw(rvalue);
		if(calc_lkw) {
			if(!kvfile.empty()) {
				redundant |= Keywords::RED_IN_KEYWORDS;
				kv_set_nofile = new WordSet;
				resolve_plus_minus(kv_set_nofile, kv);
				// Add items from /etc/portage/package.accept_keywords to kv
				// but remove matching -... items from kv_set_nofile
				for(WordVec::const_iterator fit(kvfile.begin());
					fit != kvfile.end(); ++fit) {
					if((*fit)[0] == '-') {
						WordSet::iterator where(kv_set_nofile->find(fit->substr(1)));
						if(where != kv_set_nofile->end()) {
							kv_set_nofile->erase(where);
						}
					}
				}
				kv.insert(kv.end(), kvfile.begin(), kvfile.end());
			} else if((check & (Keywords::RED_ALL_KEYWORDS &
				~(Keywords::RED_DOUBLE_LINE | Keywords::RED_IN_KEYWORDS)))
				== Keywords::RED_NOTHING) {
				calc_lkw = false;
			}
		}

		// Keywords were added or we must check for redundancy?
		if(calc_lkw) {
			// Create keywords_set of KEYWORDS from the ebuild
			WordUnorderedSet keywords_set;
			split_string(&keywords_set, it->get_effective_keywords());

			// Create kv_set (of now active keywords), possibly testing for double keywords and -*
			WordSet kv_set;
			if(check & Keywords::RED_DOUBLE) {
				if(kv.size() != WordUnorderedSet(kv.begin(), kv.end()).size()) {
					redundant |= Keywords::RED_DOUBLE;
				}
				if(resolve_plus_minus(&kv_set, kv, &(m_settings->m_accepted_keywords_set))) {
					redundant |= Keywords::RED_DOUBLE;
				}
			} else {
				resolve_plus_minus(&kv_set, kv);
			}

			// First apply the original ACCEPT_KEYWORDS (with package.accept_keywords sets).
			// The point is that we temporarily disable "check" so that
			// ACCEPT_KEYWORDS does not trigger any -T alarm.
			bool stable(false);
			if(kv_set_nofile == NULLPTR) {
				// The case that nothing was added from /etc/portage/package.accept_keywords?
				for(WordSet::const_iterator sit(kv_set.begin());
					sit != kv_set.end(); ++sit) {
					if(apply_keyword(*sit, keywords_set, kf,
						m_settings->m_local_arch_set,
						&redundant, Keywords::RED_NOTHING, true)
						!= ARCH_NOTHING) {
						stable = true;
					}
				}
				kv_set.clear();
			} else {
				// The case that we have some data from /etc/portage/package.accept_keywords in kv_set:
				// In this case, we use kv_set_nofile instead of kv_set for demasking.
				// Moreover, we must remove the demasked data from kv_set...
				for(WordSet::const_iterator sit(kv_set_nofile->begin());
				sit != kv_set_nofile->end(); ++sit) {
					if(apply_keyword(*sit, keywords_set, kf,
						m_settings->m_local_arch_set,
						&redundant, Keywords::RED_NOTHING, true)
						!= ARCH_NOTHING) {
						stable = true;
					}
					// Tests whether keyword is admissible and remove it from kv_set:
					WordSet::iterator where(kv_set.find(*sit));
					if(where == kv_set.end()) {
						// The original keyword was removed by -...
						continue;
					}
					kv_set.erase(where);
				}
				delete kv_set_nofile;
			}

			// Now apply the remaining keywords (i.e. from /etc/portage/package.accept_keywords)
			ArchUsed arch_used(ARCH_NOTHING);
			for(WordSet::iterator kvi(kv_set.begin());
				likely(kvi != kv_set.end()); ++kvi) {
				ArchUsed arch_curr(apply_keyword(*kvi, keywords_set, kf,
					m_settings->m_local_arch_set,
					&redundant, check, shortcut));
				if(arch_curr == ARCH_NOTHING) {
					continue;
				}
				if(arch_used < arch_curr) {
					arch_used = arch_curr;
				}
				if(unlikely(stable || ori_is_stable)) {
					redundant |= (check & Keywords::RED_MIXED);
				}
				stable = true;
			}

			// Was there a reason to trigger a WEAKER alarm?
			if(check & Keywords::RED_WEAKER) {
				ArchUsed arch_needed;
				if(ori_is_stable) {
					arch_needed = ARCH_NOTHING;
				} else if(kf.havesome(KeywordsFlags::KEY_ARCHUNSTABLE)) {
					arch_needed = ARCH_UNSTABLE;
				} else if(kf.havesome(KeywordsFlags::KEY_ALIENSTABLE)) {
					arch_needed = ARCH_ALIENSTABLE;
				} else if(kf.havesome(KeywordsFlags::KEY_ALIENUNSTABLE)) {
					arch_needed = ARCH_ALIENUNSTABLE;
				} else {
					arch_needed = ARCH_EVERYTHING;
				}
				if(unlikely(arch_used > arch_needed)) {
					redundant |= Keywords::RED_WEAKER;
				}
			}

			// If stability was changed, note it and write it back.
			if(stable == kf.havesome(KeywordsFlags::KEY_STABLE)) {
				redundant |= Keywords::RED_NO_CHANGE;
			} else {
				if(stable)
					kf.setbits(KeywordsFlags::KEY_STABLE);
				else
					kf.clearbits(KeywordsFlags::KEY_STABLE);
			}
		}

		// Store the result:

		it->keyflags = kf;
		it->save_keyflags(Version::SAVEKEY_USER);
		if(redundant) {
			it->set_redundant(redundant);
		}
	}
	return rvalue;
}

void PortageUserConfig::pushback_set_accepted_keywords(WordVec *result, const Version *v) const {
	for(Version::SetsIndizes::const_iterator it(v->sets_indizes.begin());
		unlikely(it != v->sets_indizes.end()); ++it) {
		const MaskList<KeywordMask>::Get *keyword_masks(m_accept_keywords.get_setname(m_settings->set_names[*it]));
		if(keyword_masks == NULLPTR) {
			continue;
		}
		for(MaskList<KeywordMask>::Get::const_iterator i(keyword_masks->begin());
			i != keyword_masks->end(); ++i) {
			split_string(result, i->keywords);
		}
		delete keyword_masks;
	}
}

/**
Set stability according to arch or local ACCEPT_KEYWORDS
**/
void PortageSettings::setKeyflags(Package *p, bool use_accepted_keywords) const {
	const WordSet *accept_set;
	Version::SavedKeyIndex ind;
	if(use_accepted_keywords) {
		ind = Version::SAVEKEY_ACCEPT;
		accept_set = &m_accepted_keywords_set;
	} else {
		ind = Version::SAVEKEY_ARCH;
		accept_set = m_auto_arch_set;
	}
	if(p->restore_keyflags(ind))
		return;
	get_effective_keywords_profile(p);
	for(Package::iterator t(p->begin()); likely(t != p->end()); ++t) {
		t->set_keyflags(*accept_set);
		t->save_keyflags(ind);
	}
}

void PortageSettings::get_effective_keywords_profile(Package *p) const {
	if(!p->restore_accepted_effective(Version::SAVEEFFECTIVE_PROFILE)) {
		profile->applyKeywords(p);
		p->save_accepted_effective(Version::SAVEEFFECTIVE_PROFILE);
	}
}

void PortageSettings::get_effective_keywords_userprofile(Package *p) const {
	if(!p->restore_accepted_effective(Version::SAVEEFFECTIVE_USERPROFILE)) {
		bool done(false);
		if(user_config) {
			CascadingProfile *cascade(user_config->profile);
			if(cascade) {
				cascade->applyKeywords(p);
				done = true;
			}
		}
		if(!done)
			get_effective_keywords_profile(p);
		p->save_accepted_effective(Version::SAVEEFFECTIVE_USERPROFILE);
	}
}

void PortageUserConfig::setProfileMasks(Package *p) const {
	if(p->restore_maskflags(Version::SAVEMASK_USERPROFILE)) {
		return;
	}
	if(profile != NULLPTR) {
		profile->applyMasks(p);
	} else {
		m_settings->setMasks(p);
	}
	p->save_maskflags(Version::SAVEMASK_USERPROFILE);
}

/**
@return true if something from /etc/portage/package.* applied and check involves masks
**/
bool PortageUserConfig::setMasks(Package *p, Keywords::Redundant check, bool file_mask_is_profile) const {
	Version::SavedMaskIndex ind(file_mask_is_profile ?
		Version::SAVEMASK_USERFILE : Version::SAVEMASK_USER);
	if((check & Keywords::RED_ALL_KEYWORDS) == Keywords::RED_NOTHING) {
		if(p->restore_maskflags(ind)) {
			return false;
		}
	}
	if(file_mask_is_profile) {
		if(unlikely(!(p->restore_maskflags(Version::SAVEMASK_FILE)))) {
			eix::say_error(_("internal error: Tried to restore nonlocal mask without saving"));
			std::exit(EXIT_FAILURE);
		}
	} else {
		setProfileMasks(p);
	}
	bool rvalue(m_localmasks.applyMasks(p, check));
	// Now we must also apply the set-items of m_localmasks
	for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
		if(v->sets_indizes.empty()) {
			// Shortcut for the most frequent case
			continue;
		}
		for(Version::SetsIndizes::const_iterator it(v->sets_indizes.begin());
			unlikely(it != v->sets_indizes.end()); ++it) {
			m_localmasks.applySetMasks(*v, m_settings->set_names[*it]);
		}
	}

	m_settings->finalize(p);
	p->save_maskflags(ind);
	return rvalue;
}

void PortageSettings::setMasks(Package *p, bool filemask_is_profile) const {
	if(filemask_is_profile) {
		if(!(p->restore_maskflags(Version::SAVEMASK_FILE))) {
			eix::say_error(_("internal error: Tried to restore nonlocal mask without saving"));
			std::exit(EXIT_FAILURE);
		}
		return;
	}
	if(p->restore_maskflags(Version::SAVEMASK_PROFILE)) {
		return;
	}
	profile->applyMasks(p);
	p->save_maskflags(Version::SAVEMASK_PROFILE);
}

void PortageSettings::calc_local_sets(Package *p) const {
	m_package_sets.applyListItems(p);
	if(m_recurse_sets) {
		calc_recursive_sets(p);
	}
}

bool PortageSettings::use_expand(string *var, string *expvar, const string& value) const {
	string::size_type s(value.size());
	for(string::size_type pos(0);
		((pos = value.find('_', pos)) != string::npos) &&
		(pos != 0) && (pos + 1 < s); ++pos) {
		if(!know_expands) {
			know_expands = true;
			WordSet use_expands;
			resolve_plus_minus(&use_expands, (*this)["USE_EXPAND"]);
			for(WordSet::const_iterator it(use_expands.begin());
				it != use_expands.end(); ++it) {
				expand_vars[to_lower(*it)] = *it;
			}
		}
		const_iterator it(expand_vars.find(value.substr(0, pos)));
		if(it != expand_vars.end()) {
			*var = it->second;
			*expvar = value.substr(pos + 1);
			return true;
		}
	}
	return false;
}

void PortageSettings::init_static() {
	eix_assert_static(emptystring == NULLPTR);
	emptystring = new string;
	OverlayIdent::init_static();
	CascadingProfile::init_static();
}
