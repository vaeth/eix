// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/conf/cascadingprofile.h"
#include <config.h>

#include <cstring>

#include <string>
#include <utility>

#include "eixTk/assert.h"
#include "eixTk/attribute.h"
#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/filenames.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/sysutils.h"
#include "eixTk/unordered_map.h"
#include "eixTk/utils.h"
#include "portage/conf/portagesettings.h"
#include "portage/mask.h"
#include "portage/mask_list.h"

/* Path to symlink to profile */
#define PROFILE_LINK1 "/etc/make.profile"
#define PROFILE_LINK2 "/etc/portage/make.profile"

using std::pair;
using std::string;

class ParseError;

/**
Exclude this files from listing of files in profile
**/
static CONSTEXPR const char *const profile_exclude[] = { "parent", "..", "." , NULLPTR };

/**
Add all files from profile and its parents to m_profile_files
**/
bool CascadingProfile::addProfile(const char *profile, WordUnorderedSet *sourced_files) {
	string truename(normalize_path(profile, true, true));
	if(unlikely(print_profile_paths)) {
		if(likely(is_dir(truename.c_str()))) {
			eix::print() % truename;
			if(profile_paths_append.empty()) {
				eix::print('\0');
			} else {
				eix::print() % profile_paths_append;
			}
		}
	}
	if(truename.empty()) {
		return false;
	}
	bool topcall(sourced_files == NULLPTR);
	if(unlikely(topcall)) {
		sourced_files = new WordUnorderedSet;
	} else if(sourced_files->count(truename) != 0) {
		eix::say_error(_("Recursion in cascading profile"));
		return false;
	}
	sourced_files->insert(truename);
	WordVec parents;
	string currfile(truename);
	currfile.append("parent");
	// Use pushback_lines to avoid keeping file descriptor open:
	// Who knows what's our limit of open file descriptors.
	if(pushback_lines(currfile.c_str(), &parents)) {
		for(WordVec::const_iterator it(parents.begin());
			likely(it != parents.end()); ++it) {
			if(it->empty()) {
				continue;
			}
			if((*it)[0] == '/') {
				addProfile(it->c_str(), sourced_files);
				continue;
			}
			string::size_type colon(it->find(':'));
			if(colon == string::npos) {
				addProfile((truename + (*it)).c_str(), sourced_files);
				continue;
			}
			const char *path;
			RepoList repos(m_portagesettings->repos);
			if(colon == 0) {
				RepoList::iterator f(repos.find_filename(truename.c_str(), true));
				path = ((f != repos.end()) ? f->path.c_str() : NULLPTR);
			} else {
				string repo(it->substr(0, colon));
				path = repos.get_path(repo);
			}
			if(path == NULLPTR) {
				eix::say_error(_("warning: ignoring parent %s of file %s"))
					% (*it) % currfile;
				continue;
			}
			addProfile((string(path) + "/profiles/" + it->substr(colon + 1)).c_str(), sourced_files);
		}
	}
	if(unlikely(topcall)) {
		delete sourced_files;
	} else {
		sourced_files->erase(truename);
	}
	WordVec filenames;
	bool r(pushback_files(truename, &filenames, profile_exclude, 3));
	for(WordVec::const_iterator it(filenames.begin());
		likely(it != filenames.end()); ++it) {
		listaddFile(*it, 0, false);
	}
	return r;
}

class ProfileFilenames {
		typedef CascadingProfile::Handler Handler;
		typedef UNORDERED_MAP<string, Handler> NameMap;
		NameMap name_map;

	public:
		void initstring(const std::string& s, Handler h) {
			name_map.insert(pair<string, Handler>(s, h));
		}

		ATTRIBUTE_PURE Handler operator[](const std::string s) const {
			NameMap::const_iterator it(name_map.find(s));
			if(it != name_map.end()) {
				return it->second;
			}
			return NULLPTR;
		}

		ProfileFilenames() {
			initstring("packages",   &CascadingProfile::readPackages);
			initstring("packages.d", &CascadingProfile::readPackages);
			initstring("package.mask",   &CascadingProfile::readPackageMasks);
			initstring("package.mask.d", &CascadingProfile::readPackageMasks);
			initstring("package.unmask",   &CascadingProfile::readPackageUnmasks);
			initstring("package.unmask.d", &CascadingProfile::readPackageUnmasks);
			initstring("package.keywords",   &CascadingProfile::readPackageKeywords);
			initstring("package.keywords.d", &CascadingProfile::readPackageKeywords);
			initstring("package.accept_keywords",   &CascadingProfile::readPackageAcceptKeywords);
			initstring("package.accept_keywords.d", &CascadingProfile::readPackageAcceptKeywords);
		}
};

static ProfileFilenames *profile_filenames = NULLPTR;

void CascadingProfile::init_static() {
	eix_assert_static(profile_filenames == NULLPTR);
	profile_filenames = new ProfileFilenames;
}

bool CascadingProfile::readremoveFiles() {
	eix_assert_static(profile_filenames != NULLPTR);
	bool ret(false);
	for(ProfileFiles::iterator file(m_profile_files.begin());
		likely(file != m_profile_files.end()); ++file) {
		const char *filename(std::strrchr(file->c_str(), '/'));
		if(filename == NULLPTR) {
			continue;
		}
		++filename;
		CascadingProfile::Handler handler((*profile_filenames)[filename]);
		if(handler == NULLPTR) {
			continue;
		}
		OverlayIdent& overlay(m_portagesettings->repos[file->repo_num()]);
		overlay.readLabel();
		if((this->*handler)(file->name(),
			(overlay.label.empty() ? NULLPTR : overlay.label.c_str()),
			file->only_repo())) {
			ret = true;
		}
	}
	m_profile_files.clear();
	return ret;
}

bool CascadingProfile::readPackages(const string& filename, const char *repo, bool only_repo) {
	LineVec lines;
	pushback_lines(filename.c_str(), &lines, true);
	bool ret(false);
	PreList::FilenameIndex file_system(p_system.push_name(filename, repo, only_repo));
	PreList::FilenameIndex file_profile(p_profile.push_name(filename, repo, only_repo));
	PreList::LineNumber number(1);
	for(LineVec::const_iterator it(lines.begin());
		likely(it != lines.end()); ++number, ++it) {
		if(it->empty()) {
			continue;
		}
		/* lines beginning with '*' are m_system-packages
		 * all others are masked by profile .. if they don't match :) */
		const char *p(it->c_str());
		bool remove(*p == '-');
		if(unlikely(remove)) {
			++p;
		}
		if(*p == '*') {
			if(unlikely(remove)) {
				if(unlikely(p[1] == '\0')) {
					ret |= (p_system.remove_all() || p_profile.remove_all());
				} else {
					ret |= p_system.remove_line(p);
				}
			} else {
				ret |= p_system.add_line(p, file_system, number, false);
			}
		} else if(unlikely(remove)) {
			ret |= p_profile.remove_line(p);
		} else {
			ret |= p_profile.add_line(p, file_profile, number, false);
		}
	}
	return ret;
}

bool CascadingProfile::readPackageMasks(const string& filename, const char *repo, bool only_repo) {
	LineVec lines;
	pushback_lines(filename.c_str(), &lines, true, true, -1);
	return p_package_masks.handle_file(lines, filename, repo, false, true, only_repo);
}

bool CascadingProfile::readPackageUnmasks(const string& filename, const char *repo, bool only_repo) {
	LineVec lines;
	pushback_lines(filename.c_str(), &lines, true);
	return p_package_unmasks.handle_file(lines, filename, repo, false, false, only_repo);
}

bool CascadingProfile::readPackageKeywords(const string& filename, const char *repo, bool only_repo) {
	LineVec lines;
	pushback_lines(filename.c_str(), &lines, true);
	return p_package_keywords.handle_file(lines, filename, repo, false, false, only_repo);
}

bool CascadingProfile::readPackageAcceptKeywords(const string& filename, const char *repo, bool only_repo) {
	LineVec lines;
	pushback_lines(filename.c_str(), &lines, true);
	return p_package_accept_keywords.handle_file(lines, filename, repo, true, false, only_repo);
}

/**
Read all "make.defaults" files found in profile
**/
void CascadingProfile::readMakeDefaults() {
	for(WordVec::size_type i(0); likely(i < m_profile_files.size()); ++i) {
		if(unlikely(std::strcmp(std::strrchr(m_profile_files[i].c_str(), '/'), "/make.defaults") == 0)) {
			m_portagesettings->read_config(m_profile_files[i].name(), "");
		}
	}
}

/**
Populate MaskLists from PreLists.
All files must have been read and m_raised_arch
must be known when this is called.
**/
void CascadingProfile::finalize() {
	if(finalized) {
		return;
	}
	finalized = true;
	const ParseError *parse_error(m_portagesettings->parse_error);
	p_system.initialize(&m_system, Mask::maskInSystem, parse_error);
	p_profile.initialize(&m_profile, Mask::maskInProfile, parse_error);
	p_package_masks.initialize(&m_package_masks, Mask::maskMask, true, parse_error);
	p_package_unmasks.initialize(&m_package_unmasks, Mask::maskUnmask, parse_error);
	p_package_keywords.initialize(&m_package_keywords, parse_error);
	p_package_accept_keywords.initialize(&m_package_accept_keywords, m_portagesettings->m_raised_arch, parse_error);
}

/**
Cycle through profile and put path to files into this->m_profile_files.
**/
void CascadingProfile::listaddProfile(const char *profile_dir) {
	if(profile_dir) {
		addProfile(profile_dir);
		return;
	}
	const string& s((*m_portagesettings)["PORTAGE_PROFILE"]);
	if(unlikely(!s.empty())) {
		if(addProfile(s.c_str())) {
			return;
		}
	}
	if(addProfile(((m_portagesettings->m_eprefixconf) + PROFILE_LINK1).c_str())) {
		return;
	}
	addProfile(((m_portagesettings->m_eprefixconf) + PROFILE_LINK2).c_str());
}

void CascadingProfile::applyMasks(Package *p) const {
	if(m_init_world || use_world) {
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			(*it)->maskflags.set(MaskFlags::MASK_NONE);
		}
	} else {
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
GCC_DIAG_OFF(sign-conversion)
			(*it)->maskflags.clearbits(~MaskFlags::MASK_WORLD);
GCC_DIAG_ON(sign-conversion)
		}
	}
	m_profile.applyMasks(p);
	m_system.applyMasks(p);
	m_package_masks.applyMasks(p);
	m_package_unmasks.applyMasks(p);
	if(use_world) {
		m_world.applyMasks(p);
	}
	// Now we must also apply the set-items of the above lists:
	for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
		if(v->sets_indizes.empty()) {
			// Shortcut for the most frequent case
			continue;
		}
		for(Version::SetsIndizes::const_iterator it(v->sets_indizes.begin());
			unlikely(it != v->sets_indizes.end()); ++it) {
			const string& set_name(m_portagesettings->set_names[*it]);
			m_profile.applySetMasks(*v, set_name);
			m_system.applySetMasks(*v, set_name);
			m_package_masks.applySetMasks(*v, set_name);
			m_package_unmasks.applySetMasks(*v, set_name);
			if(use_world) {
				m_world.applySetMasks(*v, set_name);
			}
		}
	}
	m_portagesettings->finalize(p);
}

void CascadingProfile::applyKeywords(Package *p) const {
	for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
		it->reset_accepted_effective_keywords();
	}
	bool keywords_empty(m_package_keywords.empty());
	bool accept_empty(m_package_accept_keywords.empty());
	if(likely(keywords_empty && accept_empty)) {
		// For most profiles, we can take this shortcut:
		return;
	}
	if(!keywords_empty) {
		m_package_keywords.applyListItems(p);
	}
	if(!accept_empty) {
		m_package_accept_keywords.applyListItems(p);
	}
	// Now we must also apply the set-items of the lists:
	for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
		if(v->sets_indizes.empty()) {
			// Shortcut for the most frequent case
			continue;
		}
		for(Version::SetsIndizes::const_iterator it(v->sets_indizes.begin());
			unlikely(it != v->sets_indizes.end()); ++it) {
			const char *set_name(m_portagesettings->set_names[*it].c_str());
			m_package_keywords.applyListSetItems(*v, set_name);
			m_package_accept_keywords.applyListSetItems(*v, set_name);
		}
	}
}
