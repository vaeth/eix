// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "portagesettings.h"

#include <config.h>
#include <portage/conf/cascadingprofile.h>

#include <portage/mask.h>
#include <portage/package.h>
#include <portage/version.h>

#include <eixTk/utils.h>
#include <eixTk/sysutils.h>
#include <eixTk/filenames.h>

#include <eixrc/eixrc.h>

#include <varsreader.h>

#include <fstream>
#include <fnmatch.h>
#include <algorithm>


using namespace std;

bool
grab_masks(const char *file, Mask::Type type, MaskList<Mask> *cat_map, vector<Mask*> *mask_vec, bool recursive)
{
	vector<string> lines;
	if( ! pushback_lines(file, &lines, true, recursive))
		return false;
	for(vector<string>::iterator it=lines.begin(); it<lines.end(); ++it)
	{
		string line=*it;
		try {
			Mask *m = new Mask(line.c_str(), type);
			if(cat_map) {
				cat_map->add(m);
			}
			else {
				mask_vec->push_back(m);
			}
		}
		catch(const ExBasic &e) {
			portage_parse_error(file, lines.begin(), it, e);
		}
	}
	return true;
}

static bool
grab_setmasks(const char *file, MaskList<SetMask> *masklist, SetsIndex i, vector<string> &contains_set, bool recursive = false)
{
	vector<string> lines;
	if( ! pushback_lines(file, &lines, true, recursive))
		return false;
	for(vector<string>::iterator it=lines.begin(); it<lines.end(); ++it)
	{
		string line=*it;
		try {
			SetMask *m = new SetMask(line.c_str(), i);
			if(m->is_set()) {
				contains_set.push_back(m->getName());
				delete m;
			}
			else
				masklist->add(m);
		}
		catch(const ExBasic &e) {
			portage_parse_error(file, lines.begin(), it, e);
		}
	}
	return true;
}

/** Keys that should accumulate their content rathern then replace. */
static const char *default_accumulating_keys[] = {
	"USE",
	"CONFIG_*",
	"FEATURES",
	"ACCEPT_KEYWORDS",
	NULL
};

/** Environment variables which should take effect before reading profiles. */
static const char *test_in_env_early[] = {
	"PORTAGE_PROFILE",
	"PORTDIR",
	"PORTDIR_OVERLAY",
	NULL
};

/** Environment variables which should add/override all other settings. */
static const char *test_in_env_late[] = {
	"USE",
	"CONFIG_PROTECT",
	"CONFIG_PROTECT_MASK",
	"FEATURES",
	"ARCH",
	"ACCEPT_KEYWORDS",
	NULL
};

inline static bool is_accumulating(const char **accumulating, const char *key)
{
	const char *match;
	while((match = *(accumulating++)) != NULL) {
		if(fnmatch(match, key, 0) == 0)
			return true;
	}
	return false;
}

void PortageSettings::override_by_env(const char **vars)
{
	const char *var;
	while((var = *(vars++)) != NULL)
	{
		const char *e = getenv(var);
		if(!e)
			continue;
		if(!is_accumulating(default_accumulating_keys, var))
		{
			(*this)[var] = e;
			continue;
		}
		string &ref = ((*this)[var]);
		if(ref.empty())
			ref = e;
		else
			ref.append(string("\n") + e);
	}
}

void PortageSettings::read_config(const string &name, const string &prefix)
{
	VarsReader configfile(VarsReader::SUBST_VARS|VarsReader::INTO_MAP|VarsReader::APPEND_VALUES|VarsReader::ALLOW_SOURCE);
	configfile.accumulatingKeys(default_accumulating_keys);
	configfile.useMap(this);
	configfile.setPrefix(prefix);
	configfile.read(name.c_str());
}

string PortageSettings::resolve_overlay_name(const string &path, bool resolve)
{
	if(resolve) {
		string full = m_eprefixoverlays;
		full.append(path);
		return normalize_path(full.c_str(), true);
	}
	return normalize_path(path.c_str(), false);
}

void PortageSettings::add_overlay(string &path, bool resolve, bool modify)
{

	string name = resolve_overlay_name(path, resolve);
	if(modify)
		path = name;
	/* If the overlay exists, don't add it */
	if(find_filenames(overlays.begin(), overlays.end(),
			name.c_str(), false, false) != overlays.end())
			return;
	/* If the overlay is PORTDIR, don't add it */
	if(same_filenames((*this)["PORTDIR"].c_str(), name.c_str(), false, false))
		return;
	overlays.push_back(name);
}

void PortageSettings::add_overlay_vector(vector<string> &v, bool resolve, bool modify)
{
	for(vector<string>::iterator it = v.begin(); it != v.end(); ++it)
		add_overlay(*it, resolve, modify);
}

/** Read make.globals and make.conf. */
PortageSettings::PortageSettings(EixRc &eixrc, bool getlocal, bool init_world)
{
	settings_rc = &eixrc;
	know_upgrade_policy = false;
	m_obsolete_minusasterisk = eixrc.getBool("OBSOLETE_MINUSASTERISK");
	m_recurse_sets    = eixrc.getBool("RECURSIVE_SETS");
	m_eprefixconf     = eixrc.m_eprefixconf;
	m_eprefixprofile  = eixrc["EPREFIX_PORTAGE_PROFILE"];
	m_eprefixportdir  = eixrc["EPREFIX_PORTDIR"];
	m_eprefixoverlays = eixrc["EPREFIX_OVERLAYS"];
	m_eprefixaccessoverlays = eixrc["EPREFIX_ACCESS_OVERLAYS"];

	const string &eprefixsource = eixrc["EPREFIX_SOURCE"];
	const string &make_globals  = eixrc["MAKE_GLOBALS"];
	if(is_file(make_globals.c_str()))
		read_config(make_globals, eprefixsource);
	else
		read_config(m_eprefixconf + MAKE_GLOBALS_FILE, eprefixsource);
	read_config(m_eprefixconf + MAKE_CONF_FILE, eprefixsource);

	override_by_env(test_in_env_early);
	/* Normalize "PORTDIR": */
	{
		string &ref = (*this)["PORTDIR"];
		string full = m_eprefixportdir;
		if(ref.empty())
			full.append("/usr/portage");
		else
			full.append(ref);
		ref = normalize_path(full.c_str(), true, true);
	}
	/* Normalize overlays and erase duplicates */
	{
		string &ref = (*this)["PORTDIR_OVERLAY"];
		vector<string> overlayvec = split_string(ref, spaces, true, true);
		add_overlay_vector(overlayvec, true, true);
		ref = join_vector(overlayvec);
	}

	profile = new CascadingProfile(this, init_world);
	{
		store_world_sets(NULL);
		bool read_world = false;
		if(init_world) {
			if(eixrc.getBool("SAVE_WORLD"))
				read_world = true;
		}
		else {
			if(eixrc.getBool("CURRENT_WORLD"))
				read_world = true;
		}
		if(read_world) {
			if(grab_masks(eixrc["EIX_WORLD"].c_str(), Mask::maskInWorld, &(profile->m_world), false)) {
				profile->use_world = true;
			}
			read_world_sets(eixrc["EIX_WORLD_SETS"].c_str());
		}
		read_local_sets(split_string(eixrc["EIX_LOCAL_SETS"], spaces, true, true));
	}

	profile->listaddFile(((*this)["PORTDIR"] + PORTDIR_MASK_FILE).c_str());
	profile->listaddProfile();
	profile->readMakeDefaults();
	profile->readremoveFiles();
	CascadingProfile *local_profile = NULL;
	if(getlocal)
		local_profile = new CascadingProfile(*profile);
	addOverlayProfiles(profile);
	if(getlocal) {
		local_profile->listaddProfile((m_eprefixconf + USER_PROFILE_DIR).c_str());
		local_profile->readMakeDefaults();
		if(local_profile->readremoveFiles()) {
			addOverlayProfiles(local_profile);
			local_profile->readMakeDefaults();
			local_profile->readremoveFiles();
		}
		else {
			delete local_profile;
			local_profile = NULL;
		}
		profile->readremoveFiles();
	}
	else {
		profile->readMakeDefaults();
		profile->readremoveFiles();
		user_config = NULL;
	}
	override_by_env(test_in_env_late);

	m_accepted_keywords = split_string((*this)["ARCH"]);
	resolve_plus_minus(m_arch_set, m_accepted_keywords, m_obsolete_minusasterisk);
	push_backs<string>(m_accepted_keywords, split_string((*this)["ACCEPT_KEYWORDS"]));
	resolve_plus_minus(m_accepted_keywords_set, m_accepted_keywords, m_obsolete_minusasterisk);
	make_vector<string>(m_accepted_keywords, m_accepted_keywords_set);
	if(eixrc.getBool("ACCEPT_KEYWORDS_AS_ARCH"))
		m_local_arch_set = &m_accepted_keywords_set;
	else
		m_local_arch_set = &m_arch_set;

	if(getlocal)
		user_config = new PortageUserConfig(this, local_profile);
}

PortageSettings::~PortageSettings()
{
	if(profile) {
		delete profile;
	}
	if(user_config) {
		delete user_config;
	}
}

void
PortageSettings::read_world_sets(const char *file)
{
	vector<string> lines;
	if(!pushback_lines(file, &lines, true, false))
		return;
	vector<string> the_sets;
	for(vector<string>::const_iterator it = lines.begin();
		it != lines.end(); ++it) {
		string s;
		if((*it)[0] == '@')
			s = it->substr(1);
		else
			s = *it;
		if(s.empty())
			continue;
		the_sets.push_back(s);
	}
	store_world_sets(&the_sets, true);
}

void
PortageSettings::store_world_sets(const std::vector<std::string> *s_world_sets, bool override)
{
	if(!s_world_sets) {
		// set defaults:
		know_world_sets = false;
		world_setslist_up_to_date = false;
		world_sets.clear();
		return;
	}
	if((!override) && know_world_sets)
		return;
	know_world_sets = true;
	world_setslist_up_to_date = false;
	world_sets.clear();
	for(vector<string>::const_iterator it = s_world_sets->begin();
		it != s_world_sets->end(); ++it) {
		if(it->empty())
			continue;
		world_sets.push_back(*it);
	}
}

void
PortageSettings::add_name(SetsList &l, const string &s, bool recurse) const
{
	if(s == "system") {
		l.add_system();
		return;
	}
	for(SetsIndex i = 0; i != set_names.size(); ++i) {
		if(s == set_names[i]) {
			l.add(i);
			if(recurse)
				l.add(children_sets[i]);
			return;
		}
	}
}

void
PortageSettings::update_world_setslist()
{
	world_setslist_up_to_date = true;
	world_setslist.clear();
	for(vector<string>::const_iterator it = world_sets.begin();
		it != world_sets.end(); ++it)
		add_name(world_setslist, *it, m_recurse_sets);
}

void
PortageSettings::calc_world_sets(Package *p)
{
	if(!world_setslist_up_to_date)
		update_world_setslist();
	for(Package::iterator it = p->begin(); it != p->end(); ++it) {
		if(world_setslist.has_system()) {
			if(it->maskflags.isSystem()) {
				it->maskflags.setbits(MaskFlags::MASK_WORLD_SETS);
				continue;
			}
		}
		bool world = false;
		for(std::vector<SetsIndex>::const_iterator sit = it->sets_indizes.begin();
			sit != it->sets_indizes.end(); ++sit) {
			if(std::find(world_setslist.begin(), world_setslist.end(), *sit) != world_setslist.end()) {
				world = true;
				break;
			}
		}
		if(world)
			it->maskflags.setbits(MaskFlags::MASK_WORLD_SETS);
	}
}

void
PortageSettings::get_setnames(vector<string> &names, const Package *p, bool also_nonlocal) const
{
	names.clear();
	for(Package::const_iterator it = p->begin(); it != p->end(); ++it) {
		for(std::vector<SetsIndex>::const_iterator sit = it->sets_indizes.begin();
			sit != it->sets_indizes.end(); ++sit) {
			names.push_back(set_names[*sit]);
		}
	}
	if(also_nonlocal && p->is_system_package())
		names.push_back("system");
	sort_uniquify(names);
}

std::string
PortageSettings::get_setnames(const Package *p, bool also_nonlocal) const
{
	vector<string> names;
	get_setnames(names, p, also_nonlocal);
	return join_vector(names);
}


static const char *sets_exclude[] = { "..", "." , "system", "world", NULL };

void
PortageSettings::read_local_sets(const vector<string> &dir_list)
{
	world_setslist_up_to_date = false;
	set_names.clear();

	vector<vector<string>::size_type> dir_size(dir_list.size());
	for(vector<string>::size_type i = 0; i != dir_list.size(); ++i) {
		vector<string>::size_type s = set_names.size();
		pushback_files(dir_list[i], set_names, sets_exclude, 0, false, false);
		dir_size[i] = set_names.size() - s;
	}
	vector<vector<string> > child_names(set_names.size());
	SetsIndex c = 0;
	for(vector<string>::size_type i = 0; i != dir_list.size(); ++i) {
		string dir_slash = dir_list[i];
		optional_append(dir_slash, '/');
		for(vector<string>::size_type j = 0; j != dir_size[i]; ++j) {
			grab_setmasks((dir_slash + set_names[c]).c_str(), &m_package_sets, c, child_names[c]);
			++c;
		}
	}

	// calculate children_sets and parent_sets:

	if(!m_recurse_sets) // Shortcut if we do not need the data
		return;

	children_sets.assign(set_names.size(), SetsList());
	parent_sets.assign(set_names.size(), SetsList());

	// First, we resolve all the set names:
	for(SetsIndex i = 0; i != set_names.size(); ++i) {
		for(vector<string>::const_iterator it = child_names[i].begin();
			it != child_names[i].end(); ++it)
			add_name(children_sets[i], *it, m_recurse_sets);
	}

	// Now we recurse:
	for(bool startnew = true; startnew; ) {
		startnew = false;
		for(SetsIndex i = 0; i != set_names.size(); ++i) {
			for(SetsList::const_iterator it = children_sets[i].begin();
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

void
PortageSettings::calc_recursive_sets(Package *p) const
{
	for(Package::iterator vi = p->begin(); vi != p->end(); ++vi) {
		SetsList will_add;
		for(vector<SetsIndex>::const_iterator it = vi->sets_indizes.begin();
			it != vi->sets_indizes.end(); ++it)
			will_add.add(parent_sets[*it]);
		for(SetsList::const_iterator it = will_add.begin();
			it != will_add.end(); ++it)
			vi->add_to_set(*it);
	}
}

bool
PortageSettings::calc_allow_upgrade_slots(const Package *p)
{
	if(!know_upgrade_policy) {
		upgrade_policy = settings_rc->getBool("UPGRADE_TO_HIGHEST_SLOT");
		upgrade_policy_exceptions.clear();
		vector<string> exceptions = split_string(
			((*settings_rc)[upgrade_policy ?
				"SLOT_UPGRADE_FORBID" : "SLOT_UPGRADE_ALLOW"]),
			spaces, true, true);
		for(vector<string>::const_iterator it = exceptions.begin();
			it != exceptions.end(); ++it)
			grab_masks(it->c_str(), Mask::maskTypeNone, &upgrade_policy_exceptions, NULL, true);
	}
	if((!upgrade_policy_exceptions.empty()) && upgrade_policy_exceptions.get(p))
		return !upgrade_policy;
	return upgrade_policy;
}

/** Return vector of all possible categories.
 * Reads categories on first call. */
vector<string> *PortageSettings::getCategories()
{
	if(m_categories.empty()) {
		/* Merge categories from /etc/portage/categories and
		 * portdir/profile/categories */
		pushback_lines((m_eprefixconf + USER_CATEGORIES_FILE).c_str(), &m_categories);

		pushback_lines(((*this)["PORTDIR"] + PORTDIR_CATEGORIES_FILE).c_str(), &m_categories);
		for(vector<string>::iterator i = overlays.begin();
			i != overlays.end();
			++i)
		{
			pushback_lines((m_eprefixaccessoverlays + (*i) + "/" + PORTDIR_CATEGORIES_FILE).c_str(),
			               &m_categories);
		}

		sort_uniquify(m_categories);
	}
	return &m_categories;
}

void
PortageSettings::addOverlayProfiles(CascadingProfile *p) const
{
	for(vector<string>::const_iterator i = overlays.begin();
		i != overlays.end(); ++i)
		p->listaddFile((m_eprefixaccessoverlays + (*i) + "/" + PORTDIR_MASK_FILE).c_str());
}

PortageUserConfig::PortageUserConfig(PortageSettings *psettings, CascadingProfile *local_profile)
{
	m_settings = psettings;
	profile    = local_profile;
	readKeywords();
	readMasks();
	read_use = read_cflags = false;
}

PortageUserConfig::~PortageUserConfig()
{
	if(profile) {
		delete profile;
	}
}

bool
PortageUserConfig::readMasks()
{
	bool mask_ok = grab_masks(((m_settings->m_eprefixconf) + USER_MASK_FILE).c_str(), Mask::maskMask, &m_localmasks, true);
	bool unmask_ok = grab_masks(((m_settings->m_eprefixconf) + USER_UNMASK_FILE).c_str(), Mask::maskUnmask, &m_localmasks, true);
	return mask_ok && unmask_ok;
}

void
PortageUserConfig::ReadVersionFile (const char *file, MaskList<KeywordMask> *list)
{
	vector<string> lines;
	pushback_lines(file, &lines, false, true);
	for(vector<string>::iterator i(lines.begin());
		i != lines.end();
		i++)
	{
		if(i->empty())
			continue;
		try {
			KeywordMask *m = NULL;
			string::size_type n = i->find_first_of("\t ");
			if(n == string::npos) {
				m = new KeywordMask(i->c_str());
			}
			else {
				m = new KeywordMask(i->substr(0, n).c_str());
				m->keywords = "1"; //i->substr(n + 1);
			}
			list->add(m);
		}
		catch(const ExBasic &e)
		{ }
	}
}

/// @return true if some mask from list applied
bool PortageUserConfig::CheckList(Package *p, const MaskList<KeywordMask> *list, Keywords::Redundant flag_double, Keywords::Redundant flag_in)
{
	const eix::ptr_list<KeywordMask> *keyword_masks = list->get(p);
	map<Version*,char> sorted_by_versions;

	if(!keyword_masks)
		return false;
	if(keyword_masks->empty())
		return false;
	for(eix::ptr_list<KeywordMask>::const_iterator it = keyword_masks->begin();
		it != keyword_masks->end();
		++it)
	{
		eix::ptr_list<Version> matches = it->match(*p);

		for(eix::ptr_list<Version>::iterator  v = matches.begin();
			v != matches.end();
			++v)
		{
			if(it->keywords.empty())
				continue;
			char &s = sorted_by_versions[*v];
			if(s)
				s = 2;
			else
				s = 1;
		}
	}

	for(Package::iterator i = p->begin();
		i != p->end();
		++i)
	{
		char s = sorted_by_versions[*i];
		if(!s)
			continue;
		Keywords::Redundant redundant = flag_in | i->get_redundant();
		if(s == 2)
			redundant |= flag_double;
		i->set_redundant(redundant);
	}
	return true;
}

bool PortageUserConfig::CheckFile(Package *p, const char *file, MaskList<KeywordMask> *list, bool *readfile, Keywords::Redundant flag_double, Keywords::Redundant flag_in) const
{
	if(!(*readfile))
	{
		ReadVersionFile(((m_settings->m_eprefixconf) + file).c_str(), list);
		*readfile = true;
	}
	return CheckList(p, list, flag_double, flag_in);
}

typedef struct {
	string keywords;
	bool locally_double;
} KeywordsData;

bool PortageUserConfig::readKeywords() {
	// Prepend a ~ to every token.
	string fscked_arch;
	{
		vector<string> archvec;
		for(set<string>::const_iterator it = m_settings->m_arch_set.begin(); it != m_settings->m_arch_set.end(); ++it) {
			if(strchr("-~", (*it)[0]) == NULL) {
				archvec.push_back(string("~") + *it);
			}
		}
		sort_uniquify(archvec);
		fscked_arch = join_vector(archvec);
	}

	vector<string> lines;
	string filename((m_settings->m_eprefixconf) + USER_KEYWORDS_FILE);

	pushback_lines(filename.c_str(), &lines, false, true);

	/* Build a dictionary of atom -> arguments, e.g. in the example
	 *   foo/bar 1
	 *   foo/bar 2
	 *   =foo/bar-1 3
	 *   =foo/bar-1 4
	 *
	 * the resulting dictionary would look like this
	 *   foo/bar    -> 1 2
	 *   =foo/bar-1 -> 3 4
	 *
	 * default keywords (~ARCH) are only attached if the value in the
	 * dictionary is empty.
	 *
	 * We read in two passes, first creating the dictionary (and remember BTW
	 * which were doubled) and then create the KeywordMask from them.
	 */

	map<string, KeywordsData> have;
	for(vector<string>::size_type i = 0; i < lines.size(); ++i)
	{
		if(lines[i].empty())
			continue;

		string::size_type n = lines[i].find_first_of("\t ");
		string name, content;
		if(n == string::npos) {
			name = lines[i];
		}
		else {
			name = lines[i].substr(0, n);
			content = lines[i].substr(n);
		}
		lines[i] = name;
		map<string, KeywordsData>::iterator old = have.find(name);
		if(old == have.end()) {
			KeywordsData *f = &(have[name]);
			f->locally_double = false;
			f->keywords = content;
		}
		else {
			(old->second).locally_double = true;
			(old->second).keywords.append(content);
		}
	}

	for(vector<string>::iterator i(lines.begin());
		i != lines.end();
		++i)
	{
		if(i->empty())
			continue;
		try {
			KeywordMask *m = new KeywordMask(i->c_str());
			KeywordsData *f = &(have[*i]);
			m->keywords       = f->keywords.empty() ? fscked_arch : f->keywords;
			m->locally_double = f->locally_double;
			m_keywords.add(m);
		}
		catch(const ExBasic &e) {
			portage_parse_error(filename, lines.begin(), i, e);
		}
	}
	return true;
}

typedef char ArchUsed;
static const ArchUsed
	ARCH_NOTHING        = 0,
	ARCH_STABLE         = 1,
	ARCH_UNSTABLE       = 2,
	ARCH_ALIENSTABLE    = 3,
	ARCH_ALIENUNSTABLE  = 4,
	ARCH_EVERYTHING     = 5,
	ARCH_MINUSASTERISK  = 6; // -* always matches -T WEAKER because it is higher than arch_needed default

static inline ArchUsed
apply_keyword(const string &key, const set<string> &keywords_set, KeywordsFlags kf,
	const set<string> *arch_set, bool obsolete_minus,
	Keywords::Redundant &redundant, Keywords::Redundant check, bool shortcut)
{
	static string tilde("~"), minus("-");
	if(!obsolete_minus) {
		if(key[0] == '-') {
			redundant |= (check & Keywords::RED_STRANGE);
			return ARCH_NOTHING;
		}
	}
	if(keywords_set.find(key) == keywords_set.end()) {
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
			redundant |= (check & Keywords::RED_STRANGE);
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
		char have_searched = key[0];
		if((have_searched == '-') || (have_searched == '~')) {
			r = key.substr(1); s = &r;
		}
		else {
			s = &key; have_searched = '\0';
		}

		// Is the "blank" keyword in arch_set (possibly with -/~)?
		if(arch_set->find(*s) != arch_set->end())
			return ARCH_NOTHING;
		if(arch_set->find(tilde + *s) != arch_set->end())
			return ARCH_NOTHING;
		if(arch_set->find(minus + *s) != arch_set->end())
			return ARCH_NOTHING;

		// Is the "blank" keyword in KEYWORDS (possibly with -/~)?
		// (We can avoid the test which already has failed...)
		if(have_searched != '\0') {
			if(keywords_set.find(*s) != keywords_set.end())
				return ARCH_NOTHING;
		}
		if(have_searched != '~') {
			if(keywords_set.find(tilde + *s) != keywords_set.end())
				return ARCH_NOTHING;
		}
		if(have_searched != '-') {
			if(keywords_set.find(minus + *s) != keywords_set.end())
				return ARCH_NOTHING;
		}

		// None of the above tests succeeded, so have a strange key:
		redundant |= Keywords::RED_STRANGE;
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
		if(key == "~*")
			return ARCH_ALIENUNSTABLE;
		// We have an ARCH_UNSTABLE if key is in arch (with or without ~)
		if(arch_set->find(key) != arch_set->end())
			return ARCH_UNSTABLE;
		if(arch_set->find(key.substr(1)) != arch_set->end())
			return ARCH_UNSTABLE;
		return ARCH_ALIENUNSTABLE;
	}
	// Usually, we have ARCH_STABLE, but there are exceptions.
	// First, test special cases:
	if(key[0] == '-')
		return ARCH_MINUSASTERISK;
	if(key == "*")
		return ARCH_ALIENSTABLE;
	if(key == "**")
		return ARCH_EVERYTHING;
	// We have an ARCH_STABLE if key is in arch (with or without ~)
	if(arch_set->find(key) != arch_set->end())
		return ARCH_STABLE;
	if(arch_set->find(tilde + key) != arch_set->end())
		return ARCH_STABLE;
	return ARCH_ALIENSTABLE;
}

/// @return true if something from /etc/portage/package.* applied and check involves keywords
bool
PortageUserConfig::setKeyflags(Package *p, Keywords::Redundant check) const
{
	if((check & Keywords::RED_ALL_KEYWORDS) == Keywords::RED_NOTHING)
	{
		if(p->restore_keyflags(Version::SAVEKEY_USER))
			return false;
	}

	const eix::ptr_list<KeywordMask> *keyword_masks = m_keywords.get(p);
	map<Version*,vector<string> > sorted_by_versions;
	bool rvalue = false;

	bool obsolete_minusasterisk = m_settings->m_obsolete_minusasterisk;
	if(keyword_masks && (!keyword_masks->empty()))
	{
		rvalue = true;
		for(eix::ptr_list<KeywordMask>::const_iterator it = keyword_masks->begin();
			it != keyword_masks->end();
			++it)
		{
			eix::ptr_list<Version> matches = it->match(*p);

			for(eix::ptr_list<Version>::iterator  v = matches.begin();
				v != matches.end();
				++v)
			{
				push_backs<string>(sorted_by_versions[*v], split_string(it->keywords));
				// Set RED_DOUBLE_LINE depending on locally_double
				if(it->locally_double) {
					if(check & Keywords::RED_DOUBLE_LINE)
						v->set_redundant((v->get_redundant()) |
							Keywords::RED_DOUBLE_LINE);
				}
			}
		}
	}

	bool shortcut = !(check & (Keywords::RED_MIXED | Keywords::RED_WEAKER));
	for(Package::iterator it = p->begin();
		it != p->end(); ++it)
	{
		// Calculate ACCEPT_KEYWORDS (with package.keywords sets) state:

		Keywords::Redundant redundant = it->get_redundant();
		vector<string> kv = m_settings->m_accepted_keywords;
		vector<string>::size_type kvsize = kv.size();
		KeywordsFlags kf;
		bool use_default;
		if(it->sets_indizes.empty())
			use_default = true;
		else {
			pushback_set_accepted_keywords(kv, *it);
			if(kv.size() == kvsize)
				use_default = true;
			else {
				set<string> s;
				resolve_plus_minus(s, kv, obsolete_minusasterisk);
				make_vector(kv, s);
				kf.set(it->get_keyflags(s, obsolete_minusasterisk));
				kvsize = kv.size();
				use_default = false;
			}
		}
		if(use_default) {
			kf.set(it->get_keyflags(m_settings->m_accepted_keywords_set, obsolete_minusasterisk));
			it->keyflags = kf;
			it->save_keyflags(Version::SAVEKEY_ACCEPT);
		}
		bool ori_is_stable = kf.havesome(KeywordsFlags::KEY_STABLE);

		// Were keywords added from /etc/portage/package.keywords?
		vector<string> &kvnew = sorted_by_versions[*it];
		bool calc_lkw = rvalue;
		if(calc_lkw) {
			if(!kvnew.empty()) {
				redundant |= Keywords::RED_IN_KEYWORDS;
				push_backs(kv, kvnew);
			}
			else if((check & (Keywords::RED_ALL_KEYWORDS &
				~(Keywords::RED_DOUBLE_LINE | Keywords::RED_IN_KEYWORDS)))
				== Keywords::RED_NOTHING)
				calc_lkw = false;
		}

		// Keywords were added or we must check for redundancy?
		if(calc_lkw)
		{
			// Create keywords_set of KEYWORDS from the ebuild
			set<string> keywords_set;
			make_set<string>(keywords_set, split_string(it->get_full_keywords()));

			// Create kv_set (of now active keywords), possibly testing for double keywords and -*
			set<string> kv_set;
			bool minusasterisk;
			if(check & Keywords::RED_DOUBLE) {
				vector<string> sorted = kv;
				if(sort_uniquify(sorted, true))
					redundant |= Keywords::RED_DOUBLE;
				bool minuskeyword = false;
				minusasterisk = resolve_plus_minus(kv_set, kv, obsolete_minusasterisk, &minuskeyword, &(m_settings->m_accepted_keywords_set));
				if(minuskeyword)
					redundant |= Keywords::RED_DOUBLE;
			}
			else
				minusasterisk = resolve_plus_minus(kv_set, kv, obsolete_minusasterisk);
			if(minusasterisk && !obsolete_minusasterisk)
				redundant |= (check & Keywords::RED_MINUSASTERISK);

			// First apply the original ACCEPT_KEYWORDS (with package.keywords sets),
			// removing them from kv_set meanwhile.
			// The point is that we temporarily disable "check" so that
			// ACCEPT_KEYWORDS does not trigger any -T alarm.
			bool stable = false;
			for(vector<string>::size_type i = 0; i != kv.size(); ++i) {
				// Tests whether keyword is admissible and remove it:
				set<string>::iterator where = kv_set.find(kv[i]);
				if(where == kv_set.end()) {
					// The original keyword was removed by -...
					continue;
				}
				kv_set.erase(where);
				if(apply_keyword(kv[i], keywords_set, kf,
					m_settings->m_local_arch_set,
					obsolete_minusasterisk,
					redundant, Keywords::RED_NOTHING, true)
					!= ARCH_NOTHING)
					stable = true;
			}

			// Now apply the remaining keywords (i.e. from /etc/portage/package.keywords)
			ArchUsed arch_used = ARCH_NOTHING;
			for(set<string>::iterator kvi = kv_set.begin();
				kvi != kv_set.end(); ++kvi) {
				ArchUsed arch_curr = apply_keyword(*kvi, keywords_set, kf,
					m_settings->m_local_arch_set,
					obsolete_minusasterisk,
					redundant, check, shortcut);
				if(arch_curr == ARCH_NOTHING)
					continue;
				if(arch_used < arch_curr)
					arch_used = arch_curr;
				if(stable || ori_is_stable)
					redundant |= (check & Keywords::RED_MIXED);
				stable = true;
			}

			// Was there a reason to trigger a WEAKER alarm?
			if(check & Keywords::RED_WEAKER)
			{
				ArchUsed arch_needed;
				if(ori_is_stable)
					arch_needed = ARCH_NOTHING;
				else if(kf.havesome(KeywordsFlags::KEY_ARCHUNSTABLE))
					arch_needed = ARCH_UNSTABLE;
				else if(kf.havesome(KeywordsFlags::KEY_ALIENSTABLE))
					arch_needed = ARCH_ALIENSTABLE;
				else if(kf.havesome(KeywordsFlags::KEY_ALIENUNSTABLE))
					arch_needed = ARCH_ALIENUNSTABLE;
				else
					arch_needed = ARCH_EVERYTHING;
				if(arch_used > arch_needed)
					redundant |= Keywords::RED_WEAKER;
			}

			// If stability was changed, note it and write it back.
			if(stable == kf.havesome(KeywordsFlags::KEY_STABLE))
				redundant |= Keywords::RED_NO_CHANGE;
			else {
				if(stable)
					kf.setbits(KeywordsFlags::KEY_STABLE);
				else
					kf.clearbits(KeywordsFlags::KEY_STABLE);
			}
		}

		// Store the result:

		it->keyflags=kf;
		it->save_keyflags(Version::SAVEKEY_USER);
		if(redundant)
			it->set_redundant(redundant);
	}
	return rvalue;
}

void
PortageUserConfig::pushback_set_accepted_keywords(vector<string> &result, const Version *v) const
{
	for(vector<SetsIndex>::const_iterator it = v->sets_indizes.begin();
		it != v->sets_indizes.end(); ++it) {
		const eix::ptr_list<KeywordMask> *keyword_masks = m_keywords.get(m_settings->set_names[*it]);
		if(!keyword_masks)
			continue;
		for(eix::ptr_list<KeywordMask>::const_iterator i = keyword_masks->begin();
			i != keyword_masks->end(); ++i) {
			push_backs(result, split_string(i->keywords));
		}
	}
}

/// Set stability according to arch or local ACCEPT_KEYWORDS
void
PortageSettings::setKeyflags(Package *pkg, bool use_accepted_keywords) const
{
	const set<string> *accept_set;
	Version::SavedKeyIndex ind;
	if(use_accepted_keywords) {
		ind = Version::SAVEKEY_ACCEPT;
		accept_set = &m_accepted_keywords_set;
	}
	else {
		ind = Version::SAVEKEY_ARCH;
		accept_set = &m_arch_set;
	}
	if(pkg->restore_keyflags(ind))
		return;
	for(Package::iterator t = pkg->begin(); t != pkg->end(); ++t) {
		t->set_keyflags(*accept_set, m_obsolete_minusasterisk);
		t->save_keyflags(ind);
	}
}

void
PortageUserConfig::setProfileMasks(Package *p) const
{
	if(p->restore_maskflags(Version::SAVEMASK_USERPROFILE))
		return;
	if(profile)
		profile->applyMasks(p);
	else
		m_settings->setMasks(p);
	p->save_maskflags(Version::SAVEMASK_USERPROFILE);
}

/// @return true if something from /etc/portage/package.* applied and check involves masks
bool
PortageUserConfig::setMasks(Package *p, Keywords::Redundant check, bool file_mask_is_profile) const
{
	Version::SavedMaskIndex ind = file_mask_is_profile ?
		Version::SAVEMASK_USERFILE : Version::SAVEMASK_USER;
	if((check & Keywords::RED_ALL_KEYWORDS) == Keywords::RED_NOTHING)
	{
		if(p->restore_maskflags(ind))
			return false;
	}
	if(file_mask_is_profile) {
		if(!(p->restore_maskflags(Version::SAVEMASK_FILE))) {
			throw ExBasic("internal error: Tried to restore nonlocal mask without saving");
		}
	}
	else
		setProfileMasks(p);
	bool rvalue = m_localmasks.applyMasks(p, check);
	m_settings->finalize(p);
	p->save_maskflags(ind);
	return rvalue;
}

void
PortageSettings::setMasks(Package *p, bool filemask_is_profile) const
{
	if(filemask_is_profile) {
		if(!(p->restore_maskflags(Version::SAVEMASK_FILE))) {
			throw ExBasic("internal error: Tried to restore nonlocal mask without saving");
		}
		return;
	}
	if(p->restore_maskflags(Version::SAVEMASK_PROFILE))
		return;
	profile->applyMasks(p);
	p->save_maskflags(Version::SAVEMASK_PROFILE);
}
