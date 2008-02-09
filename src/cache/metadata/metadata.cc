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
#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <dirent.h>

#include <config.h>

using namespace std;

/* Path to portage cache */
#define METADATA_PATH "/metadata/cache/"

static int cachefiles_selector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strchr(dent->d_name, '-') != 0);
}

bool MetadataCache::readCategory(Category &vec) throw(ExBasic)
{
	string catpath = m_prefix + m_scheme + METADATA_PATH + vec.name();
	struct dirent **dents;
	int numfiles = my_scandir(catpath.c_str(), &dents, cachefiles_selector, alphasort);
	char **aux = NULL;

	for(int i=0; i<numfiles;)
	{
		Version *version;
		Version *newest = NULL;

		/* Split string into package and version, and catch any errors. */
		aux = ExplodeAtom::split(dents[i]->d_name);
		if(aux == NULL)
		{
			m_error_callback(string("Can't split '") +
				dents[i]->d_name + "' into package and version");
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
				m_error_callback(string("Can't split '") + dents[i]->d_name
					+ "' into package and version");
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
		for(int i=0; i<numfiles; i++ )
			free(dents[i]);
		free(dents);
	}
	return true;
}
