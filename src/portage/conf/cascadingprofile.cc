// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "cascadingprofile.h"
#ifdef DEBUG_PROFILE_PATHS
#include <eixTk/formated.h>
#endif
#include <eixTk/filenames.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <eixTk/utils.h>
#include <portage/conf/portagesettings.h>
#include <portage/mask.h>
#include <portage/mask_list.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <cstring>

/* Path to symlink to profile */
#define PROFILE_LINK1 "/etc/make.profile"
#define PROFILE_LINK2 "/etc/portage/make.profile"

using namespace std;

bool
ProfileFile::have_repo()
{
	if(!overlay.know_path)
		return false;
	overlay.readLabel();
	return !overlay.label.empty();
}


/** Exclude this files from listing of files in profile. */
static const char *profile_exclude[] = { "parent", "..", "." , NULLPTR };

/** Add all files from profile and its parents to m_profile_files. */
bool CascadingProfile::addProfile(const char *profile, unsigned int depth)
{
	// Use pushback_lines to avoid keeping file descriptor open:
	// Who know what's our limit of open file descriptors.
	if(unlikely(depth >= 255)) {
		cerr << _("Recursion level for cascading profiles exceeded; stopping reading parents") << endl;
		return false;
	}
	string s(normalize_path(profile, true, true));
#ifdef DEBUG_PROFILE_PATHS
	cout << eix::format("Adding to Profile:\n\t(%s) -> %r\n") % profile % s;
#endif
	if(s.empty()) {
		return false;
	}
	vector<string> parents;
	if(pushback_lines((s + "parent").c_str(), &parents)) {
		for(vector<string>::const_iterator it(parents.begin());
			likely(it != parents.end()); ++it) {
			if(it->empty())
				continue;
			if((*it)[0] == '/')
				addProfile(it->c_str(), depth + 1);
			else
				addProfile((s + (*it)).c_str(), depth + 1);
		}
	}
	vector<string> filenames;
	bool r(pushback_files(s, filenames, profile_exclude, 3));
	for(vector<string>::const_iterator it(filenames.begin());
		likely(it != filenames.end()); ++it) {
		listaddFile(*it, NULLPTR);
	}
	return r;
}

class ProfileFilenames
{
	typedef CascadingProfile::Handler Handler;
	typedef map<string, Handler> NameMap;
	NameMap name_map;
public:
	void initstring(const std::string &s, Handler h)
	{ name_map.insert(pair<string, Handler>(s, h)); }

	Handler operator[](const std::string s) const ATTRIBUTE_PURE
	{
		NameMap::const_iterator it(name_map.find(s));
		if(it != name_map.end()) {
			return it->second;
		}
		return NULLPTR;
	}

	ProfileFilenames()
	{
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
static ProfileFilenames profile_filenames;

bool
CascadingProfile::readremoveFiles()
{
	bool ret(false);
	for(vector<ProfileFile>::iterator file(m_profile_files.begin());
		likely(file != m_profile_files.end()); ++file) {
		const char *filename(strrchr(file->c_str(), '/'));
		if(filename == NULLPTR)
			continue;
		++filename;
		CascadingProfile::Handler handler(profile_filenames[filename]);
		if(handler == NULLPTR) {
			continue;
		}

		vector<string> lines;
		pushback_lines(file->c_str(), &lines, false, true, true);
		const char *repo;
		if(file->have_repo()) {
			repo = file->get_repo().c_str();
		}
		else {
			repo = NULLPTR;
		}
		ret |= (this->*handler)(lines, file->name(), repo);
	}
	m_profile_files.clear();
	return ret;
}

bool
CascadingProfile::readPackages(const vector<string> &lines, const string &filename, const char *repo)
{
	bool ret(false);
	PreList::FilenameIndex file_system(p_system.push_name(filename, repo));
	PreList::FilenameIndex file_system_allowed(p_system_allowed.push_name(filename, repo));
	PreList::LineNumber number(1);
	for(vector<string>::const_iterator it(lines.begin());
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
				ret |= p_system.remove_line(p);
			}
			else {
				ret |= p_system.add_line(p, file_system, number);
			}
		}
		else if(unlikely(remove)) {
			ret |= p_system_allowed.remove_line(p);
		}
		else {
			ret |= p_system_allowed.add_line(p, file_system_allowed, number);
		}
	}
	return ret;
}

bool
CascadingProfile::readPackageMasks(const vector<string> &lines, const string &filename, const char *repo)
{
	return p_package_masks.handle_file(lines, filename, repo, false);
}

bool
CascadingProfile::readPackageUnmasks(const vector<string> &lines, const string &filename, const char *repo)
{
	return p_package_unmasks.handle_file(lines, filename, repo, false);
}

bool
CascadingProfile::readPackageKeywords(const vector<string> &lines, const string &filename, const char *repo)
{
	return p_package_keywords.handle_file(lines, filename, repo, false);
}

bool
CascadingProfile::readPackageAcceptKeywords(const vector<string> &lines, const string &filename, const char *repo)
{
	return p_package_accept_keywords.handle_file(lines, filename, repo, true);
}

/** Read all "make.defaults" files found in profile. */
void
CascadingProfile::readMakeDefaults()
{
	for(vector<string>::size_type i(0); likely(i < m_profile_files.size()); ++i) {
		if(unlikely(strcmp(strrchr(m_profile_files[i].c_str(), '/'), "/make.defaults") == 0)) {
			m_portagesettings->read_config(m_profile_files[i].name(), "");
		}
	}
}

/** Populate MaskLists from PreLists.
    All files must have been read and m_raised_arch
    must be known when this is called. */
void
CascadingProfile::finalize()
{
	if(finalized) {
		return;
	}
	finalized = true;
	p_system.initialize(m_system, Mask::maskInSystem);
	p_system_allowed.initialize(m_system_allowed, Mask::maskAllowedByProfile);
	p_package_masks.initialize(m_package_masks, Mask::maskMask);
	p_package_unmasks.initialize(m_package_unmasks, Mask::maskUnmask);
	p_package_keywords.initialize(m_package_keywords);
	p_package_accept_keywords.initialize(m_package_accept_keywords, m_portagesettings->m_raised_arch);
}

/** Cycle through profile and put path to files into this->m_profile_files. */
void
CascadingProfile::listaddProfile(const char *profile_dir)
{
	if(profile_dir) {
		addProfile(profile_dir);
		return;
	}
	const string &s((*m_portagesettings)["PORTAGE_PROFILE"]);
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

void
CascadingProfile::applyMasks(Package *p) const
{
	if(m_init_world || use_world) {
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			(*it)->maskflags.set(MaskFlags::MASK_NONE);
		}
	}
	else {
		for(Package::iterator it(p->begin()); likely(it != p->end()); ++it) {
			(*it)->maskflags.clearbits(~MaskFlags::MASK_WORLD);
		}
	}
	m_system_allowed.applyMasks(p);
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
		for(vector<SetsIndex>::const_iterator it(v->sets_indizes.begin());
			unlikely(it != v->sets_indizes.end()); ++it) {
			const std::string &set_name(m_portagesettings->set_names[*it]);
			m_system_allowed.applySetMasks(*v, set_name);
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

void
CascadingProfile::applyKeywords(Package *p) const
{
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
		for(vector<SetsIndex>::const_iterator it(v->sets_indizes.begin());
			unlikely(it != v->sets_indizes.end()); ++it) {
			const char *set_name(m_portagesettings->set_names[*it].c_str());
			m_package_keywords.applyListSetItems(*v, set_name);
			m_package_accept_keywords.applyListSetItems(*v, set_name);
		}
	}
}
