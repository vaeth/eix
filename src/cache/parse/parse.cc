// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "parse.h"
#include <cache/common/selectors.h>
#include <cache/common/flat_reader.h>
#include <cache/metadata/metadata.h>

#include <portage/package.h>
#include <portage/packagetree.h>
#include <eixTk/varsreader.h>

#include <map>

using namespace std;

bool
ParseCache::initialize(const string &name)
{
	vector<string> names = split_string(name, true, "#");
	vector<string>::const_iterator it_name = names.begin();
	if(it_name == names.end())
		return false;
	vector<string> s = split_string(*it_name, false, "|");
	if(s.empty())
		return false;
	try_parse = nosubst = false;
	bool try_ebuild = false, use_sh = false;
	for(vector<string>::const_iterator it = s.begin(); it != s.end(); ++it) {
		if(*it == "parse") {
			try_parse = true; nosubst = false;
		}
		else if(*it == "parse*") {
			try_parse = true; nosubst = true;
		}
		else if(*it == "ebuild") {
			try_ebuild = true; use_sh = false;
		}
		else if(*it == "ebuild*") {
			try_ebuild = true; use_sh = true;
		}
		else
			return false;
	}
	if(try_ebuild)
		ebuild_exec = new EbuildExec(use_sh, this);
	while(++it_name != names.end()) {
		MetadataCache *p = new MetadataCache;
		if(p->initialize(*it_name)) {
			further.push_back(p);
			continue;
		}
		delete p;
		return false;
	}
	return true;
}

const char *
ParseCache::getType() const
{
	static string s;
	if(try_parse) {
		if(nosubst)
			s = "parse*";
		else
			s = "parse";
	}
	if(ebuild_exec) {
		const char *t;
		if(ebuild_exec->use_sh())
			t = "ebuild*";
		else
			t = "ebuild";
		if(s.empty())
			s = t;
		else {
			s.append("|");
			s.append(t);
		}
	}
	for(vector<BasicCache*>::const_iterator it = further.begin();
		it != further.end(); ++it) {
		s.append("#");
		s.append((*it)->getType());
	}
	return s.c_str();
}

ParseCache::~ParseCache()
{
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		delete *it;
	if(ebuild_exec) {
		ebuild_exec->delete_cachefile();
		delete ebuild_exec;
		ebuild_exec = NULL;
	}
}

void
ParseCache::setScheme(const char *prefix, const char *prefixport, std::string scheme)
{
	BasicCache::setScheme(prefix, prefixport, scheme);
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		(*it)->setScheme(prefix, prefixport, scheme);
}

void
ParseCache::setKey(Version::Overlay key)
{
	BasicCache::setKey(key);
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		(*it)->setKey(key);
}

void
ParseCache::setOverlayName(std::string name)
{
	BasicCache::setOverlayName(name);
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		(*it)->setOverlayName(name);
}

void
ParseCache::setErrorCallback(ErrorCallback error_callback)
{
	BasicCache::setErrorCallback(error_callback);
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		(*it)->setErrorCallback(error_callback);
}

void
ParseCache::set_checking(string &str, const char *item, const VarsReader &ebuild, bool *ok)
{
	bool check = (ebuild_exec && ok && (*ok));
	const string *s = ebuild.find(item);
	if(!s) {
		str.clear();
		if(check)
			*ok = false;
		return;
	}
	str = join_vector(split_string(*s));
	if(!check)
		return;
	if((str.find_first_of('`') != string::npos) ||
		(str.find("$(") != string::npos))
		*ok = false;
}

void
ParseCache::parse_exec(const char *fullpath, const string &dirpath, bool read_onetime_info, bool &have_onetime_info, Package *pkg, Version *version)
{
	version->overlay_key = m_overlay_key;
	string keywords, restr, props, iuse;
	bool ok = try_parse;
	if(ok) {
		VarsReader::Flags flags = VarsReader::NONE;
		if(!read_onetime_info)
			flags |= VarsReader::ONLY_KEYWORDS_SLOT;
		map<string, string> env;
		if(!nosubst) {
			flags |= VarsReader::INTO_MAP | VarsReader::SUBST_VARS;
			env_add_package(env, *pkg, *version, dirpath, fullpath);
		}
		VarsReader ebuild(flags);
		if(flags & VarsReader::INTO_MAP)
			ebuild.useMap(&env);
		try {
			ebuild.read(fullpath);
		}
		catch(const ExBasic &e) {
			m_error_callback(eix::format(_("Could not properly parse %s: %s")) % fullpath % e.getMessage());
		}

		set_checking(keywords, "KEYWORDS", ebuild, &ok);
		set_checking(version->slotname, "SLOT", ebuild, &ok);
		// Empty SLOT is not ok:
		if(ok && ebuild_exec && version->slotname.empty())
			ok = false;
		set_checking(restr, "RESTRICT", ebuild);
		set_checking(props, "PROPERTIES", ebuild);
		set_checking(iuse, "IUSE", ebuild, &ok);
		if(read_onetime_info) {
			set_checking(pkg->homepage, "HOMEPAGE",    ebuild, &ok);
			set_checking(pkg->licenses, "LICENSE",     ebuild, &ok);
			set_checking(pkg->desc,     "DESCRIPTION", ebuild, &ok);
			set_checking(pkg->provide,  "PROVIDE",     ebuild);
			have_onetime_info = true;
		}
	}
	if(!ok) {
		string *cachefile = ebuild_exec->make_cachefile(fullpath, dirpath, *pkg, *version);
		if(cachefile) {
			flat_get_keywords_slot_iuse_restrict(cachefile->c_str(), keywords, version->slotname, iuse, restr, props, m_error_callback);
			flat_read_file(cachefile->c_str(), pkg, m_error_callback);
			ebuild_exec->delete_cachefile();
		}
		else
			m_error_callback(eix::format(_("Could not properly execute %s")) % fullpath);
	}
	version->set_full_keywords(keywords);
	version->set_restrict(restr);
	version->set_properties(props);
	version->set_iuse(iuse);
	pkg->addVersionFinalize(version);
}

void
ParseCache::readPackage(Category &vec, const string &pkg_name, const string &directory_path, const vector<string> &files) throw(ExBasic)
{
	bool have_onetime_info = false;

	Package *pkg = vec.findPackage(pkg_name);
	if(pkg)
		have_onetime_info = true;
	else
		pkg = vec.addPackage(pkg_name);

	for(vector<string>::const_iterator fileit = files.begin();
		fileit != files.end(); ++fileit) {
		string::size_type pos = ebuild_pos(*fileit);
		if(pos == string::npos)
			continue;

		/* Check if we can split it */
		char *ver = ExplodeAtom::split_version(fileit->substr(0, pos).c_str());
		if(!ver) {
			m_error_callback(eix::format(_("Can't split filename of ebuild %s/%s")) %
				directory_path % (*fileit));
			continue;
		}

		/* Make version and add it to package. */
		Version *version = new Version(ver);
		pkg->addVersionStart(version);

		string full_path = directory_path + '/' + (*fileit);

		/* For the latest version read/change corresponding data */
		bool read_onetime_info = true;
		if(have_onetime_info) {
			if(*(pkg->latest()) != *version)
				read_onetime_info = false;
		}

		time_t ebuild_time = 0;
		vector<BasicCache*>::const_iterator it;
		for(it = further.begin(); it != further.end(); ++it) {
			time_t t = (*it)->get_time(pkg_name.c_str(), ver);
			if(!t)
				continue;
			if(!ebuild_time)
				ebuild_time = get_mtime(full_path.c_str());
			if(t >= ebuild_time)
				break;
		}
		if(it == further.end()) {
			parse_exec(full_path.c_str(), directory_path, read_onetime_info, have_onetime_info, pkg, version);
		}
		else {
			(*it)->get_version_info(pkg_name.c_str(), ver, version);
			if(read_onetime_info) {
				(*it)->get_common_info(pkg_name.c_str(), ver, pkg);
				have_onetime_info = true;
			}
		}

		free(ver);
	}

	if(!have_onetime_info) {
		vec.deletePackage(pkg_name);
	}
}

bool
ParseCache::readCategoryPrepare(Category &vec) throw(ExBasic)
{
	further_works.clear();
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		further_works.push_back((*it)->readCategoryPrepare(vec));
	catpath = m_prefix + m_scheme + '/' + vec.name;
	return scandir_cc(catpath, packages, package_selector);
}

void
ParseCache::readCategoryFinalize()
{
	further_works.clear();
	for(std::vector<BasicCache*>::iterator it = further.begin();
		it != further.end(); ++it)
		(*it)->readCategoryFinalize();
	catpath.clear();
	packages.clear();
}

bool
ParseCache::readCategory(Category &vec) throw(ExBasic)
{
	if(!readCategoryPrepare(vec))
		return false;

	for(vector<string>::const_iterator pit = packages.begin();
		pit != packages.end(); ++pit) {
		vector<string> files;
		string pkg_path = catpath + '/' + (*pit);

		if(scandir_cc(pkg_path, files, ebuild_selector))
			readPackage(vec, *pit, pkg_path, files);
	}
	readCategoryFinalize();
	return true;
}
