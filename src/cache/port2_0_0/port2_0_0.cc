// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "port2_0_0.h"
#include <cache/common/flat-reader.h>

#include <portage/packagetree.h>
#include <portage/package.h>
#include <portage/version.h>

#include <eixTk/stringutils.h>

#include <dirent.h>

#include <config.h>

/* Path to portage cache */
#define PORTAGE_CACHE_PATH "/var/cache/edb/dep"

using namespace std;

static int cachefiles_selector (SCANDIR_ARG3 dent)
{
	return (dent->d_name[0] != '.'
			&& strchr(dent->d_name, '-') != 0);
}

bool Port2_0_0_Cache::readCategory(Category &vec) throw(ExBasic)
{
	string catpath = m_prefix + PORTAGE_CACHE_PATH + m_scheme + vec.name();
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
			m_error_callback("Can't split '%s' into package and version.", dents[i]->d_name);
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
			flat_get_keywords_slot_iuse_restrict(catpath + "/" + dents[i]->d_name, keywords, version->slot, iuse, restr, m_error_callback);
			version->set_full_keywords(keywords);
			version->set_iuse(iuse);
			version->set_restrict(restr);
			version->overlay_key = m_overlay_key;

			pkg->addVersion(version);
			if(*(pkg->latest()) == *version)
				newest=version;

			/* Free old split */
			free(aux[0]);
			free(aux[1]);
			memset(aux, '\0', sizeof(char*) * 2);

			/* If this is the last file we break so we can get the full
			 * information after this while-loop. If we still have more files
			 * ahead we can just read the next file. */
			if(++i == numfiles)
				break;

			/* Split new filename into package and version, and catch any errors. */
			aux = ExplodeAtom::split(dents[i]->d_name);
			if(!aux) {
				m_error_callback("Can't split %s into package and version.", dents[i]->d_name);
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
