// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "metadata.h"
#include <cache/common/flat-reader.h>

#include <eixTk/stringutils.h>
#include <eixTk/formated.h>
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <dirent.h>

#include <config.h>

using namespace std;

/* Subpath to metadata cache *with* trailing / */
#define METADATA_PATH "/metadata/cache/"
/* Path to portage-2.0 cache *without* trailing / (since scheme will be appended) */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"

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
	if(strcasecmp(pure_name.c_str(), "metadata") == 0) {
		metadata = true;
		return true;
	}
	if((strcasecmp(pure_name.c_str(), "flat") == 0) ||
		(strcasecmp(pure_name.c_str(), "portage-2.0") == 0) ||
		(strcasecmp(pure_name.c_str(), "portage-2.0.51") == 0)) {
		metadata = false;
		return true;
	}
	return false;
}

const char *
MetadataCache::getType() const
{
	static string s;
	if(metadata)
		s = "metadata";
	else
		s = "flat";
	if(have_override_path) {
		s.append(":");
		s.append(override_path);
	}
	return s.c_str();
}

static int cachefiles_selector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strchr(dent->d_name, '-') != 0);
}

bool MetadataCache::readCategory(Category &vec) throw(ExBasic)
{
	string catpath;
	if(have_override_path) {
		catpath = override_path;
		if(!metadata)
			catpath.append(m_scheme);
	}
	else {
		catpath = m_prefix;
		if(metadata) {
			// m_scheme is actually the portdir
			catpath.append(m_scheme);
			catpath.append(METADATA_PATH);
		}
		else {
			catpath.append(PORTAGE_CACHE_PATH);
			catpath.append(m_scheme);
		}
	}
	if(catpath.empty() || (*(catpath.rbegin()) != '/'))
		catpath.append("/");
	catpath.append(vec.name);
	struct dirent **dents;
	int numfiles = my_scandir(catpath.c_str(), &dents, cachefiles_selector, alphasort);
	char **aux = NULL;

	for(int i = 0; i < numfiles; )
	{
		Version *version;
		Version *newest = NULL;

		/* Split string into package and version, and catch any errors. */
		aux = ExplodeAtom::split(dents[i]->d_name);
		if(aux == NULL)
		{
			m_error_callback(eix::format("Can't split %r into package and version") % dents[i]->d_name);
			++i;
			continue;
		}

		/* Search for existing package */
		Package *pkg = vec.findPackage(aux[0]);

		/* If none was found create one */
		if(pkg == NULL)
			pkg = vec.addPackage(aux[0]);

		do {
			/* Make version and add it to package. */
			version = new Version(aux[1]);

			/* Read stability from cachefile */
			string keywords, iuse, restr;
			flat_get_keywords_slot_iuse_restrict(catpath + "/" + dents[i]->d_name, keywords, version->slotname, iuse, restr, m_error_callback);
			version->set_full_keywords(keywords);
			version->set_iuse(iuse);
			version->set_restrict(restr);
			version->overlay_key = m_overlay_key;

			pkg->addVersion(version);
			if(*(pkg->latest()) == *version)
				newest = version;

			/* If this is the last file we break so we can get the full
			 * information after this while-loop. If we still have more files
			 * ahead we can just read the next file. */
			if(++i == numfiles)
				break;

			/* Free old split */
			free(aux[0]);
			free(aux[1]);

			/* Split new filename into package and version, and catch any errors. */
			aux = ExplodeAtom::split(dents[i]->d_name);
			if(!aux) {
				m_error_callback(eix::format("Can't split %r into package and version") % dents[i]->d_name);
				break;
			}
		} while(strcmp(aux[0], pkg->name.c_str()) == 0);
		if(aux)
		{
			free(aux[0]);
			free(aux[1]);
		}

		/* Read the cache file of the last version completely */
		if(newest) // provided we have read the "last" version
			flat_read_file(string(catpath + "/" + pkg->name + "-" + newest->getFull()).c_str(), pkg, m_error_callback);
	}

	if(numfiles > 0)
	{
		for(int i = 0; i < numfiles; i++ )
			free(dents[i]);
		free(dents);
	}
	return true;
}
