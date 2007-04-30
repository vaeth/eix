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

#include "config.h"

#include "portagesettings.h"

#include <portage/mask.h>
#include <portage/package.h>
#include <portage/version.h>

#include <eixTk/utils.h>
#include <eixTk/stringutils.h>
#include <eixTk/filenames.h>

#include <varsreader.h>

#include <fstream>
#include <fnmatch.h>


using namespace std;

bool grab_masks(const char *file, Mask::Type type, MaskList<Mask> *cat_map, vector<Mask*> *mask_vec, bool recursive)
{
	vector<string> lines;
	if( ! pushback_lines(file, &lines, true, recursive))
		return false;
	for(vector<string>::iterator it=lines.begin(); it<lines.end(); ++it)
	{
		string line=*it;
		try {
			Mask *m = new Mask(line.c_str(), type);
			OOM_ASSERT(m);
			if(cat_map) {
				cat_map->add(m);
			}
			else {
				mask_vec->push_back(m);
			}
		}
		catch(ExBasic e) {
			cerr << "-- Invalid line in " << file << ": \"" << line << "\"" << endl
			     << "   " << e.getMessage() << endl;
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
	NULL
};

/** Environment variables which should add/override all other settings. */
static const char *test_in_env_late[] = {
	"PORTDIR",
	"PORTDIR_OVERLAY",
	"USE",
	"CONFIG_PROTECT",
	"CONFIG_PROTECT_MASK",
	"FEATURES",
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
PortageSettings::PortageSettings(const string &eprefixconf, const string &eprefixprofile, const string &eprefixportdir, const string &eprefixoverlays, const string &eprefixaccessoverlays, const string &eprefixsource, bool obsolete_minusasterisk)
{
	m_obsolete_minusasterisk = obsolete_minusasterisk;
	m_eprefixconf     = eprefixconf;
	m_eprefixprofile  = eprefixprofile;
	m_eprefixportdir  = eprefixportdir;
	m_eprefixoverlays = eprefixoverlays;
	m_eprefixaccessoverlays = eprefixaccessoverlays;

	read_config(m_eprefixconf + MAKE_GLOBALS_FILE, eprefixsource);
	read_config(m_eprefixconf + MAKE_CONF_FILE, eprefixsource);
	override_by_env(test_in_env_early);
	profile     = new CascadingProfile(this);
	user_config = new PortageUserConfig(this);
	override_by_env(test_in_env_late);

	/* Normalize "PORTDIR": */
	{
		string &ref = (*this)["PORTDIR"];
		string full = m_eprefixportdir;
		if(ref.empty())
			full.append("/usr/portage");
		else
			full.append(ref);
		ref = normalize_path(full.c_str(), true);
		if(ref[ref.size() - 1] != '/')
			ref.append("/");
	}

	/* Normalize overlays and erase duplicates */
	{
		string &ref = (*this)["PORTDIR_OVERLAY"];
		vector<string> overlayvec = split_string(ref);
		add_overlay_vector(overlayvec, true, true);
		ref = join_vector(overlayvec);
	}

	m_accepted_keyword = split_string((*this)["ACCEPT_KEYWORDS"]);
	m_accepted_keyword = resolve_plus_minus(m_accepted_keyword);
	(*this)["ACCEPT_KEYWORDS"] = join_vector(m_accepted_keyword);
	m_accepted_keywords.set((*this)["ARCH"], (*this)["ACCEPT_KEYWORDS"]);
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

/** Read maskings & unmaskings as well as user-defined ones */
MaskList<Mask> *PortageSettings::getMasks()
{
	if(m_masks.empty()) {
		if(!grab_masks(((*this)["PORTDIR"] + PORTDIR_MASK_FILE).c_str(), Mask::maskMask, &m_masks) )
			WARNING("Can't read %sprofiles/package.mask\n", (*this)["PORTDIR"].c_str());
		for(vector<string>::iterator i = overlays.begin();
			i != overlays.end();
			++i)
		{
			grab_masks((m_eprefixaccessoverlays + (*i) + "/" + PORTDIR_MASK_FILE).c_str(), Mask::maskMask, &m_masks);
		}
	}
	return &(m_masks);
}

bool
PortageUserConfig::readMasks()
{
	bool mask_ok = grab_masks(((m_settings->m_eprefixconf) + USER_MASK_FILE).c_str(), Mask::maskMask, &m_mask, true);
	bool unmask_ok = grab_masks(((m_settings->m_eprefixconf) + USER_UNMASK_FILE).c_str(), Mask::maskUnmask, &m_mask, true);
	return mask_ok && unmask_ok;
}

void
PortageUserConfig::ReadVersionFile (const char *file, MaskList<KeywordMask> *list)
{
	vector<string> lines;
	pushback_lines(file, &lines, false, true);
	for(vector<string>::size_type i = 0;
		i<lines.size();
		i++)
	{
		if(lines[i].empty())
			continue;
		try {
			KeywordMask *m = NULL;
			string::size_type n = lines[i].find_first_of("\t ");
			if(n == string::npos) {
				m = new KeywordMask(lines[i].c_str());
			}
			else {
				m = new KeywordMask(lines[i].substr(0, n).c_str());
				if(m)
					m->keywords = "1"; //lines[i].substr(n + 1);
			}
			if(m)
				list->add(m);
		}
		catch(ExBasic e) { }
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
} KeywordsFlag;

bool PortageUserConfig::readKeywords() {
	/* Prepend a ~ to every token.
	 * FIXME: So do we only care for ARCH or do we also care for ACCEPT_KEYWORDS? */
	vector<string> splitted = split_string((*m_settings)["ARCH"], "\t \n\r");
	for(vector<string>::iterator it = splitted.begin(); it != splitted.end(); ++it) {
		if(strchr("-~", (*it)[0]) == NULL) {
			*it = "~" + *it;
		}
	}
	string fscked_arch = join_vector(splitted);

	vector<string> lines;
	string filename((m_settings->m_eprefixconf) + USER_KEYWORDS_FILE);

	pushback_lines(filename.c_str(), &lines, false, true);

	/* Read only the last line for each "first" entry, e.g. in the example
		foo/bar 1
		foo/bar 2
		=foo/bar-1 3
		=foo/bar-1 4
	   the line 1 and 3 are ignored but 2 and 4 are both put to keywords
	   (even if they should influence each other).
	   This is strange, but this is the way portage does it.

	   We read in two passes, first creating the actual list in a map
	   (and remember BTW which were doubled) and then we push the map
	   in the original order to m_keywords */

	map<string, KeywordsFlag> have;
	for(vector<string>::size_type i = 0; i < lines.size(); ++i)
	{
		if(lines[i].empty())
			continue;

		string::size_type n = lines[i].find_first_of("\t ");
		string name, content;
		if(n == string::npos) {
			name = lines[i];
			content = fscked_arch;
		}
		else {
			name = lines[i].substr(0, n);
			content = lines[i].substr(n + 1);
		}
		lines[i] = name;
		map<string, KeywordsFlag>::iterator old = have.find(name);
		if(old == have.end()) {
			KeywordsFlag *f = &(have[name]);
			f->locally_double = false;
			f->keywords = content;
		}
		else {
			(old->second).locally_double = true;
			(old->second).keywords = content;
		}
	}

	for(vector<string>::size_type i = 0; i != lines.size(); ++i)
	{
		if(lines[i].empty())
			continue;
		try {
			KeywordMask *m = new KeywordMask(lines[i].c_str());
			if(m) {
				KeywordsFlag *f = &(have[lines[i]]);
				m->keywords       = f->keywords;
				m->locally_double = f->locally_double;
				m_keywords.add(m);
			}
		}
		catch(ExBasic e) {
			portage_parse_error(filename.c_str(), i, lines[i], e);
		}
	}
	return true;
}

/// @return true if something from /etc/portage/package.* applied
bool PortageUserConfig::setMasks(Package *p, Keywords::Redundant check) const {
	/* Set hardmasks */
	return m_mask.applyMasks(p, check);
}

inline void apply_keywords(Version &v, Keywords::Type t, bool alwaysstable)
{
	if(alwaysstable || (v.get() & t)) {
		v |= Keywords::KEY_STABLE;
	}
	else {
		v &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
	}
}

#define set_arch_used(value) do { \
	if(arch_used < value) { \
		if(arch_used != ARCH_NOTHING) \
			redundant |= (check & Keywords::RED_MIXED); \
		arch_used = value; \
	} else \
		redundant |= (check & Keywords::RED_MIXED); \
	} while(0)

// Return value is true if -* occurs in keywords (and new_minusasterisk is true)
inline bool add_local_keywords(vector<string> &collect, const string &keywords, bool new_minusasterisk)
{
	bool had_minus = false;
	const vector<string> kv = split_string(keywords);
	for(vector<string>::const_iterator it = kv.begin();
		it != kv.end(); ++it)
	{
		// "-*" possibly deletes all previous keywords set by user.
		if(new_minusasterisk) {
			if(*it == "-*") {
				had_minus = true;
				collect.clear();
				continue;
			}
		}
		collect.push_back(*it);
	}
	return had_minus;
}

/// @return true if something from /etc/portage/package.* applied
bool
PortageUserConfig::setStability(Package *p, const Keywords &kw, Keywords::Redundant check) const
{
	const eix::ptr_list<KeywordMask> *keyword_masks = m_keywords.get(p);
	map<Version*,vector<string> > sorted_by_versions;
	bool rvalue = false;

	bool new_minusasterisk = !(m_settings->m_obsolete_minusasterisk);
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
				if(add_local_keywords(sorted_by_versions[*v], it->keywords, new_minusasterisk)) {
					if(check & Keywords::RED_MINUSASTERISK)
						v->set_redundant((v->get_redundant()) |
							Keywords::RED_MINUSASTERISK);
				}
				// Set RED_DOUBLE_LINE depending on locally_double
				if(it->locally_double) {
					if(check & Keywords::RED_DOUBLE_LINE)
						v->set_redundant((v->get_redundant()) |
							Keywords::RED_DOUBLE_LINE);
				}
			}
		}
	}

	string arch = (*m_settings)["ARCH"];

	for(Package::iterator i = p->begin();
		i != p->end();
		++i)
	{
		const char ARCH_NOTHING        = 0,
		           ARCH_TESTING        = 1,
		           ARCH_ALIENSTABLE    = 2,
		           ARCH_ALIENUNSTABLE  = 3,
		           ARCH_MISSINGKEYWORD = 4;
		char arch_needed;
		char arch_used = ARCH_NOTHING;
		Keywords::Redundant redundant = i->get_redundant();
		Keywords::Type oritype = i->get();
		if(oritype & Keywords::KEY_STABLE)
			arch_needed = ARCH_NOTHING;
		else if(oritype & Keywords::KEY_UNSTABLE)
			arch_needed = ARCH_TESTING;
		else if((oritype & Keywords::KEY_ALIENSTABLE) ||
			(oritype & Keywords::KEY_MINUSKEYWORD))
			arch_needed = ARCH_ALIENSTABLE;
		else if(oritype & Keywords::KEY_ALIENUNSTABLE)
			arch_needed = ARCH_ALIENUNSTABLE;
		else
			arch_needed = ARCH_MISSINGKEYWORD;

		bool alwaysstable = false;

		Keywords lkw(kw.get());

		vector<string> &kv = sorted_by_versions[*i];
		if(!kv.empty())
		{
			if(check & Keywords::RED_IN_KEYWORDS)
				redundant |= Keywords::RED_IN_KEYWORDS;
			vector<string> *arr = NULL;
			for(vector<string>::iterator kvi = kv.begin();
				kvi != kv.end();
				++kvi)
			{
				if(*kvi == arch) {
					set_arch_used(ARCH_NOTHING);
					lkw |= Keywords::KEY_STABLE;
					continue;
				}
				if(*kvi == "~" + arch) {
					set_arch_used(ARCH_TESTING);
					lkw |= Keywords::KEY_UNSTABLE;
					continue;
				}
				if(*kvi == "*") {
					set_arch_used(ARCH_ALIENSTABLE);
					lkw |= Keywords::KEY_ALIENSTABLE;
					continue;
				}
				if(*kvi == "~*") {
					set_arch_used(ARCH_ALIENUNSTABLE);
					lkw |= Keywords::KEY_ALIENUNSTABLE;
					continue;
				}
				if(*kvi == "**") {
					set_arch_used(ARCH_MISSINGKEYWORD);
					alwaysstable = true;
					continue;
				}
				if(*kvi == "-*") {
					set_arch_used(ARCH_MISSINGKEYWORD);
					lkw |= Keywords::KEY_MINUSASTERISK;
					continue;
				}
				if(*kvi == "-" + arch) {
					lkw &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
					// The -ARCH is here to *allow* installations:
					if(oritype & Keywords::KEY_MINUSKEYWORD) {
						set_arch_used(ARCH_ALIENSTABLE);
						lkw |= Keywords::KEY_MINUSKEYWORD;
					}
					continue;
				}
				if(*kvi == "-~" + arch) {
					lkw &= (~Keywords::KEY_UNSTABLE | ~Keywords::KEY_ALL);
					// No continue! (might match strange keyword)
				}
				// match alien or strange keywords:
				const char *s = kvi->c_str();
				if(s[0] == '-')	{
					redundant |= (check & Keywords::RED_STRANGE);
				}
				if(!arr) {
					arr = new vector<string>;
					*arr = split_string((*i)->get_full_keywords());
				}
				if(find(arr->begin(), arr->end(), s) != arr->end())
				{
					alwaysstable = true;
					if(s[0] == '-') { // Strange match
						set_arch_used(ARCH_ALIENUNSTABLE);
						redundant |= (check & Keywords::RED_STRANGE);
					}
					else if(s[0] != '~') { // ALIEN match
						set_arch_used(ARCH_ALIENSTABLE);
					}
					else { // ~ALIEN match
						set_arch_used(ARCH_ALIENUNSTABLE);
						// We do not consider * weaker than ~ALIEN.
						// Therefore we should set arch_needed = ARCH_ALIENUNSTABLE
						//    to avoid redundancy tagging.
						// However, ALIEN is weaker than ~ALIEN, so
						// we do this only if ALIEN is not stable:
						if((arch_needed == ARCH_ALIENSTABLE) &&
							(check & (~redundant) & Keywords::RED_WEAKER))
						{
							if(find(arr->begin(), arr->end(), s + 1) == arr->end())
								arch_needed = ARCH_ALIENUNSTABLE;
						}
					}
				}
				else if(check & (Keywords::RED_STRANGE | Keywords::RED_MIXED))
				{
					if(s[0] == '-') {
						redundant |= (check & Keywords::RED_STRANGE);
					}
					else if(s[0] != '~') {
						// an non-matching ALIEN keyword need not necessarily be "strange":
						// it could also simply be too weak because ~ALIEN exists.
						string testing = "~" + *kvi;
						if(find(arr->begin(), arr->end(), testing) != arr->end()) {
							set_arch_used(ARCH_ALIENSTABLE);
						}
						else
							redundant |= (check & Keywords::RED_STRANGE);
					}
					else {
						// an unknown ~ALIEN keyword need not necessarily be "strange":
						// it could also simply be too strong because ALIEN exists.
						if(find(arr->begin(), arr->end(), s + 1) != arr->end()) {
							set_arch_used(ARCH_ALIENUNSTABLE);
						}
						else
							redundant |= (check & Keywords::RED_STRANGE);
					}
				}
			}
			if(arr)
				delete arr;
			if(arch_used > arch_needed)
				redundant |= (check & Keywords::RED_WEAKER);
			if(check & Keywords::RED_DOUBLE) {
				if(sort_uniquify(kv, true))
					redundant |= Keywords::RED_DOUBLE;
			}
		}
		apply_keywords(**i, lkw.get(), alwaysstable);
		if(rvalue && (check & Keywords::RED_NO_CHANGE)) {
			if((i->get() & Keywords::KEY_ALL) == (oritype & Keywords::KEY_ALL))
				redundant |= Keywords::RED_NO_CHANGE;
		}
		if(redundant)
			i->set_redundant(redundant);
	}
	return rvalue;
}

void
PortageSettings::setStability(Package *pkg, const Keywords &kw, bool save_after_setting) const
{
	Package::iterator t = pkg->begin();
	for(; t != pkg->end(); ++t) {
		if(t->get() & kw.get())
		{
			**t |= Keywords::KEY_STABLE;
		}
		else {
			**t &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
		}
	}
	if(save_after_setting)
		pkg->save_maskstuff();
}
