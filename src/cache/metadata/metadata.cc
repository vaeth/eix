// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "metadata.h"
#include <cache/common/flat_reader.h>
#include <cache/common/assign_reader.h>

#include <eixTk/utils.h>
#include <portage/package.h>
#include <portage/packagetree.h>

#include <dirent.h>

using namespace std;

/* Subpath to metadata cache */
#define METADATA_PATH "metadata/cache"
/* Path to portage-2.0 cache */
#define PORTAGE_CACHE_PATH "var/cache/edb/dep"

bool
MetadataCache::initialize(const string &name)
{
	string pure_name = name;
	string::size_type i = pure_name.find_first_of(':');
	if(i != string::npos) {
		pure_name.erase(i);
		have_override_path = true;
		override_path = name.substr(i + 1);
	}
	else
		have_override_path = false;
	if((strcasecmp(pure_name.c_str(), "metadata") == 0) ||
		(strcasecmp(pure_name.c_str(), "metadata-flat") == 0)) {
		setType(PATH_METADATA, true);
		return true;
	}
	if((strcasecmp(pure_name.c_str(), "metadata*") == 0) ||
		(strcasecmp(pure_name.c_str(), "metadata-assign") == 0)) {
		setType(PATH_METADATA, false);
		return true;
	}
	if((strcasecmp(pure_name.c_str(), "flat") == 0) ||
		(strcasecmp(pure_name.c_str(), "portage-2.0") == 0) ||
		(strcasecmp(pure_name.c_str(), "portage-2.0.51") == 0)) {
		setType(PATH_FULL, true);
		return true;
	}
	if((strcasecmp(pure_name.c_str(), "assign") == 0) ||
		(strcasecmp(pure_name.c_str(), "backport") == 0) ||
		(strcasecmp(pure_name.c_str(), "portage-2.1") == 0) ||
		(strcasecmp(pure_name.c_str(), "portage-2.1.0") == 0)) {
		setType(PATH_FULL, false);
		return true;
	}
	if(strcasecmp(pure_name.c_str(), "repo-flat") == 0) {
		setType(PATH_REPOSITORY, true);
		return true;
	}
	if(strcasecmp(pure_name.c_str(), "repo-assign") == 0) {
		setType(PATH_REPOSITORY, false);
		return true;
	}
	return false;
}

void
MetadataCache::setType(PathType set_path_type, bool set_flat)
{
	path_type = set_path_type;
	switch(set_path_type) {
		case PATH_METADATA:
			m_type = "metadata-";
			break;
		case PATH_REPOSITORY:
			m_type = "repo-";
			break;
		// case PATH_FULL:
		default:
			m_type.clear();
	}
	flat = set_flat;
	if(set_flat)
		m_type.append("flat");
	else
		m_type.append("assign");
	if(have_override_path) {
		m_type.append(":");
		m_type.append(override_path);
	}
}

static int
cachefiles_selector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strchr(dent->d_name, '-') != 0);
}

typedef void (*x_get_keywords_slot_iuse_restrict_t)(const string &filename, string &keywords, string &slotname, string &iuse, string &restr, string &props, BasicCache::ErrorCallback error_callback);
typedef void (*x_read_file_t)(const char *filename, Package *pkg, BasicCache::ErrorCallback error_callback);

bool
MetadataCache::readCategory(Category &vec) throw(ExBasic)
{
	string catpath;
	if(have_override_path) {
		catpath = override_path;
	}
	else {
		catpath = m_prefix;
		if(path_type == PATH_METADATA) {
			// m_scheme is actually the portdir
			catpath.append(m_scheme);
			optional_append(catpath, '/');
			catpath.append(METADATA_PATH);
		}
		else {
		// path_type == PATH_REPOSITORY || path_type == PATH_FULL
			optional_append(catpath, '/');
			catpath.append(PORTAGE_CACHE_PATH);
		}
	}
	switch(path_type) {
		case PATH_FULL:
			catpath.append(m_scheme);
			break;
		case PATH_REPOSITORY:
			optional_append(catpath, '/');
			if(m_overlay_name.empty()) {
				// Paludis' way of resolving missing repo_name:
				catpath.append("x-");
				string::size_type p = m_scheme.size();
				while(p) {
					string::size_type c = m_scheme.rfind('/', p);
					if(c == string::npos) {
						catpath.append(m_scheme.substr(0,p));
						break;
					}
					if(c == --p)
						continue;
					catpath.append(m_scheme.substr(c + 1, p - c));
					break;
				}
			}
			else {
				catpath.append(m_overlay_name);
			}
			break;
		default:
		// case PATH_METADATA:
			break;
	}
	optional_append(catpath, '/');
	catpath.append(vec.name);
	vector<string> names;
	if(!scandir_cc(catpath, names, cachefiles_selector))
		return false;

	x_get_keywords_slot_iuse_restrict_t x_get_keywords_slot_iuse_restrict;
	x_read_file_t x_read_file;
	if(flat) {
		x_get_keywords_slot_iuse_restrict = flat_get_keywords_slot_iuse_restrict;
		x_read_file = flat_read_file;
	}
	else {
		x_get_keywords_slot_iuse_restrict = assign_get_keywords_slot_iuse_restrict;
		x_read_file = assign_read_file;
	}
	for(vector<string>::const_iterator it = names.begin();
		it != names.end(); )
	{
		Version *version;
		Version *newest = NULL;

		/* Split string into package and version, and catch any errors. */
		char **aux = ExplodeAtom::split(it->c_str());
		if(!aux) {
			m_error_callback(eix::format("Can't split %r into package and version") % (*it));
			++it;
			continue;
		}

		/* Search for existing package */
		Package *pkg = vec.findPackage(aux[0]);

		/* If none was found create one */
		if(!pkg)
			pkg = vec.addPackage(aux[0]);

		do {
			/* Make version and add it to package. */
			version = new Version(aux[1]);

			/* Read stability from cachefile */
			string keywords, iuse, restr, props;
			(*x_get_keywords_slot_iuse_restrict)(catpath + "/" + (*it), keywords, version->slotname, iuse, restr, props, m_error_callback);
			version->set_full_keywords(keywords);
			version->set_iuse(iuse);
			version->set_restrict(restr);
			version->set_properties(props);
			version->overlay_key = m_overlay_key;

			pkg->addVersion(version);
			if(*(pkg->latest()) == *version)
				newest = version;

			/* Free old split */
			free(aux[0]);
			free(aux[1]);

			/* If this is the last file we break so we can get the full
			 * information after this while-loop. If we still have more files
			 * ahead we can just read the next file. */
			if(++it == names.end())
				break;

			/* Split new filename into package and version, and catch any errors. */
			aux = ExplodeAtom::split(it->c_str());
			if(!aux) {
				m_error_callback(eix::format("Can't split %r into package and version") % (*it));
				++it;
				break;
			}
		} while(!strcmp(aux[0], pkg->name.c_str()));

		/* Read the cache file of the last version completely */
		if(newest) // provided we have read the "last" version
			(*x_read_file)((catpath + "/" + pkg->name + "-" + newest->getFull()).c_str(), pkg, m_error_callback);
	}

	return true;
}
