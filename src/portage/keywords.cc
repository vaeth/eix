// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "keywords.h"

#include <eixTk/likely.h>
#include <eixTk/stringutils.h>

#include <algorithm>
#include <set>
#include <string>
#include <vector>

using namespace std;

const MaskFlags::MaskType
	MaskFlags::MASK_NONE,
	MaskFlags::MASK_PACKAGE,
	MaskFlags::MASK_PROFILE,
	MaskFlags::MASK_HARD,
	MaskFlags::MASK_SYSTEM,
	MaskFlags::MASK_WORLD,
	MaskFlags::MASK_WORLD_SETS;

const KeywordsFlags::KeyType
	KeywordsFlags::KEY_EMPTY,
	KeywordsFlags::KEY_STABLE,
	KeywordsFlags::KEY_ARCHSTABLE,
	KeywordsFlags::KEY_ARCHUNSTABLE,
	KeywordsFlags::KEY_ALIENSTABLE,
	KeywordsFlags::KEY_ALIENUNSTABLE,
	KeywordsFlags::KEY_MINUSKEYWORD,
	KeywordsFlags::KEY_MINUSUNSTABLE,
	KeywordsFlags::KEY_MINUSASTERISK,
	KeywordsFlags::KEY_SOMESTABLE,
	KeywordsFlags::KEY_SOMEUNSTABLE,
	KeywordsFlags::KEY_TILDESTARMATCH;

KeywordsFlags::KeyType
KeywordsFlags::get_keyflags(const std::set<string> &accepted_keywords, const string &keywords, bool obsolete_minus)
{
	KeyType m(KEY_EMPTY);
	std::set<string> keywords_set;
	make_set<string>(keywords_set, split_string(keywords));
	for(std::set<string>::const_iterator it(keywords_set.begin());
		likely(it != keywords_set.end()); ++it) {
		bool found(false);
		if((*it)[0] == '-') {
			if(*it == "-*")
				m |= KEY_MINUSASTERISK;
			else if(*it == "-~*")
				m |= KEY_MINUSUNSTABLE;
			else if(accepted_keywords.find(it->substr(1)) != accepted_keywords.end())
				m |= KEY_MINUSKEYWORD;
			if(!obsolete_minus)
				continue;
		}
		if(accepted_keywords.find(*it) != accepted_keywords.end()) {
			found = true;
			m |= KEY_STABLE;
		}
		if((*it)[0] == '~') {
			if(found)
				m |= KEY_ARCHUNSTABLE;
			else {
				if(accepted_keywords.find(it->substr(1)) != accepted_keywords.end())
					m |= KEY_ARCHUNSTABLE;
				else
					m |= KEY_ALIENUNSTABLE;
			}
		}
		else {
			if((*it)[0] != '-')
				m |= found ? KEY_ARCHSTABLE : KEY_ALIENSTABLE;
		}
	}
	if(m & KEY_STABLE)
		return m;
	if(accepted_keywords.find("**") != accepted_keywords.end())
		return (m | KEY_STABLE);
	if(m & KEY_SOMESTABLE) {
		if(accepted_keywords.find("*") != accepted_keywords.end())
			return (m | KEY_STABLE);
	}
	if(m & KEY_TILDESTARMATCH) {
		if(accepted_keywords.find("~*") != accepted_keywords.end())
			return (m | KEY_STABLE);
	}
	return m;
}

const Keywords::Redundant
	Keywords::RED_NOTHING,
	Keywords::RED_DOUBLE,
	Keywords::RED_DOUBLE_LINE,
	Keywords::RED_MIXED,
	Keywords::RED_WEAKER,
	Keywords::RED_STRANGE,
	Keywords::RED_NO_CHANGE,
	Keywords::RED_MINUSASTERISK,
	Keywords::RED_IN_KEYWORDS,
	Keywords::RED_ALL_KEYWORDS,
	Keywords::RED_MASK,
	Keywords::RED_DOUBLE_MASK,
	Keywords::RED_IN_MASK,
	Keywords::RED_UNMASK,
	Keywords::RED_DOUBLE_UNMASK,
	Keywords::RED_IN_UNMASK,
	Keywords::RED_ALL_MASK,
	Keywords::RED_ALL_UNMASK,
	Keywords::RED_ALL_MASKSTUFF,
	Keywords::RED_DOUBLE_USE,
	Keywords::RED_IN_USE,
	Keywords::RED_ALL_USE,
	Keywords::RED_DOUBLE_ENV,
	Keywords::RED_IN_ENV,
	Keywords::RED_ALL_ENV,
	Keywords::RED_DOUBLE_LICENSE,
	Keywords::RED_IN_LICENSE,
	Keywords::RED_ALL_LICENSE,
	Keywords::RED_DOUBLE_CFLAGS,
	Keywords::RED_IN_CFLAGS,
	Keywords::RED_ALL_CFLAGS;

bool
Keywords::modify_keywords(string &result, const string &original, const string &modify_keys)
{
	bool modified(false);
	vector<string> modify;
	split_string(modify, modify_keys);
	if(modify.empty())
		return false;
	vector<string> words;
	split_string(words, original);
	for(vector<string>::const_iterator it(modify.begin());
		it != modify.end(); ++it) {
		if(it->empty())
			continue;
		if((*it)[0] == '-') {
			vector<string>::iterator f(find(words.begin(), words.end(), it->substr(1)));
			if(f != words.end()) {
				modified = true;
				words.erase(f);
			}
		}
		else {
			if(find(words.begin(), words.end(), *it) == words.end()) {
				modified = true;
				words.push_back(*it);
			}
		}
	}
	if(likely(!modified))
		return false;
	result.clear();
	join_to_string(result, words);
	return true;
}
