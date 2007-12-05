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

#include "io.h"

using namespace std;

io::OffsetType io::counter;

/** Read a string */
string
io::read_string(FILE *fp)
{
	string::size_type len = io::read<string::size_type>(fp);
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	fread(static_cast<void *>(buf.get()), sizeof(char), len, fp);
	return string(buf.get());
}

/** Write a string */
void
io::write_string(FILE *fp, const string &str)
{
	io::write<string::size_type>(fp, str.size());
	if(fp)
		fwrite(static_cast<const void *>(str.c_str()), sizeof(char), str.size(), fp);
	else
		counter += str.size();
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
	MaskFlags::MaskType mask = io::read<MaskFlags::MaskType>(fp);
	v->maskflags.set(mask & MaskFlags::MaskType(0x0F));
	v->restrictFlags = (ExtendedVersion::Restrict(mask >> 4) & ExtendedVersion::Restrict(0x0F));
	v->full_keywords = io::read_string(fp);

	// read primary version part
	for(size_t i = io::read<vector<LeadNum>::size_type>(fp); i; --i) {
		v->m_primsplit.push_back(io::read_LeadNum(fp));
	}

	v->m_primarychar = io::readUChar(fp);

	// read m_suffix
	for(size_t i = io::read<vector<Suffix>::size_type>(fp); i; --i) {
		Suffix::Level l = io::read<Suffix::Level>(fp);
		LeadNum       n = io::read_LeadNum(fp);
		v->m_suffix.push_back(Suffix(l, n));
	}

	// read m_gentoorevision
	v->m_gentoorevision= io::read_LeadNum(fp);

	v->slotname = io::read_string(fp);
	v->overlay_key = io::read<Version::Overlay>(fp);

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
	io::writeUChar(fp, (v->maskflags.get()) | (MaskFlags::MaskType(v->restrictFlags) << 4));

	// write full keywords unless small database is required
	io::write_string(fp, v->full_keywords);

	// write m_primsplit
	io::write<vector<LeadNum>::size_type>(fp, v->m_primsplit.size());

	for(vector<LeadNum>::const_iterator it = v->m_primsplit.begin();
		it != v->m_primsplit.end();
		++it)
	{
		io::write_LeadNum(fp, *it);
	}

	io::writeUChar(fp, v->m_primarychar);

	// write m_suffix
	io::write<vector<Suffix>::size_type>(fp, v->m_suffix.size());
	for(vector<Suffix>::const_iterator it = v->m_suffix.begin();
		it != v->m_suffix.end();
		++it)
	{
		io::write<Suffix::Level>(fp, it->m_suffixlevel);
		io::write_LeadNum(fp, it->m_suffixnum);
	}

	// write m_gentoorevision
	io::write_LeadNum(fp, v->m_gentoorevision);

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
