// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "vardbpkg.h"

#include <eixTk/stringutils.h>
#include <eixTk/utils.h>
#include <eixTk/sysutils.h>
#include <portage/basicversion.h>
#include <portage/package.h>
#include <database/header.h>

#include <dirent.h>

#if defined(USE_BZLIB)
#include <bzlib.h>
#endif

#define UNUSED(p) ((void)(p)

using namespace std;

inline static void
sort_installed(map<string,vector<InstVersion> > *maping)
{
	map<string,vector<InstVersion> >::iterator it = maping->begin();
	while(it != maping->end())
	{
		sort(it->second.begin(), it->second.end());
		++it;
	}
}

/** Find installed versions of packet "name" in category "category".
 * @return NULL if not found .. else pointer to vector of versions. */
vector<InstVersion> *
VarDbPkg::getInstalledVector(const string &category, const string &name)
{
	map<string, map<string, vector<InstVersion> >* >::iterator map_it = installed.find(category);
	/* Not yet read */
	if(map_it == installed.end()) {
		readCategory(category.c_str());
		return getInstalledVector(category, name);
	}

	map<string, vector<InstVersion> >* installed_cat = map_it->second;
	/* No such category in db-directory. */
	if(!installed_cat)
		return NULL;

	/* Find packet */
	map<string, vector<InstVersion> >::iterator cat_it = installed_cat->find(name);
	if(cat_it == installed_cat->end())
		return NULL; /* Not installed */
	return &(cat_it->second);
}

/** Returns true if v is in vec. v=NULL is always in vec.
    If a serious result is found and r is nonzero, r points to that result */
bool
VarDbPkg::isInVec(vector<InstVersion> *vec, const BasicVersion *v, InstVersion **r)
{
	if(vec)
	{
		if(!v)
			return true;
		for(vector<InstVersion>::size_type i = 0; i < vec->size(); ++i) {
			if((*vec)[i] == *v) {
				if(r)
					*r = &((*vec)[i]);
				return true;
			}
		}
	}
	return false;
}

short
VarDbPkg::isInstalledVersion(const Package &p, const Version *v, const DBHeader& header, const char *portdir)
{
	InstVersion *inst = NULL;
	if(!isInstalled(p, v, &inst))
		return 0;
	if(!readOverlay(p, *inst, header, portdir))
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
	vector<InstVersion> *vec = getInstalledVector(p);
	if(!vec)
		return 0;
	return vec->size();
}

bool
VarDbPkg::readOverlay(const Package &p, InstVersion &v, const DBHeader& header, const char *portdir) const
{
	if(v.know_overlay)
		return !v.overlay_failed;

	v.know_overlay = true;
	v.overlay_failed = false;
	v.overlay_keytext.clear();

	// Do not really check if the package is only at one overlay.
	if(check_installed_overlays == 0) {
		if(p.have_same_overlay_key) {
			v.overlay_key = p.largest_overlay;
			return true;
		}
	}

	string label = readOverlayLabel(&p, dynamic_cast<BasicVersion *>(&v));
	if(label.empty()) {
		if(check_installed_overlays < 0) {
			if(p.have_same_overlay_key) {
				v.overlay_key = p.largest_overlay;
				return true;
			}
		}
	}
	else if(header.find_overlay(&v.overlay_key, label.c_str(), NULL, 0, DBHeader::OVTEST_LABEL))
		return true;

	string opath = readOverlayPath(&p, dynamic_cast<BasicVersion *>(&v));
	if(opath.empty()) {
		v.overlay_keytext = label;
		v.overlay_failed = true;
		return false;
	}
	else if(header.find_overlay(&v.overlay_key, opath.c_str(), portdir, 0, DBHeader::OVTEST_ALLPATH))
		return true;
	v.overlay_failed = true;
	if(label.empty())
		v.overlay_keytext = opath;
	else
		v.overlay_keytext = string("\"") + label + "\" "+ opath;
	return false;
}

string
VarDbPkg::readOverlayLabel(const Package *p, const BasicVersion *v) const
{
	std::vector<std::string> lines;
	pushback_lines(
		(_directory + p->category + "/" + p->name + "-" + v->getFull() + "/repository").c_str(),
		&lines, true, false, false);
	for(std::vector<std::string>::const_iterator i = lines.begin();
		i != lines.end(); ++i) {
		if(i->empty())
			continue;
		return *i;
	}
	return "";
}

string
VarDbPkg::readOverlayPath(const Package *p, const BasicVersion *v) const
{
#if defined(USE_BZLIB)
	BZFILE *fh = BZ2_bzopen(
		(_directory + p->category + "/" + p->name + "-" + v->getFull() + "/environment.bz2").c_str(),
		"rb");
	if(!fh)
		return "";
	typedef int BufInd;
	const BufInd bufsize = 256;
	const BufInd strsize = 7;
	char buffer[bufsize + 1];
	BufInd bufend = BZ2_bzread(fh, buffer, bufsize);
	if(bufend < strsize) {
		BZ2_bzclose(fh);
		return "";
	}

	// find EBUILD=... (cycling buffer if necessary)
	BufInd i = 0;
	bool in_newline = true;
	for(;;) {
		if(i + strsize < bufend) {
			if(in_newline &&
				(strncmp(buffer + i, "EBUILD=", strsize) == 0))
				break;
		}
		else {
			BufInd j = bufend - i;
			if(j)
				strncpy(buffer, buffer + i, j);
			bufend = BZ2_bzread(fh, buffer + j, bufsize - j);
			if(bufend > 0)
				bufend += j;
			if(bufend < strsize) {
				BZ2_bzclose(fh);
				return "";
			}
			i = 0;
			continue;
		}
		in_newline = false;
		while(i < bufend) {
			if(buffer[i++] == '\n') {
				in_newline = true;
				break;
			}
		}
	}
	i += strsize;

	// Store EBUILD=  content in path (cycling buffer if necessary)
	string path;
	bool done = false;
	for(;;) {
		char *ptr = buffer + i;
		for(; i < bufend; i++) {
			if(buffer[i] == '\n') {
				done = true;
				break;
			}
		}
		buffer[i] = '\0';
		path.append(ptr);
		if(done)
			break;
		i = 0;
		bufend = BZ2_bzread(fh, buffer, bufsize);
		if(bufend <=0)
			break;
	}
	// Reading has finished successfully
	BZ2_bzclose(fh);

	// Chop /*/*/*
	string::size_type l = path.size() + 1;
	for(int c = 0; c < 3; c++) {
		l = path.rfind('/', l - 1);
		if(l == string::npos)
			return "";
	}
	path.erase(l);
	return path;
#else
	UNUSED(p); UNUSED(v);
	return ""
#endif
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
		if(!pushback_lines(
			(_directory + p.category + "/" + p.name + "-" + v.getFull() + "/SLOT").c_str(),
			&lines, true, false, false))
		{
			v.read_failed = true;
			return false;
		}
		if(!lines.size())
			v.slotname = "";
		else if(lines[0] == "0")
			v.slotname = "";
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
	if(v.know_use)
		return true;
	v.know_use = true;
	v.inst_iuse.clear();
	v.usedUse.clear();
	set<string> iuse_set;
	vector<string> alluse;
	try {
		string dirname = _directory + p.category + "/" + p.name + "-" + v.getFull();
		vector<string> lines;
		if(!pushback_lines((dirname + "/IUSE").c_str(),
			&lines, true, false, false))
			return false;
		v.inst_iuse = split_string(join_vector(lines, " "));
		sort_uniquify(v.inst_iuse);
		make_set<string>(iuse_set, v.inst_iuse);
		/* If you do not want the alphabetical order in v.inst_iuse
		   but instead the original order in the IUSE file,
		   use the following code instead of the above 4 lines.

		vector<string> iuse = split_string(join_vector(lines, " "));
		for(vector<string>::iterator it = iuse.begin();
			it != iuse.end(); ++it)
		{
			if(iuse_set.find(*it) != iuse_set.end())
				continue;
			iuse_set.insert(*it);
			v.inst_iuse.push_back(*it);
		}
		*/
		lines.clear();
		if(!pushback_lines((dirname + "/USE").c_str(),
			&lines, true, false, false))
			return false;
		alluse = split_string(join_vector(lines, " "));
	}
	catch(const ExBasic &e) {
		cerr << e << endl;
		return false;
	}
	for(vector<string>::iterator it = alluse.begin();
		it != alluse.end(); ++it)
	{
		if(iuse_set.find(*it) != iuse_set.end()) {
			v.usedUse.insert(*it);
		}
	}
	return true;
}

bool
VarDbPkg::readRestricted(const Package &p, InstVersion &v, const DBHeader& header, const char *portdir) const
{
	if(v.know_restricted)
		return true;
	v.know_restricted = true;
	if(!care_of_restrictions) {
		for(Package::const_iterator it = p.begin(); it != p.end(); ++it) {
			if(*dynamic_cast<const BasicVersion*>(*it) != *dynamic_cast<BasicVersion*>(&v))
				continue;
			if(readSlot(p, v)) {
				if(it->slotname != v.slotname)
					continue;
			}
			if(readOverlay(p, v, header, portdir)) {
				if(it->overlay_key != v.overlay_key)
					continue;
			}
			v.restrictFlags = it->restrictFlags;
			return true;
		}
	}
	try {
		string dirname = _directory + p.category + "/" + p.name + "-" + v.getFull();
		vector<string> lines;
		if(!pushback_lines((dirname + "/RESTRICT").c_str(),
			&lines, true, false, false))
			return false;
		if(lines.size() == 1)
			v.set_restrict(lines[0]);
		else
			v.set_restrict(join_vector(lines));
	}
	catch(const ExBasic &e) {
		cerr << e << endl;
		return false;
	}
	return true;
}

/** Read category from db-directory. */
void
VarDbPkg::readCategory(const char *category)
{
	/* Pointers to db and category DIRectory */
	DIR *dir_category;
	struct dirent* package_entry;  /* current package dirent */

	/* Open category-directory */
	string dir_category_name = _directory + category;
	if( (dir_category = opendir(dir_category_name.c_str())) == NULL) {
		installed[category] = NULL;
		return;
	}
	dir_category_name.append("/");
	map<string, vector<InstVersion> >* category_installed;
	installed[category] = category_installed = new map<string,vector<InstVersion> >;

	/* Cycle through this category */
	while( (package_entry = readdir(dir_category)) != NULL )
	{

		if(package_entry->d_name[0] == '.')
			continue; /* Don't want dot-stuff */
		char **aux = ExplodeAtom::split( package_entry->d_name);
		InstVersion *instver = NULL;
		if(aux == NULL)
			continue;
		try {
			instver = new InstVersion(aux[1]);
		}
		catch(const ExBasic &e) {
			cerr << e << endl;
		}
		if(instver)
		{
			instver->instDate = get_mtime((dir_category_name + package_entry->d_name).c_str());
			(*category_installed)[aux[0]].push_back(*instver);
			delete instver;
		}

		free(aux[0]);
		free(aux[1]);
	}
	closedir(dir_category);
	sort_installed(installed[category]);
}

