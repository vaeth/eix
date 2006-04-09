/***************************************************************************
 *   eix fp a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program fp free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program fp distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>

#include <database/header.h>
#include <database/package_reader.h>

#include <eixTk/auto_ptr.h>

#include <dirent.h>
#include <unistd.h>

using namespace std;

/** Read a string of the format { unsigned short len; char[] string;
 * (without the 0) } */
std::string
io::read_string(FILE *fp)
{
	unsigned short len = read<short>(fp);
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	fread((void*)(buf.get()), sizeof(char), len, (fp));
	return std::string(buf.get());
}

/** Write a string in the format { unsigned short len; char[] string;
 * (without the 0) } */
void
io::write_string(FILE *fp, const std::string &str)
{
	unsigned short len = str.size();
	write<short>(fp, len);
	fwrite((const void*)(str.c_str()), sizeof(char), len, (fp));
}

Version *
io::read_version(FILE *fp)
{
	Version *v = new Version();

	// read full version string
	v->m_full = io::read_string(fp);

	// read stability & masking
	v->m_mask = io::read<Keywords::Type>(fp);

	// read primary version part
	for(unsigned char i = io::read<unsigned char>(fp);
		i > 0;
		--i)
	{
		v->m_primsplit.push_back(io::read<unsigned short>(fp));
	}

	v->m_primarychar = io::read<unsigned char>(fp);

	// read m_suffixlevel,m_suffixnum,m_gentoorevision
	v->m_suffixlevel   = io::read<unsigned char>(fp);
	v->m_suffixnum     = io::read<unsigned int>(fp);
	v->m_gentoorevision = io::read<unsigned char>(fp);

	v->overlay_key = io::read<short>(fp);

	return v;
}

void
io::write_version(FILE *fp, const Version *v)
{
	// write m_full string
	io::write_string(fp, v->m_full);

	// write stability & masking
	io::write<Keywords::Type>(fp, v->m_mask);

	// write m_primsplit
	io::write<unsigned char>(fp, (unsigned char)v->m_primsplit.size());

	for(vector<unsigned short>::const_iterator it = v->m_primsplit.begin();
		it != v->m_primsplit.end();
		++it)
	{
		io::write<unsigned short>(fp, *it);
	}

	io::write<unsigned char>(fp, v->m_primarychar);

	// write m_suffixlevel,m_suffixnum,m_gentoorevision
	io::write<unsigned char>(fp, v->m_suffixlevel);
	io::write<unsigned int>(fp, v->m_suffixnum);
	io::write<unsigned char>(fp, v->m_gentoorevision);

	io::write<short>(fp, v->overlay_key);
}

unsigned int
io::read_category_header(FILE *fp, std::string &name)
{
	name = io::read_string(fp);
	return io::read<unsigned int>(fp);
}

void
io::write_category_header(FILE *fp, const std::string &name, unsigned int size)
{
	io::write_string(fp,  name);
	io::write<unsigned int>(fp, size);
}


void
io::write_package(FILE *fp, const Package &pkg)
{
	off_t offset_position = ftello(fp);
	fseek(fp, sizeof(PackageReader::offset_type), SEEK_CUR);

	io::write_string(fp, pkg.name);
	io::write_string(fp, pkg.desc);
	io::write_string(fp, pkg.provide);
	io::write_string(fp, pkg.homepage);
	io::write_string(fp, pkg.licenses);

	// write all version entries
	io::write<PackageReader::size_type>(fp, pkg.size());

	for(Package::const_iterator i = pkg.begin();
		i != pkg.end();
		++i)
	{
		io::write_version(fp, *i);
	}

	off_t pkg_end = ftello(fp);
	fseek(fp, offset_position, SEEK_SET);
	off_t v = (pkg_end - offset_position);
	io::write<PackageReader::offset_type>(fp, v);
	fseek(fp, pkg_end, SEEK_SET);
}


void
io::write_header(FILE *fp, const DBHeader &hdr)
{
	io::write(fp, DBHeader::current);
	io::write<int>(fp, hdr.size);

	io::write<short>(fp, hdr.countOverlays());
	for(int i = 0; i<hdr.countOverlays(); i++)
		io::write_string(fp, hdr.getOverlay(i));
}

void
io::read_header(FILE *fp, DBHeader &hdr)
{
	hdr.version = io::read<int>(fp);
	hdr.size = io::read<int>(fp);

	unsigned short overlay_sz = io::read<short>(fp);
	while(overlay_sz--)
		hdr.addOverlay(io::read_string(fp));
}

void
io::write_packagetree(FILE *fp, const PackageTree &tree)
{
	for(PackageTree::const_iterator ci = tree.begin(); ci != tree.end(); ++ci)
	{
		// Write category-header followed by a list of the packages.
		io::write_category_header(fp, ci->name(), ci->size());

		for(Category::iterator p = ci->begin();
			p != ci->end();
			++p)
		{
			// write package to fp
			io::write_package(fp, **p);
		}
	}
}

void
io::read_packagetree(FILE *fp, unsigned int size, PackageTree &tree)
{
	PackageReader reader(fp, size);
	Package *p = NULL;

	while(reader.next())
	{
		p = reader.release();
		tree[p->category].push_back(p);
	}
}
