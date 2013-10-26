// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <dirent.h>

#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "database/header.h"
#include "eixTk/diagnostics.h"
#include "eixTk/eixint.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixTk/sysutils.h"
#include "eixTk/utils.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/instversion.h"
#include "portage/vardbpkg.h"

using std::map;
using std::set;
using std::string;
using std::vector;

using std::cerr;
using std::endl;

inline static void sort_installed(map<string, vector<InstVersion> > *maping) ATTRIBUTE_NONNULL_;


inline static void sort_installed(map<string, vector<InstVersion> > *maping) {
	for(map<string, vector<InstVersion> >::iterator it(maping->begin());
		likely(it != maping->end()); ++it) {
		sort(it->second.begin(), it->second.end());
	}
}

/** Find installed versions of packet "name" in category "category".
 * @return NULLPTR if not found .. else pointer to vector of versions. */
vector<InstVersion> *VarDbPkg::getInstalledVector(const string &category, const string &name) {
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
bool VarDbPkg::isInVec(vector<InstVersion> *vec, const BasicVersion *v, InstVersion **r) {
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

eix::SignedBool VarDbPkg::isInstalledVersion(InstVersion **inst, const Package &p, const Version *v, const DBHeader& header) {
	*inst = NULLPTR;
	if(!isInstalled(p, v, inst)) {
		return 0;
	}
	if(!readOverlay(p, *inst, header)) {
		return -1;
	}
	if(((*inst)->overlay_key) != (v->overlay_key)) {
		return 0;
	}
	return 1;
}

/** Return matching available version or NULLPTR */
Version *VarDbPkg::getAvailable(const Package &p, InstVersion *v, const DBHeader& header) const {
	for(Package::const_iterator it(p.begin()); likely(it != p.end()); ++it) {
		if(BasicVersion::compare(**it, *v) != 0) {
			continue;
		}
		if(readSlot(p, v)) {
			if(unlikely(it->slotname != v->slotname) || unlikely(it->subslotname != v->subslotname)) {
				continue;
			}
		}
		if(readOverlay(p, v, header)) {
			if(unlikely(it->overlay_key != v->overlay_key)) {
				continue;
			}
		}
		return *it;
	}
	return NULLPTR;
}

/** Returns number of installed versions of this package
 * @param p Check for this Package. */
vector<InstVersion>::size_type VarDbPkg::numInstalled(const Package &p) {
	vector<InstVersion> *vec(getInstalledVector(p));
	if(vec == NULLPTR)
		return 0;
	return vec->size();
}

bool VarDbPkg::readOverlay(const Package &p, InstVersion *v, const DBHeader& header) const {
	if(likely(v->know_overlay))
		return !v->overlay_failed;

	v->know_overlay = true;
	v->overlay_failed = false;

	// Do not really check if the package is only at one overlay.
	if(check_installed_overlays == 0) {
		if(likely(p.have_same_overlay_key())) {
			v->overlay_key = p.largest_overlay;
			return true;
		}
	}

	v->reponame = readOverlayLabel(&p, v);
	if(v->reponame.empty()) {
		if(check_installed_overlays < 0) {
			if(likely(p.have_same_overlay_key())) {
				v->overlay_key = p.largest_overlay;
				return true;
			}
		}
	} else if(header.find_overlay(&(v->overlay_key), v->reponame.c_str(), NULLPTR, 0, DBHeader::OVTEST_LABEL)) {
		return true;
	}
	v->overlay_failed = true;
	return false;
}

string VarDbPkg::readOverlayLabel(const Package *p, const BasicVersion *v) const {
	vector<string> lines;
	string dirname(m_directory);
	dirname.append(p->category);
	dirname.append(1, '/');
	dirname.append(p->name);
	dirname.append(1, '-');
	dirname.append(v->getFull());
	pushback_lines((dirname + "/repository").c_str(),
		&lines, false, false, 1);
	pushback_lines((dirname + "/REPOSITORY").c_str(),
		&lines, false, false, 1);
	if(lines.empty())
		return "";
	return lines[0];
}

bool VarDbPkg::readSlot(const Package &p, InstVersion *v) const {
	if(v->know_slot)
		return true;
	if(!get_slots)
		return false;
	if(v->read_failed)
		return false;
	vector<string> lines;
	if(unlikely(!pushback_lines(
		(m_directory + p.category + "/" + p.name + "-" + v->getFull() + "/SLOT").c_str(),
		&lines, false, false, 1))) {
		return (v->read_failed = true);
	}
	if((lines.empty()) || (lines[0] == "0")) {
		v->set_slotname("");
	} else {
		v->set_slotname(lines[0]);
	}
	return (v->know_slot = true);
}

bool VarDbPkg::readUse(const Package &p, InstVersion *v) const {
	if(likely(v->know_use))
		return true;
	v->know_use = true;
	v->inst_iuse.clear();
	v->usedUse.clear();
	set<string> iuse_set;
	vector<string> alluse;
	string dirname(m_directory + p.category + "/" + p.name + "-" + v->getFull());
	vector<string> lines;
	if(unlikely(!pushback_lines((dirname + "/IUSE").c_str(),
		&lines, false, false, 1))) {
		return false;
	}
	join_and_split(&(v->inst_iuse), lines);

	lines.clear();
	if(unlikely(!pushback_lines((dirname + "/USE").c_str(),
		&lines, false, false, 1))) {
		return false;
	}
	join_and_split(&alluse, lines);
	for(vector<string>::iterator it(v->inst_iuse.begin());
		it != v->inst_iuse.end(); ++it) {
		while(((*it)[0] == '+') || ((*it)[0] == '-'))
			it->erase(0, 1);
	}
	make_set<string>(&iuse_set, v->inst_iuse);
	make_vector<string>(&(v->inst_iuse), iuse_set);

	for(vector<string>::iterator it(alluse.begin());
		it != alluse.end(); ++it) {
		while(((*it)[0] == '+') || ((*it)[0] == '-'))
			it->erase(0, 1);
	}

	for(vector<string>::iterator it(alluse.begin());
		likely(it != alluse.end()); ++it) {
		if(iuse_set.find(*it) != iuse_set.end()) {
			v->usedUse.insert(*it);
		}
	}
	return true;
}

void VarDbPkg::readRestricted(const Package &p, InstVersion *v, const DBHeader& header) const {
	if(unlikely(!get_restrictions)) {
		return;
	}
	if(likely(v->know_restricted)) {
		return;
	}
	v->know_restricted = true;
	Version *av(getAvailable(p, v, header));
	if(av == NULLPTR) {
		v->restrictFlags = ExtendedVersion::RESTRICT_NONE;
		v->propertiesFlags = ExtendedVersion::PROPERTIES_NONE;
	} else {
		v->restrictFlags = av->restrictFlags;
		v->propertiesFlags = av->propertiesFlags;
		if(!care_of_restrictions) {
			return;
		}
	}
	string dirname(m_directory + p.category + "/" + p.name + "-" + v->getFull());
	vector<string> lines;
	if(unlikely(!pushback_lines((dirname + "/RESTRICT").c_str(),
		&lines, false, false, 1))) {
		// It is OK that this file does not exist:
		// Portage does this if RESTRICT is not set.
		v->restrictFlags = ExtendedVersion::RESTRICT_NONE;
		return;
	}
	if(likely(lines.size() == 1)) {
		v->set_restrict(lines[0]);
	} else {
		v->set_restrict(join_to_string(lines));
	}
}

void VarDbPkg::readInstDate(const Package &p, InstVersion *v) const {
	if(v->instDate != 0) {
		return;
	}
	string dirname(m_directory + p.category + "/" + p.name + "-" + v->getFull());
	vector<string> datelines;
	if(use_build_time &&
		pushback_lines((dirname + "/BUILD_TIME").c_str(),
			&datelines, false, false, 1)) {
		for(vector<string>::const_iterator it(datelines.begin());
			it != datelines.end(); ++it) {
GCC_DIAG_OFF(sign-conversion)
			if((v->instDate = my_atoi(it->c_str())) != 0) {
GCC_DIAG_ON(sign-conversion)
				return;
			}
		}
	}
	v->instDate = get_mtime(dirname.c_str());
}

void VarDbPkg::readDepend(const Package &p, InstVersion *v, const DBHeader& header) const {
	if(unlikely(!Depend::use_depend)) {
		return;
	}
	if(likely(v->know_deps)) {
		return;
	}
	v->know_deps = true;
	Version *av(getAvailable(p, v, header));
	if(av == NULLPTR) {
		v->depend.clear();
	} else {
		v->depend = av->depend;
		if(!care_of_deps) {
			return;
		}
	}
	string dirname(m_directory + p.category + "/" + p.name + "-" + v->getFull());
	vector<string> depend(4);
	depend[0] = v->depend.get_depend();
	depend[1] = v->depend.get_rdepend();
	depend[2] = v->depend.get_pdepend();
	depend[3] = v->depend.get_hdepend();
	static const char *filenames[4] = {
		"/DEPEND",
		"/RDEPEND",
		"/PDEPEND",
		"/HDEPEND"
	};
	for(eix::TinyUnsigned i(0); likely(i < 4); ++i) {
		vector<string> lines;
		if(likely(pushback_lines((dirname + filenames[i]).c_str(),
			&lines, false, false, 1))) {
			if(likely(lines.size() == 1)) {
				depend[i].assign(lines[0]);
			} else {
				depend[i].clear();
				join_to_string(&(depend[i]), lines);
			}
		}
	}
	v->depend.set(depend[0], depend[1], depend[2], depend[3], true);
}

/** Read category from db-directory. */
void VarDbPkg::readCategory(const char *category) {
	/* Pointer to category DIRectory */
	DIR *dir_category;

	/* Open category-directory */
	string dir_category_name(m_directory);
	dir_category_name.append(category);
	if((dir_category = opendir(dir_category_name.c_str())) == NULLPTR) {
		installed[category] = NULLPTR;
		return;
	}
	dir_category_name.append(1, '/');
	map<string, vector<InstVersion> >* category_installed;
	installed[category] = category_installed = new map<string, vector<InstVersion> >;

	struct dirent *package_entry;  /* current package dirent */
	/* Cycle through this category */
	while(likely((package_entry = readdir(dir_category)) != NULLPTR)) {  // NOLINT(runtime/threadsafe_fn)
		if(package_entry->d_name[0] == '.')
			continue;  /* Don't want dot-stuff */
		char **aux(ExplodeAtom::split( package_entry->d_name));
		if(aux == NULLPTR)
			continue;
		string errtext;
		InstVersion instver;
		BasicVersion::ParseResult r(instver.parseVersion(aux[1], &errtext));
		if(unlikely(r != BasicVersion::parsedOK)) {
			cerr << errtext << endl;
		}
		if(likely(r != BasicVersion::parsedError)) {
			(*category_installed)[aux[0]].push_back(instver);
		}
		free(aux[0]);
		free(aux[1]);
	}
	closedir(dir_category);
	sort_installed(installed[category]);
}

