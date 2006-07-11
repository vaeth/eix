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

/** Return vector of all possible all categories.
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
		// I don't know why this does not work:
		// unique(m_categories.begin(), m_categories.end());
		// So we implement it manually:
		vector<string> copy;
		const char *previous=NULL;
		for(vector<string>::iterator i = m_categories.begin();
			i!=m_categories.end(); ++i)
		{
			if(previous)
				if(strcmp(previous, i->c_str()) == 0)
					continue;
			copy.push_back(*i);
			previous=i->c_str();
		}
		m_categories=copy;
	}
	return &m_categories;
}

/** Read maskings & unmaskings as well as user-defined ones */
MaskList<Mask> *PortageSettings::getMasks()
{
	if(m_masks.empty()) {
		if(!grab_masks(string((*this)["PORTDIR"]+"profiles/package.mask").c_str(), Mask::maskMask, &m_masks) )
			WARNING("Can't read %sprofiles/package.mask\n", (*this)["PORTDIR"].c_str());
	}
	return &(m_masks);
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

	for(unsigned int i = 0;
		i<lines.size();
		i++)
	{
		if(lines[i].size() == 0)
		{
			continue;
		}

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

void PortageUserConfig::setMasks(Package *p) {
	/* Set hardmasks */
	m_mask.applyMasks(p);
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

void
PortageUserConfig::setStability(Package *p, Keywords kw)
{
	const eix::ptr_list<KeywordMask> *keyword_masks = m_keywords.get(p);
	map<Version*,string> sorted_by_versions;

	if(keyword_masks != NULL && keyword_masks->empty() == false)
	{
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
		Keywords lkw(kw.get());

		string skw = sorted_by_versions[*i];
		if(skw.empty() == false)
		{
			vector<string> kv = split_string(skw);
			kv.erase(unique(kv.begin(), kv.end()), kv.end());

			for(vector<string>::iterator kvi = kv.begin();
				kvi != kv.end();
				++kvi)
			{
				if(*kvi == "-" + arch) {
					lkw &= (~Keywords::KEY_STABLE | ~Keywords::KEY_ALL);
				}
				else if(*kvi == "~" + arch) {
					lkw |= Keywords::KEY_UNSTABLE;
				}
				else if(*kvi == "-~" + arch) {
					lkw &= (~Keywords::KEY_UNSTABLE | ~Keywords::KEY_ALL);
				}
				else if(*kvi == "-*") {
					lkw |= Keywords::KEY_MINUSASTERISK;
				}
			}

		}
		apply_keywords(**i, lkw.get());
	}
}

void
PortageSettings::setStability(Package *pkg, Keywords &kw)
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
}
