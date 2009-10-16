// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "cascadingprofile.h"

#include <eixTk/utils.h>
#include <eixTk/filenames.h>
#include <eixTk/varsreader.h>
#include <portage/conf/portagesettings.h>

#include <fstream>

/* Path to symlink to profile */
#define PROFILE_LINK "/etc/make.profile"

using namespace std;

/** Exclude this files from listing of files in profile. */
static const char *profile_exclude[] = { "parent", "..", "." , NULL };

/** Add all files from profile ans its parents to m_profile_files. */
void CascadingProfile::addProfile(const char *profile, unsigned int depth)
{
	// Use pushback_lines to avoid keeping file descriptor open:
	// Who know what's our limit of open file descriptors.
	if(depth >= 255)
		cerr << _("Recursion level for cascading profiles exceeded; stopping reading parents") << endl;
	string s = normalize_path(profile, true, true);
#ifdef DEBUG_PROFILE_PATHS
	cout << eix::format_("Adding to Profile:\n\t(%s) -> %r\n") % profile % s;
#endif
	if(s.empty())
		return;
	vector<string> parents;
	if(pushback_lines((s + "parent").c_str(), &parents)) {
		for(vector<string>::const_iterator it = parents.begin();
			it != parents.end(); ++it) {
			if(it->empty())
				continue;
			if((*it)[0] == '/')
				addProfile(it->c_str(), depth + 1);
			else
				addProfile((s + (*it)).c_str(), depth + 1);
		}
	}
	pushback_files(s, m_profile_files, profile_exclude, 3);
}

bool
CascadingProfile::readremoveFiles()
{
	bool ret = false;
	for(vector<string>::iterator file = m_profile_files.begin();
		file != m_profile_files.end();
		++file)
	{
		bool (CascadingProfile::*handler)(const string &line);
		const char *filename = strrchr(file->c_str(), '/');
		if(!filename)
			continue;
		++filename;
		if(!strcmp(filename, "packages"))
			handler = &CascadingProfile::readPackages;
		else if(!strcmp(filename, "packages.d"))
			handler = &CascadingProfile::readPackages;
		else if(!strcmp(filename, "package.mask"))
			handler = &CascadingProfile::readPackageMasks;
		else if(!strcmp(filename, "package.mask.d"))
			handler = &CascadingProfile::readPackageMasks;
		else if(!strcmp(filename, "package.unmask"))
			handler = &CascadingProfile::readPackageUnmasks;
		else if(!strcmp(filename, "package.unmask.d"))
			handler = &CascadingProfile::readPackageUnmasks;
		else if(!strcmp(filename, "package.keywords"))
			handler = &CascadingProfile::readPackageKeywords;
		else if(!strcmp(filename, "package.keywords.d"))
			handler = &CascadingProfile::readPackageKeywords;
		else
			continue;

		vector<string> lines;
		pushback_lines(file->c_str(), &lines, true, true, true);

		for(vector<string>::iterator i(lines.begin()), i_end(lines.end());
			i != i_end;
			++i)
		{
			try {
				if((this->*handler) (*i))
					ret = true;
			}
			catch(const ExBasic &e)
			{
				portage_parse_error(*file, lines.begin(), i, e);
			}
		}
	}
	m_profile_files.clear();
	return ret;
}

bool
CascadingProfile::readPackages(const string &line)
{
	/* Cycle through and get rid of comments ..
	 * lines beginning with '*' are m_system-packages
	 * all others are masked by profile .. if they don't match :) */
	const char *p = line.c_str();
	bool remove = (*p == '-');

	if(remove)
	{
		++p;
	}

	Mask *m = NULL;
	MaskList<Mask> *ml = NULL;
	switch(*p)
	{
		case '*':
			++p;
			m = new Mask(p, Mask::maskInSystem) ;
			ml = &m_system;
			break;
		default:
			m = new Mask(p, Mask::maskAllowedByProfile);
			ml = &m_system_allowed;
			break;
	}

	if(remove)
	{
		bool ret = (ml->remove(m));
		delete m;
		return ret;
	}
	else
	{
		return (ml->add(m));
	}
}

/** Read all "make.defaults" files found in profile. */
void
CascadingProfile::readMakeDefaults()
{
	for(vector<string>::size_type i = 0; i < m_profile_files.size(); ++i) {
		if(!strcmp(strrchr(m_profile_files[i].c_str(), '/'), "/make.defaults")) {
			m_portagesettings->read_config(m_profile_files[i], "");
		}
	}
}

bool
CascadingProfile::readPackageMasks(const string &line)
{
	if(line[0] == '-') {
		Mask *m = new Mask(line.substr(1).c_str(), Mask::maskMask);
		bool ret = m_package_masks.remove(m);
		delete m;
		return ret;
	}
	else {
		return m_package_masks.add(new Mask(line.c_str(), Mask::maskMask));
	}
}

bool
CascadingProfile::readPackageUnmasks(const string &line)
{
	if(line[0] == '-') {
		Mask *m = new Mask(line.substr(1).c_str(), Mask::maskUnmask);
		bool ret = m_package_unmasks.remove(m);
		delete m;
		return ret;
	}
	else {
		return m_package_unmasks.add(new Mask(line.c_str(), Mask::maskUnmask));
	}
}

bool
CascadingProfile::readPackageKeywords(const string &line)
{
	string::size_type s = line.find(' ');
	if(line[0] == '-') {
		string name;
		if(s == string::npos) {
			name = line;
		}
		else {
			if(s == 1)
				return false;
			name = line.substr(1, s);
		}
		PKeywordMask *m = new PKeywordMask(name.c_str());
		bool ret = m_package_keywords.remove(m);
		delete m;
		return ret;
	}
	else {
		if(s == string::npos)
			return false;
		if(s + 1 >= line.size())
			return false;
		PKeywordMask *m = new PKeywordMask(line.substr(0, s).c_str());
		m->keywords = line.substr(s + 1);
		return m_package_keywords.add(m);
	}
}

/** Cycle through profile and put path to files into this->m_profile_files. */
void
CascadingProfile::listaddProfile(const char *profile_dir) throw(ExBasic)
{
	if(profile_dir) {
		addProfile(profile_dir);
		return;
	}
	{
		const string &s = (*m_portagesettings)["PORTAGE_PROFILE"];
		if(!s.empty()) {
			addProfile(s.c_str());
			return;
		}
	}
	addProfile(((m_portagesettings->m_eprefixconf) + PROFILE_LINK).c_str());
}

void
CascadingProfile::applyMasks(Package *p) const
{
	if(m_init_world || use_world) {
		for(Package::iterator it = p->begin(); it != p->end(); ++it) {
			(*it)->maskflags.set(MaskFlags::MASK_NONE);
		}
	}
	else {
		for(Package::iterator it = p->begin(); it != p->end(); ++it) {
			if((*it)->maskflags.isWorld()) {
				(*it)->maskflags.set(MaskFlags::MASK_WORLD);
			}
			else {
				(*it)->maskflags.set(MaskFlags::MASK_NONE);
			}
		}
	}
	getAllowedPackages()->applyMasks(p);
	getSystemPackages()->applyMasks(p);
	getPackageMasks()->applyMasks(p);
	getPackageUnmasks()->applyMasks(p);
	if(use_world)
		getWorldPackages()->applyMasks(p);
	m_portagesettings->finalize(p);
}

void
CascadingProfile::applyKeywords(Package *p) const
{
	for(Package::iterator it = p->begin(); it != p->end(); ++it) {
		it->reset_effective_keywords();
	}
	getPackageKeywords()->applyMasks(p);
}
