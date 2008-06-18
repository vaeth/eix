// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "port2_1_2.h"
#include <cache/common/unpickle.h>

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <eixTk/stringutils.h>
#include <eixTk/formated.h>

#include <config.h>

using namespace std;

/* Path to portage cache */
#define PORTAGE_PICKLE "/var/cache/edb/vdb_metadata.pickle";

class MapFile {
	private:
		void *pf_data;
		off_t pf_data_size;

		bool mapData(int fd) {
			struct stat st;
			if (fstat(fd,&st) == 0) {
				void *x = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
				if (x != MAP_FAILED) {
					pf_data_size = st.st_size;
					pf_data = x;
					return true;
				}
			}
			return false;
		}

	public:
		MapFile(const char *file) {
			pf_data = NULL;
			pf_data_size = 0;
			int fd = open(file, O_RDONLY);
			if(fd == -1) {
				return;
			}
			if( ! mapData(fd))  {
				close(fd);
				return;
			}
			close(fd);
		}

		~MapFile() {
			if(pf_data != NULL) {
				munmap(pf_data, pf_data_size);
			}
		}

		bool isReady(const char **data, const char **end) {
			if(!pf_data)
				return false;
			*data = static_cast<const char *>(pf_data);
			*end = (static_cast<const char *>(pf_data)) + pf_data_size;
			return true;
		}
};

bool
Port2_1_2_Cache::readEntry(map<string,string> &mapper, PackageTree *packagetree, std::vector<std::string> *categories, Category *category)
{
	UNUSED(categories);
	string catstring = mapper["KEY"];
	if(catstring.empty())
		return false;
	string::size_type pos = catstring.find_first_of('/');
	if(pos == string::npos) {
		m_error_callback(eix::format("%r not of the form package/catstring-version") % catstring);
		return false;
	}
	string name_ver = catstring.substr(pos + 1);
	catstring.resize(pos);
	// Does the category match?
	// Currently, we do not add non-matching categories with this method.
	Category *dest_cat;
	if(category) {
		dest_cat = category;
		if(dest_cat->name != catstring)
			return false;
	}
	else {
		dest_cat = packagetree->find(catstring);
		if(!dest_cat)
			return false;
	}
	char **aux = ExplodeAtom::split(name_ver.c_str());
	if(aux == NULL)
	{
		m_error_callback(eix::format("Can't split %r into package and version") % name_ver);
		return false;
	}
	/* Search for existing package */
	Package *pkg = dest_cat->findPackage(aux[0]);

	/* If none was found create one */
	if(pkg == NULL)
		pkg = dest_cat->addPackage(aux[0]);

	/* Create a new version and add it to package */
	Version *version = new Version(aux[1]);
	// reading slots and stability
	version->slotname = mapper["SLOT"];
	version->set_full_keywords(mapper["KEYWORDS"]);
	version->set_iuse(mapper["IUSE"]);
	version->set_restrict(mapper["RESTRICT"]);
	pkg->addVersion(version);

	/* For the newest version, add all remaining data */
	if(*(pkg->latest()) == *version)
	{
		pkg->homepage = mapper["HOMEPAGE"];
		pkg->licenses = mapper["LICENSES"];
		pkg->desc     = mapper["DESCRIPTION"];
		pkg->provide  = mapper["PROVIDE"];
	}
	/* Free old split */
	free(aux[0]);
	free(aux[1]);
	return true;
}

bool Port2_1_2_Cache::readCategories(PackageTree *packagetree, std::vector<std::string> *categories, Category *category) throw(ExBasic)
{
	string filename = m_prefix + PORTAGE_PICKLE;
	const char *data, *end;
	map<string,string> unpickled;

	MapFile picklefile(filename.c_str());
	if( ! picklefile.isReady(&data, &end) )
	{
		m_error_callback(eix::format("Can't read cache file %s") % filename);
		return true;
	}
	if(category)
	{
		packagetree = NULL;
		categories = NULL;
	}
	if(packagetree)
		packagetree->need_fast_access(categories);
	try {
		Unpickler unpickler(data, end);
		while(! unpickler.is_finished())
		{
			unpickler.get(unpickled);
			readEntry(unpickled, packagetree, categories, category);
		}
	}
	catch(const ExBasic &e) {
		cerr << "Problems with " << filename <<
			":\n" << e << endl;
		if(packagetree)
			packagetree->finish_fast_access();
		return false;
	}
	if(packagetree)
		packagetree->finish_fast_access();
	return true;
}

