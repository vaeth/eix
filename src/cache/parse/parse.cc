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
#include <cache/common/flat-reader.h>

#include <eixTk/stringutils.h>
#include <eixTk/formated.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>
#include <varsreader.h>

#include <map>

using namespace std;

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
ParseCache::readPackage(Category &vec, const string &pkg_name, const string &directory_path, const vector<string> &files) throw(ExBasic)
{
	bool have_onetime_info = false;

	Package *pkg = vec.findPackage(pkg_name);
	if( pkg )
		have_onetime_info = true;
	else
		pkg = vec.addPackage(pkg_name);

	for(vector<string>::const_iterator it = files.begin();
		it != files.end(); ++it)
	{
		/* Check if this is an ebuild  */
		string::size_type pos = it->length();
		static const string::size_type append_size = 7;
		if(pos <= append_size)
			continue;
		pos -= append_size;
		if(it->compare(pos, append_size, ".ebuild"))
			continue;

		/* Check if we can split it */
		char *ver = ExplodeAtom::split_version(it->substr(0, pos).c_str());
		if(!ver) {
			m_error_callback(eix::format("Can't split filename of ebuild %s/%s") %
				directory_path % (*it));
			continue;
		}

		/* Make version and add it to package. */
		Version *version = new Version(ver);
		pkg->addVersionStart(version);

		string full_path = directory_path + '/' + (*it);

		/* For the latest version read/change corresponding data */
		bool read_onetime_info = true;
		if( have_onetime_info ) {
			if(*(pkg->latest()) != *version)
				read_onetime_info = false;
		}
		/* read the ebuild */
		VarsReader::Flags flags = VarsReader::NONE;
		if(!read_onetime_info)
			flags |= VarsReader::ONLY_KEYWORDS_SLOT;
		map<string, string> env;
		if(!nosubst)
		{
			flags |= VarsReader::INTO_MAP | VarsReader::SUBST_VARS;
			env_add_package(env, *pkg, *version, directory_path, full_path.c_str());
		}
		VarsReader ebuild(flags);
		if(flags & VarsReader::INTO_MAP)
			ebuild.useMap(&env);
		version->overlay_key = m_overlay_key;
		try {
			ebuild.read(full_path.c_str());
		}
		catch(const ExBasic &e) {
			m_error_callback(eix::format("Could not properly parse %s: %s") % full_path % e.getMessage());
		}

		bool ok = true;
		string keywords, restr, iuse;
		set_checking(keywords, "KEYWORDS", ebuild, &ok);
		set_checking(version->slotname, "SLOT", ebuild, &ok);
		// Empty SLOT is not ok:
		//if(ok && ebuild_exec && version->slotname.empty())
		//	ok = false;
		set_checking(restr, "RESTRICT", ebuild);
		set_checking(iuse, "IUSE", ebuild, &ok);
		if(read_onetime_info)
		{
			set_checking(pkg->homepage, "HOMEPAGE",    ebuild, &ok);
			set_checking(pkg->licenses, "LICENSE",     ebuild, &ok);
			set_checking(pkg->desc,     "DESCRIPTION", ebuild, &ok);
			set_checking(pkg->provide,  "PROVIDE",     ebuild);

			have_onetime_info = true;
		}
		if(!ok) {
			string *cachefile = ebuild_exec->make_cachefile(full_path.c_str(), directory_path, *pkg, *version);
			if(cachefile) {
				flat_get_keywords_slot_iuse_restrict(cachefile->c_str(), keywords, version->slotname, iuse, restr, m_error_callback);
				flat_read_file(cachefile->c_str(), pkg, m_error_callback);
				ebuild_exec->delete_cachefile();
			}
			else
				m_error_callback(eix::format("Could not properly execute %s") % full_path);
		}

		version->set_full_keywords(keywords);
		version->set_restrict(restr);
		version->set_iuse(iuse);
		pkg->addVersionFinalize(version);

		free(ver);
	}

	if(!have_onetime_info) {
		vec.deletePackage(pkg_name);
	}
}

bool ParseCache::readCategory(Category &vec) throw(ExBasic)
{
	vector<string> packages;

	string catpath = m_prefix + m_scheme + '/' + vec.name;
	if(!scandir_cc(catpath, packages, package_selector))
		return false;

	for(vector<string>::const_iterator pit = packages.begin();
		pit != packages.end(); ++pit) {
		vector<string> files;
		string pkg_path = catpath + '/' + (*pit);

		if(scandir_cc(pkg_path, files, ebuild_selector))
			readPackage(vec, *pit, pkg_path, files);
	}
	return true;
}
