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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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
#include <portage/overlay.h>

#include <database/header.h>
#include <database/package_reader.h>

#include <eixTk/auto_ptr.h>

#include <dirent.h>

using namespace std;

/** Read a string of the format { unsigned short len; char[] string;
 * (without the 0) } */
std::string
io::read_string(FILE *fp)
{
	io::Short len = io::Short(io::read<io::Char>(io::Charsize, fp));
	if(len == 0xFF)
		len = io::Short(io::read<io::Short>(io::Shortsize, fp));
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	fread(static_cast<void *>(buf.get()), sizeof(char), len, (fp));
	return std::string(buf.get());
}

/** Write a string in the format { unsigned short len; char[] string;
 * (without the 0) } */
void
io::write_string(FILE *fp, const std::string &str)
{
	io::Short len = io::Short(str.size());
	if(len < 0xFF)
		io::write<io::Char>(io::Charsize, fp, io::Char(len));
	else {
		io::write<io::Char>(io::Charsize, fp, io::Char(0xFF));
		io::write<io::Short>(io::Shortsize, fp, len);
	}
	fwrite(static_cast<const void *>(str.c_str()), sizeof(char), len, (fp));
}

LeadNum io::read_LeadNum(FILE *fp)
{
	return LeadNum(io::read_string(fp).c_str());
}

Version *
io::read_version(FILE *fp)
{
	Version *v = new Version();

#if defined(SAVE_VERSIONTEXT)
	// read full version string
	v->m_full = io::read_string(fp);
#else
	v->m_garbage = io::read_string(fp);
#endif

	// read masking
	v->maskflags.set(io::read<MaskFlags::MaskType>(MaskFlags::MaskTypesize, fp));
	v->full_keywords = io::read_string(fp);

	// read primary version part
	for(io::Char i = io::read<io::Char>(io::Charsize, fp);
		i;
		--i)
	{
		v->m_primsplit.push_back(io::read_LeadNum(fp));
	}

	v->m_primarychar = io::read<BasicVersion::Primchar>(BasicVersion::Primcharsize, fp);

	// read m_suffix
	for(io::Char i = io::read<io::Char>(io::Charsize, fp);
		i > 0;
		--i)
	{
		Suffix::Level l = io::read<Suffix::Level>(Suffix::Levelsize, fp);
		LeadNum       n = io::read_LeadNum(fp);
		v->m_suffix.push_back(Suffix(l, n));
	}

	// read m_gentoorevision
	v->m_gentoorevision= io::read_LeadNum(fp);

	v->slot = io::read_string(fp);
	v->overlay_key = io::read<Version::Overlay>(Version::Overlaysize, fp);

	v->set_iuse(io::read_string(fp));
	//v->save_maskflags(Version::SAVEMASK_FILE);// This is done in package_reader
	return v;
}

void
io::write_LeadNum(FILE *fp, const LeadNum &n)
{
	io::write_string(fp, n.represent());
}

void
io::write_version(FILE *fp, const Version *v)
{
#if defined(SAVE_VERSIONTEXT)
	// write m_full string
	io::write_string(fp, v->m_full);
#else
	io::write_string(fp, v->m_garbage);
#endif

	// write masking
	io::write<MaskFlags::MaskType>(MaskFlags::MaskTypesize, fp, v->maskflags.get());

	// write full keywords unless small database is required
	io::write_string(fp, v->full_keywords);

	// write m_primsplit
	io::write<io::Char>(io::Charsize, fp, io::Char(v->m_primsplit.size()));

	for(vector<LeadNum>::const_iterator it = v->m_primsplit.begin();
		it != v->m_primsplit.end();
		++it)
	{
		io::write_LeadNum(fp, *it);
	}

	io::write<BasicVersion::Primchar>(BasicVersion::Primcharsize, fp, v->m_primarychar);

	// write m_suffix
	io::write<io::Char>(io::Charsize, fp, io::Char(v->m_suffix.size()));
	for(vector<Suffix>::const_iterator it = v->m_suffix.begin();
		it != v->m_suffix.end();
		++it)
	{
		io::write<Suffix::Level>(Suffix::Levelsize, fp, it->m_suffixlevel);
		io::write_LeadNum(fp, it->m_suffixnum);
	}

	// write m_gentoorevision
	io::write_LeadNum(fp, v->m_gentoorevision);

	io::write_string(fp, v->slot);
	io::write<Version::Overlay>(Version::Overlaysize, fp, v->overlay_key);
	io::write_string(fp, v->get_iuse());
}

io::Treesize
io::read_category_header(FILE *fp, std::string &name)
{
	name = io::read_string(fp);
	return io::read<io::Treesize>(io::Treesizesize, fp);
}

void
io::write_category_header(FILE *fp, const std::string &name, io::Treesize size)
{
	io::write_string(fp, name);
	io::write<io::Treesize>(io::Treesizesize, fp, size);
}


void
io::write_package(FILE *fp, const Package &pkg)
{
	off_t offset_position = ftello(fp);
	fseek(fp, PackageReader::Offsetsize, SEEK_CUR);

	io::write_string(fp, pkg.name);
	io::write_string(fp, pkg.desc);
	io::write_string(fp, pkg.provide);
	io::write_string(fp, pkg.homepage);
	io::write_string(fp, pkg.licenses);
#if defined(NOT_FULL_USE)
	io::write_string(fp, pkg.coll_iuse);
#else
	io::write_string(fp, "");
#endif

	// write all version entries
	io::write<io::Versize>(io::Versizesize, fp, pkg.size());

	for(Package::const_iterator i = pkg.begin();
		i != pkg.end();
		++i)
	{
		io::write_version(fp, *i);
	}

	off_t pkg_end = ftello(fp);
	fseek(fp, offset_position, SEEK_SET);
	off_t v = (pkg_end - offset_position);
	io::write<PackageReader::Offset>(PackageReader::Offsetsize, fp, v);
	fseek(fp, pkg_end, SEEK_SET);
}


void
io::write_header(FILE *fp, const DBHeader &hdr)
{
	io::write<DBHeader::DBVersion>(DBHeader::DBVersionsize, fp, DBHeader::current);
	io::write<io::Catsize>(io::Catsizesize, fp, hdr.size);

	io::write<Version::Overlay>(Version::Overlaysize, fp, hdr.countOverlays());
	for(Version::Overlay i = 0; i != hdr.countOverlays(); i++) {
		const OverlayIdent& overlay = hdr.getOverlay(i);
		io::write_string(fp, overlay.path);
		io::write_string(fp, overlay.label);
	}
}

void
io::read_header(FILE *fp, DBHeader &hdr)
{
	hdr.version = io::read<DBHeader::DBVersion>(DBHeader::DBVersionsize, fp);
	hdr.size    = io::read<io::Catsize>(io::Catsizesize, fp);

	Version::Overlay overlay_sz = io::read<Version::Overlay>(Version::Overlaysize, fp);
	while(overlay_sz--) {
		string path = io::read_string(fp);
		hdr.addOverlay(OverlayIdent(path.c_str(), io::read_string(fp).c_str()));
	}
}

void
io::write_packagetree(FILE *fp, const PackageTree &tree)
{
	for(PackageTree::const_iterator ci = tree.begin(); ci != tree.end(); ++ci)
	{
		// Write category-header followed by a list of the packages.
		io::write_category_header(fp, ci->name(), io::Treesize(ci->size()));

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
io::read_packagetree(FILE *fp, io::Treesize size, PackageTree &tree)
{
	PackageReader reader(fp, size);
	Package *p = NULL;

	while(reader.next())
	{
		p = reader.release();
		tree[p->category].push_back(p);
	}
}

#if defined(INSTANTIATE_TEMPLATES)
template <io::Char>  read<io::Char>  (const unsigned short size, FILE *fp);
template <io::Short> read<io::Short> (const unsigned short size, FILE *fp);
template <io::Int>   read<io::Int>   (const unsigned short size, FILE *fp);
template <io::Long>  read<io::Long>  (const unsigned short size, FILE *fp);
template write<io::Char> (const unsigned short size, FILE *fp,<io::Char>  t);
template write<io::Short>(const unsigned short size, FILE *fp,<io::Short> t);
template write<io::Int>  (const unsigned short size, FILE *fp,<io::Int>   t);
template write<io::Long> (const unsigned short size, FILE *fp,<io::Long>  t);
#endif
