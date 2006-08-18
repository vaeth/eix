

/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "eixcache.h"

#include <portage/package.h>
#include <portage/version.h>
#include <database/package_reader.h>
#include <database/header.h>
#include <portage/packagetree.h>

#include <config.h>

#include <string>
#include <sstream>

using namespace std;

const char *EixCache::getType() const
{
	string ret="eix";
//cout << "FILENAME <" << m_file << "> overlay: " << m_get_overlay << "\n";
	if(m_file.length())
	{
		ret = ret + ": " + m_file;
	}
	if(m_only_overlay)
	{
		stringstream ss;
		ss << m_get_overlay;
		string num;
		ss >> num;
		ret = ret + " [" + num + "]";
	}
	return ret.c_str();
}

int EixCache::readCategory(Category &vec) throw(ExBasic)
{
	const char *file = EIX_CACHEFILE;
	if(m_file.length())
		file = m_file.c_str();
	FILE *fp = fopen(file, "rb");
	if(!fp) {
		throw ExBasic("Can't read cache file %s: %s",
	              file, strerror(errno));
	}

	DBHeader header;

	io::read_header(fp, header);
	if(!header.isCurrent()) {
		fclose(fp);
		throw ExBasic("Cache file %s uses an obsoleteformat (%i current is %i)",
	              file, header.version, DBHeader::current);
	}

	for(PackageReader reader(fp, header.size); reader.next(); reader.skip())
	{
		reader.read(PackageReader::NAME);
		Package *p = reader.get();
		if (p->category != vec.name()) // wrong category
			continue;
		reader.read(PackageReader::VERSIONS);
		p = reader.get();
		bool have_onetime_info = false;
		Package *pkg = vec.findPackage(p->name);
		if(pkg == NULL)
			pkg = vec.addPackage(p->name);
		else
			have_onetime_info = true;
		for(Package::iterator it = p->begin();
			it != p->end(); ++it)
		{
			if(m_only_overlay)
			{
				if(it->overlay_key != m_get_overlay)
					continue;
			}
			Version *version = new Version(it->getFull());
			version->overlay_key = m_overlay_key;
			version->set(m_arch, it->get_full_keywords());
			version->slot = it->slot;
			pkg->addVersion(version);
			if(*(pkg->latest()) == *version)
			{
				pkg->homepage = p->homepage;
				pkg->licenses = p->licenses;
				pkg->desc     = p->desc;
				pkg->provide  = p->provide;
				have_onetime_info = true;
			}
		}
		if(!have_onetime_info)
			vec.deletePackage(p->name);
	}
	fclose(fp);
	return 0;
}
