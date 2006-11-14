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

#include <varsreader.h>

#include <fstream>

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




/** Key that should accumelate their content rathern then replace. */
static const char *default_accumulating_keys[] = {
	"USE",
	"CONFIG_*",
	"FEATURES",
	"ACCEPT_KEYWORDS",
	NULL
};

/** Read make.globals and make.conf. */
PortageSettings::PortageSettings()
{
	profile     = new CascadingProfile(this);
	user_config = new PortageUserConfig(this);

	VarsReader make_globals(VarsReader::SUBST_VARS|VarsReader::INTO_MAP|VarsReader::APPEND_VALUES|VarsReader::ALLOW_SOURCE);
	make_globals.accumulatingKeys(default_accumulating_keys);
	make_globals.useMap(this);
	make_globals.read("/etc/make.globals");

	VarsReader make_conf(VarsReader::SUBST_VARS|VarsReader::INTO_MAP|VarsReader::APPEND_VALUES|VarsReader::ALLOW_SOURCE);
	make_conf.accumulatingKeys(default_accumulating_keys);
	make_conf.useMap(this);
	make_conf.read("/etc/make.conf");

	if((*this)["PORTDIR"].size() == 0 ) {
		(*this)["PORTDIR"] = "/usr/portage/";
	}
	else {
		(*this)["PORTDIR"].append("/");
	}

	overlays = split_string((*this)["PORTDIR_OVERLAY"]);

	if(getenv("ACCEPT_KEYWORDS")) {
		(*this)["ACCEPT_KEYWORDS"].append(string(" ") + getenv("ACCEPT_KEYWORDS"));
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
		pushback_lines(USER_CATEGORIES_FILE, &m_categories);
		pushback_lines(((*this)["PORTDIR"] + PORTDIR_CATEGORIES_FILE).c_str(), &m_categories);
		for(vector<string>::iterator i = overlays.begin();
			i != overlays.end();
			++i)
		{
			pushback_lines((*i + "/" + PORTDIR_CATEGORIES_FILE).c_str(),
			               &m_categories);
		}

		sort(m_categories.begin(), m_categories.end());
		m_categories.erase(
			unique(m_categories.begin(), m_categories.end()),
			m_categories.end());
	}
	return &m_categories;
}

/** Read maskings & unmaskings as well as user-defined ones */
MaskList<Mask> *PortageSettings::getMasks()
{
	if(m_masks.empty()) {
		if(!grab_masks(string((*this)["PORTDIR"] + PORTDIR_MASK_FILE).c_str(), Mask::maskMask, &m_masks) )
			WARNING("Can't read %sprofiles/package.mask\n", (*this)["PORTDIR"].c_str());
		for(vector<string>::iterator i = overlays.begin();
			i != overlays.end();
			++i)
		{
			grab_masks(string(*i + "/" + PORTDIR_MASK_FILE).c_str(), Mask::maskMask, &m_masks);
		}
	}
	return &(m_masks);
}

void PortageUserConfigReadVersionFile (const char *file, MaskList<KeywordMask> *list)
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
bool PortageUserConfigCheckList(Package *p, const MaskList<KeywordMask> *list, Keywords::Redundant flag_double, Keywords::Redundant flag_in)
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

bool PortageUserConfigCheckFile(Package *p, const char *file, MaskList<KeywordMask> *list, bool *readfile, Keywords::Redundant flag_double, Keywords::Redundant flag_in)
{
	if(!(*readfile))
	{
		PortageUserConfigReadVersionFile(file, list);
		*readfile = true;
	}
	return PortageUserConfigCheckList(p, list, flag_double, flag_in);
}


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
	pushback_lines("/etc/portage/package.keywords", &lines, false, true);

	for(vector<string>::size_type i = 0;
		i<lines.size();
		i++)
	{
		if(lines[i].empty())
			continue;

		try {
			string::size_type n = lines[i].find_first_of("\t ");
			if(n == string::npos) {
				KeywordMask *m = new KeywordMask(lines[i].c_str());
				m->keywords = fscked_arch;
				m_keywords.add(m);
			}
			else {
				KeywordMask *m = new KeywordMask(lines[i].substr(0, n).c_str());
				m->keywords = lines[i].substr(n + 1);
				m_keywords.add(m);
			}
		}
		catch(ExBasic e) {
			portage_parse_error("/etc/portage/package.keywords", i, lines[i], e);
		}
	}
	return true;
}

/// @return true if something from /etc/portage/package.* applied
bool PortageUserConfig::setMasks(Package *p, Keywords::Redundant check) const {
	/* Set hardmasks */
	return m_mask.applyMasks(p, check);
}

inline void apply_keywords(Version &v, Keywords::Type t)
{
	if(v.get() & t) {
		v |= Keywords::KEY_STABLE;
	}
	else {
		v &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
	}
}

#define set_arch_used(value) { \
	if(arch_used < value) { \
		if(arch_used != 0) \
			redundant |= (check & Keywords::RED_MIXED); \
		arch_used = value; \
	} else if(arch_used > value) \
		redundant |= (check & Keywords::RED_MIXED); }

/// @return true if something from /etc/portage/package.* applied
bool
PortageUserConfig::setStability(Package *p, const Keywords &kw, Keywords::Redundant check) const
{
	const eix::ptr_list<KeywordMask> *keyword_masks = m_keywords.get(p);
	map<Version*,string> sorted_by_versions;
	bool rvalue = false;

	if(keyword_masks != NULL && keyword_masks->empty() == false)
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
				sorted_by_versions[*v].append(" " + it->keywords);
			}
		}
	}

	string arch = (*m_settings)["ARCH"];

	for(Package::iterator i = p->begin();
		i != p->end();
		++i)
	{
		Keywords::Redundant redundant = i->get_redundant();
		char arch_needed;
		char arch_used = 0;
		Keywords::Type oritype = i->get();
		if(oritype & Keywords::KEY_STABLE)
			arch_needed = 0;
		else if(oritype & Keywords::KEY_UNSTABLE)
			arch_needed = 1;
		else
			arch_needed = 4;

		Keywords lkw(kw.get());

		string skw = sorted_by_versions[*i];
		if(!skw.empty())
		{
			if(check & Keywords::RED_IN_KEYWORDS)
				redundant |= Keywords::RED_IN_KEYWORDS;
			vector<string> kv = split_string(skw);
			vector<string>::iterator new_end = unique(kv.begin(), kv.end());
			if(new_end != kv.end())
			{
				kv.erase(new_end, kv.end());
				redundant |= (check & Keywords::RED_DOUBLE);
			}

			vector<string> *arr = NULL;
			for(vector<string>::iterator kvi = kv.begin();
				kvi != kv.end();
				++kvi)
			{
				if(*kvi == arch) {
					redundant |= (check & Keywords::RED_DOUBLE);
				}
				else if(*kvi == "-" + arch) {
					lkw &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
				}
				else if(*kvi == "~" + arch) {
					set_arch_used(1);
					lkw |= Keywords::KEY_UNSTABLE;
				}
				else if(*kvi == "-~" + arch) {
					lkw &= (~Keywords::KEY_UNSTABLE | ~Keywords::KEY_ALL);
				}
				else if(*kvi == "-*") {
					set_arch_used(4);
					lkw |= Keywords::KEY_MINUSASTERISK;
				}
				else { // match non-arch keywords:
					const char *s = kvi->c_str();
					if(s[0] == '-')
					{
						redundant |= (check & Keywords::RED_STRANGE);
					}
					else
					{
						if(!arr)
						{
							arr = new vector<string>;
							*arr = split_string((*i)->get_full_keywords());
						}
						if(find(arr->begin(), arr->end(), s) != arr->end())
						{
							lkw |= Keywords::KEY_STABLE;
							**i |= Keywords::KEY_STABLE;
							if(s[0] != '~')// OTHERARCH stable found
							{
								set_arch_used(2);
								if(arch_needed > 2)
									arch_needed = 2;
							}
							else// ~OTHERARCH found
							{
								set_arch_used(3);
								if(arch_needed > 3)
									arch_needed = 3;
								if((arch_needed == 3) &&
									(check & (~redundant) & Keywords::RED_WEAKER))
								{
									if(find(arr->begin(), arr->end(), s+1) != arr->end())
										arch_needed = 2;
								}
							}
						}
						else if(check & (Keywords::RED_STRANGE | Keywords::RED_MIXED))
						{
							// an unknown OTHERARCH keyword need not necessarily be "strange":
							// it could also simply be too weak because ~OTHERARCH exists
							if(s[0] != '~')
							{
								string testing="~" + *kvi;
								// Do not set arch_needed = 3, even if ~OTHERARCH is found here:
								// etc/portage/package.keywords might contain ~ANOTHERARCH
								if(find(arr->begin(), arr->end(), testing) != arr->end()) {
									set_arch_used(2);
								}
								else
									redundant |= (check & Keywords::RED_STRANGE);
							}
							else
								redundant |= (check & Keywords::RED_STRANGE);
						}
					}
				}
			}
			if(arch_used > arch_needed)
				redundant |= (check & Keywords::RED_WEAKER);
			if(check & Keywords::RED_DOUBLE)
			{
				sort(kv.begin(), kv.end());
				if(unique(kv.begin(), kv.end()) != kv.end())
					redundant |= Keywords::RED_DOUBLE;
			}
		}
		apply_keywords(**i, lkw.get());
		if(rvalue && (check & Keywords::RED_NO_CHANGE))
		{
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
		if(t->get() & kw.get()) {
			**t |= Keywords::KEY_STABLE;
		}
		else {
			**t &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
		}
	}
	if(save_after_setting)
		pkg->save_maskstuff();
}
