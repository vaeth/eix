// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "vardbpkg.h"
#include <database/header.h>
#include <eixTk/exceptions.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <eixTk/stringutils.h>
#include <eixTk/sysutils.h>
#include <eixTk/utils.h>
#include <portage/basicversion.h>
#include <portage/extendedversion.h>
#include <portage/instversion.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <cstdlib>
#include <dirent.h>

using namespace std;

inline static void
sort_installed(map<string, vector<InstVersion> > *maping)
{
	for(map<string, vector<InstVersion> >::iterator it(maping->begin());
		likely(it != maping->end()); ++it) {
		sort(it->second.begin(), it->second.end());
	}
}

/** Find installed versions of packet "name" in category "category".
 * @return NULLPTR if not found .. else pointer to vector of versions. */
vector<InstVersion> *
VarDbPkg::getInstalledVector(const string &category, const string &name)
{
	map<string, map<string, vector<InstVersion> >* >::iterator map_it(installed.find(category));
	/* Not yet read */
	if(map_it == installed.end()) {
		readCategory(category.c_str());
		return getInstalledVector(category, name);
	}

	map<string, vector<InstVersion> >* installed_cat(map_it->second);
	/* No such category in db-directory. */
	if(installed_cat == NULLPTR)
		return NULLPTR;

	/* Find packet */
	map<string, vector<InstVersion> >::iterator cat_it(installed_cat->find(name));
	if(cat_it == installed_cat->end())
		return NULLPTR; /* Not installed */
	return &(cat_it->second);
}

/** Returns true if v is in vec. v=NULLPTR is always in vec.
    If a serious result is found and r is nonzero, r points to that result */
bool
VarDbPkg::isInVec(vector<InstVersion> *vec, const BasicVersion *v, InstVersion **r)
{
	if(likely(vec != NULLPTR)) {
		if(unlikely(v == NULLPTR))
			return true;
		for(vector<InstVersion>::size_type i(0); likely(i < vec->size()); ++i) {
			if((*vec)[i] == *v) {
				if(r != NULLPTR)
					*r = &((*vec)[i]);
				return true;
			}
		}
	}
	return false;
}

short
VarDbPkg::isInstalledVersion(const Package &p, const Version *v, const DBHeader& header)
{
	InstVersion *inst(NULLPTR);
	if(!isInstalled(p, v, &inst))
		return 0;
	if(!readOverlay(p, *inst, header))
		return -1;
	if((inst->overlay_key) != (v->overlay_key))
		return 0;
	return 1;
}

/** Returns number of installed versions of this package
 * @param p Check for this Package. */
vector<InstVersion>::size_type
VarDbPkg::numInstalled(const Package &p)
{
	vector<InstVersion> *vec(getInstalledVector(p));
	if(vec == NULLPTR)
		return 0;
	return vec->size();
}

bool
VarDbPkg::readOverlay(const Package &p, InstVersion &v, const DBHeader& header) const
{
	if(likely(v.know_overlay))
		return !v.overlay_failed;

	v.know_overlay = true;
	v.overlay_failed = false;

	// Do not really check if the package is only at one overlay.
	if(check_installed_overlays == 0) {
		if(likely(p.have_same_overlay_key())) {
			v.overlay_key = p.largest_overlay;
			return true;
		}
	}

	v.reponame = readOverlayLabel(&p, &v);
	if(v.reponame.empty()) {
		if(check_installed_overlays < 0) {
			if(likely(p.have_same_overlay_key())) {
				v.overlay_key = p.largest_overlay;
				return true;
			}
		}
	}
	else if(header.find_overlay(&v.overlay_key, v.reponame.c_str(), NULLPTR, 0, DBHeader::OVTEST_LABEL))
		return true;
	v.overlay_failed = true;
	return false;
}

string
VarDbPkg::readOverlayLabel(const Package *p, const BasicVersion *v) const
{
	vector<string> lines;
	string dirname(m_directory + p->category + "/" + p->name + "-" + v->getFull());
	pushback_lines((dirname + "/repository").c_str(),
		&lines, true, false, false);
	pushback_lines((dirname + "/REPOSITORY").c_str(),
		&lines, true, false, false);
	if(lines.empty())
		return emptystring;
	return lines[0];
}

bool
VarDbPkg::readSlot(const Package &p, InstVersion &v) const
{
	if(v.know_slot)
		return true;
	if(!get_slots)
		return false;
	if(v.read_failed)
		return false;
	try {
		vector<string> lines;
		if(unlikely(!pushback_lines(
			(m_directory + p.category + "/" + p.name + "-" + v.getFull() + "/SLOT").c_str(),
			&lines, true, false, false))) {
			v.read_failed = true;
			return false;
		}
		if(lines.empty())
			v.slotname = emptystring;
		else if(lines[0] == "0")
			v.slotname = emptystring;
		else
			v.slotname = lines[0];
		v.know_slot = true;
		return true;
	}
	catch(const ExBasic &e) {
		cerr << e << endl;
		v.read_failed = true;
		return false;
	}
}

bool
VarDbPkg::readUse(const Package &p, InstVersion &v) const
{
	if(likely(v.know_use))
		return true;
	v.know_use = true;
	v.inst_iuse.clear();
	v.usedUse.clear();
	set<string> iuse_set;
	vector<string> alluse;
	try {
		string dirname(m_directory + p.category + "/" + p.name + "-" + v.getFull());
		vector<string> lines;
		if(unlikely(!pushback_lines((dirname + "/IUSE").c_str(),
			&lines, true, false, false)))
			return false;
		join_and_split(v.inst_iuse, lines);

		lines.clear();
		if(unlikely(!pushback_lines((dirname + "/USE").c_str(),
			&lines, true, false, false)))
			return false;
		join_and_split(alluse, lines);
	}
	catch(const ExBasic &e) {
		cerr << e << endl;
		return false;
	}
	for(vector<string>::iterator it(v.inst_iuse.begin());
		it != v.inst_iuse.end(); ++it) {
		while(((*it)[0] == '+') || ((*it)[0] == '-'))
			it->erase(0, 1);
	}
	make_set<string>(iuse_set, v.inst_iuse);
	make_vector<string>(v.inst_iuse, iuse_set);

	for(vector<string>::iterator it(alluse.begin());
		it != alluse.end(); ++it) {
		while(((*it)[0] == '+') || ((*it)[0] == '-'))
			it->erase(0, 1);
	}

	for(vector<string>::iterator it(alluse.begin());
		likely(it != alluse.end()); ++it) {
		if(iuse_set.find(*it) != iuse_set.end()) {
			v.usedUse.insert(*it);
		}
	}
	return true;
}

bool
VarDbPkg::readRestricted(const Package &p, InstVersion &v, const DBHeader& header) const
{
	if(likely(v.know_restricted))
		return true;
	v.know_restricted = true;
	v.restrictFlags = ExtendedVersion::RESTRICT_NONE;
	v.propertiesFlags = ExtendedVersion::PROPERTIES_NONE;
	for(Package::const_iterator it(p.begin()); likely(it != p.end()); ++it) {
		if(BasicVersion::compare(**it, v) != 0)
			continue;
		if(readSlot(p, v)) {
			if(unlikely(it->slotname != v.slotname))
				continue;
		}
		if(readOverlay(p, v, header)) {
			if(unlikely(it->overlay_key != v.overlay_key))
				continue;
		}
		v.restrictFlags = it->restrictFlags;
		v.propertiesFlags = it->propertiesFlags;
		break;
	}
	if(!care_of_restrictions)
		return true;
	try {
		string dirname(m_directory + p.category + "/" + p.name + "-" + v.getFull());
		vector<string> lines;
		if(unlikely(!pushback_lines((dirname + "/RESTRICT").c_str(),
			&lines, true, false, false))) {
			// It is OK that this file does not exist:
			// Portage does this if RESTRICT is not set.
			v.restrictFlags = ExtendedVersion::RESTRICT_NONE;
			return true;
		}
		if(lines.size() == 1)
			v.set_restrict(lines[0]);
		else
			v.set_restrict(join_to_string(lines));
	}
	catch(const ExBasic &e) {
		cerr << e << endl;
		return false;
	}
	return true;
}

void
VarDbPkg::readInstDate(const Package &p, InstVersion &v) const
{
	if(v.instDate != 0) {
		return;
	}
	string dirname(m_directory + p.category + "/" + p.name + "-" + v.getFull());
	vector<string> datelines;
	if(use_build_time &&
		pushback_lines((dirname + "/BUILD_TIME").c_str(), &datelines)) {
		for(vector<string>::const_iterator it(datelines.begin());
			it != datelines.end(); ++it) {
			if((v.instDate = my_atoi(it->c_str())) != 0) {
				return;
			}
		}
	}
	v.instDate = get_mtime(dirname.c_str());
}

/** Read category from db-directory. */
void
VarDbPkg::readCategory(const char *category)
{
	/* Pointers to db and category DIRectory */
	DIR *dir_category;
	struct dirent* package_entry;  /* current package dirent */

	/* Open category-directory */
	string dir_category_name(m_directory + category);
	if( (dir_category = opendir(dir_category_name.c_str())) == NULLPTR) {
		installed[category] = NULLPTR;
		return;
	}
	dir_category_name.append(1, '/');
	map<string, vector<InstVersion> >* category_installed;
	installed[category] = category_installed = new map<string, vector<InstVersion> >;

	/* Cycle through this category */
	while(likely( (package_entry = readdir(dir_category)) != NULLPTR )) {
		if(package_entry->d_name[0] == '.')
			continue; /* Don't want dot-stuff */
		char **aux(ExplodeAtom::split( package_entry->d_name));
		InstVersion *instver(NULLPTR);
		if(aux == NULLPTR)
			continue;
		try {
			instver = new InstVersion(aux[1]);
		}
		catch(const ExBasic &e) {
			cerr << e << endl;
		}
		if(instver != NULLPTR) {
			(*category_installed)[aux[0]].push_back(*instver);
			delete instver;
		}
		free(aux[0]);
		free(aux[1]);
	}
	closedir(dir_category);
	sort_installed(installed[category]);
}

