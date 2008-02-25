// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <portage/package.h>
#include <portage/version.h>
#include <portage/packagetree.h>
#include <portage/overlay.h>

#include <database/header.h>
#include <database/package_reader.h>

#include <eixTk/auto_ptr.h>

#include <dirent.h>
#include <iostream>

#include "io.h"

using namespace std;

io::OffsetType io::counter;

namespace io {
	/// Read a number with leading zero's
	BasicVersion::Part read_Part(FILE *fp);

	/// Write a number with leading zero's
	void write_Part(FILE *fp, const BasicVersion::Part &n);
}

/** Read a string */
string
io::read_string(FILE *fp)
{
	string::size_type len = io::read<string::size_type>(fp);
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	if(fread(static_cast<void *>(buf.get()), sizeof(char), len, fp) != len) {
		cerr << "unexpected end of file" << endl;
	}
	return string(buf.get());
}

/** Write a string */
void
io::write_string(FILE *fp, const string &str)
{
	io::write<string::size_type>(fp, str.size());
	if(fp) {
		if(fwrite(static_cast<const void *>(str.c_str()), sizeof(char), str.size(), fp) != str.size()) {
			cerr << "write error" << endl;
		}
	}
	else
		counter += str.size();
}

BasicVersion::Part io::read_Part(FILE *fp)
{
	unsigned char c = io::read<unsigned char>(fp);
	return std::make_pair(BasicVersion::PartType(c), io::read_string(fp));
}

Version *
io::read_version(FILE *fp)
{
	Version *v = new Version();

	// read masking
	MaskFlags::MaskType mask = io::read<MaskFlags::MaskType>(fp);
	v->maskflags.set(mask & MaskFlags::MaskType(0x0F));
	v->restrictFlags = (ExtendedVersion::Restrict(mask >> 4) & ExtendedVersion::Restrict(0x0F));
	v->full_keywords = io::read_string(fp);

	// read version parts
	for(size_t i = io::read<vector<BasicVersion::Part>::size_type>(fp); i; --i) {
		v->m_parts.push_back(io::read_Part(fp));
	}

	v->slotname = io::read_string(fp);
	v->overlay_key = io::read<Version::Overlay>(fp);

	v->set_iuse(io::read_string(fp));
	//v->save_maskflags(Version::SAVEMASK_FILE);// This is done in package_reader
	return v;
}

void
io::write_Part(FILE *fp, const BasicVersion::Part &n)
{
	io::write<unsigned char>(fp, n.first);
	io::write_string(fp, n.second);
}

void
io::write_version(FILE *fp, const Version *v)
{
	// write masking
	io::writeUChar(fp, (v->maskflags.get()) | (MaskFlags::MaskType(v->restrictFlags) << 4));

	// write full keywords unless small database is required
	io::write_string(fp, v->full_keywords);

	// write version parts
	io::write<vector<BasicVersion::Part>::size_type>(fp, v->m_parts.size());

	for(vector<BasicVersion::Part>::const_iterator it = v->m_parts.begin();
			it != v->m_parts.end();
			++it)
	{
		io::write_Part(fp, *it);
	}

	io::write_string(fp, v->slotname);
	io::write<Version::Overlay>(fp, v->overlay_key);
	io::write_string(fp, v->get_iuse());
}

io::Treesize
io::read_category_header(FILE *fp, string &name)
{
	name = io::read_string(fp);
	return io::read<io::Treesize>(fp);
}

void
io::write_category_header(FILE *fp, const string &name, io::Treesize size)
{
	io::write_string(fp, name);
	io::write<io::Treesize>(fp, size);
}


void
io::write_package_pure(FILE *fp, const Package &pkg)
{
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
	io::write<io::Versize>(fp, pkg.size());

	for(Package::const_iterator i = pkg.begin();
		i != pkg.end();
		++i)
	{
		io::write_version(fp, *i);
	}
}

void
io::write_package(FILE *fp, const Package &pkg)
{
	counter = 0;
	write_package_pure(NULL, pkg);
	io::write<io::OffsetType>(fp, counter);
	write_package_pure(fp, pkg);
}

void
io::write_header(FILE *fp, const DBHeader &hdr)
{
	io::write<DBHeader::DBVersion>(fp, DBHeader::current);
	io::write<io::Catsize>(fp, hdr.size);

	io::write<Version::Overlay>(fp, hdr.countOverlays());
	for(Version::Overlay i = 0; i != hdr.countOverlays(); i++) {
		const OverlayIdent& overlay = hdr.getOverlay(i);
		io::write_string(fp, overlay.path);
		io::write_string(fp, overlay.label);
	}
}

void
io::read_header(FILE *fp, DBHeader &hdr)
{
	hdr.version = io::read<DBHeader::DBVersion>(fp);
	hdr.size    = io::read<io::Catsize>(fp);

	Version::Overlay overlay_sz = io::read<Version::Overlay>(fp);
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
