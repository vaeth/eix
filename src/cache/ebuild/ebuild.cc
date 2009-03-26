// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "ebuild.h"
#include <cache/common/selectors.h>
#include <cache/common/flat_reader.h>

#include <portage/package.h>
#include <portage/packagetree.h>

using namespace std;

void
EbuildCache::readPackage(Category &vec, const string &pkg_name, const string &directory_path, const vector<string> &files) throw(ExBasic)
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
		string::size_type pos = ebuild_pos(*it);
		if(pos == string::npos)
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

		/* Execute the external program to generate cachefile */
		string full_path = directory_path + '/' + (*it);
		string *cachefile = ebuild_exec.make_cachefile(full_path.c_str(), directory_path, *pkg, *version);
		if(!cachefile) {
			m_error_callback(eix::format("Could not properly execute %s") % full_path);
			continue;
		}

		/* For the latest version read/change corresponding data */
		bool read_onetime_info = true;
		if(have_onetime_info) {
			if(*(pkg->latest()) != *version)
				read_onetime_info = false;
		}
		version->overlay_key = m_overlay_key;
		string keywords, iuse, restr, props;
		try {
			flat_get_keywords_slot_iuse_restrict(cachefile->c_str(), keywords, version->slotname, iuse, restr, props, m_error_callback);
			version->set_full_keywords(keywords);
			version->set_iuse(iuse);
			version->set_restrict(restr);
			version->set_properties(props);
			if(read_onetime_info) {
				flat_read_file(cachefile->c_str(), pkg, m_error_callback);
				have_onetime_info = true;
			}
		}
		catch(const ExBasic &e) {
			m_error_callback(eix::format("Executing %s did not produce all data") % full_path);
			// We keep the version anyway, even with wrong keywords/slots/infos:
			have_onetime_info = true;
		}
		pkg->addVersionFinalize(version);
		free(ver);
		ebuild_exec.delete_cachefile();
	}

	if(!have_onetime_info)
		vec.deletePackage(pkg_name);
}

bool
EbuildCache::readCategory(Category &vec) throw(ExBasic)
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

