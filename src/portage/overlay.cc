// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "overlay.h"

#include <eixTk/filenames.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <eixTk/stringutils.h>
#include <eixTk/utils.h>

#include <map>
#include <string>
#include <vector>

using namespace std;

static map<string, string> path_label_hash;

OverlayIdent::OverlayIdent(const char *Path, const char *Label)
{
	if(Path == NULLPTR) {
		path.clear();
		know_path = false;
	}
	else {
		path = Path;
		know_path = true;
	}
	if(Label == NULLPTR) {
		label.clear();
		know_label = false;
		return;
	}
	label = Label;
	know_label = true;
}

void
OverlayIdent::readLabel_internal(const char *Path)
{
	know_label = true;
	vector<string> lines;
	string my_path;
	if(Path == NULLPTR) {
		if(!know_path) {
			label.clear();
			return;
		}
		my_path = path;
	}
	else {
		my_path = Path;
	}
	map<string, string>::const_iterator f(path_label_hash.find(my_path));
	if(f != path_label_hash.end()) {
		label = f->second;
		return;
	}
	pushback_lines((my_path + "/profiles/repo_name").c_str(), &lines, true, false, false);
	label.clear();
	for(vector<string>::const_iterator i(lines.begin()); likely(i != lines.end()); ++i) {
		if(i->empty()) {
			continue;
		}
		label = *i;
		break;
	}
	path_label_hash[my_path] = label;
}

string
OverlayIdent::human_readable() const
{
	if(label.empty())
		return path;
	return string("\"") + label + "\" " + path;
}

const char *
RepoList::get_path(const string &label)
{
	map<string, string>::iterator f(cache.find(label));
	if(likely(f != cache.end())) {
		return f->second.c_str();
	}
	if(trust_cache) {
		return NULLPTR;
	}
	for(iterator it(begin()); likely(it != end()); ++it) {
		if(unlikely(!it->know_path)) {
			continue;
		}
		it->readLabel();
		if(unlikely(!it->know_label)) {
			continue;
		}
		const string &l(it->label);
		const string &p(it->path);
		cache[l] = p;
		if(unlikely(l == label)) {
			return p.c_str();
		}
		if(unlikely(it == begin())) {
			cache[emptystring] = p;
			if(label.empty()) { // The empty label matches the main tree
				return p.c_str();
			}
		}
	}
	trust_cache = true;
	return NULLPTR;
}

RepoList::iterator
RepoList::find_filenames(const char *search, bool list_of_patterns, bool resolve_list)
{
	for(iterator ov(begin()); likely(ov != end()); ++ov) {
		if(likely(ov->know_path)) {
			if(unlikely(same_filenames(ov->path.c_str(), search, list_of_patterns, resolve_list))) {
				return ov;
			}
		}
	}
	return end();
}
