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

void
io::write_hash_words(FILE *fp, const StringHash& hash, const vector<string>& words)
{
	io::write<vector<string>::size_type>(fp, words.size());
	for(vector<string>::const_iterator i = words.begin(); i != words.end(); ++i)
		io::write_hash_string(fp, hash, *i);
}

string
io::read_hash_words(FILE *fp, const StringHash& hash)
{
	string r;
	for(vector<string>::size_type e = io::read<vector<string>::size_type>(fp);
		e ; --e) {
		if(!r.empty())
			r.append(" ");
		r.append(io::read_hash_string(fp, hash));
	}
	return r;
}

BasicVersion::Part
io::read_Part(FILE *fp)
{
	string::size_type len = io::read<string::size_type>(fp);
	BasicVersion::PartType type(BasicVersion::PartType(len % BasicVersion::max_type));
	len /= BasicVersion::max_type;
	if(len > 0) {
		eix::auto_list<char> buf(new char[len + 1]);
		buf.get()[len] = 0;
		if(fread(static_cast<void *>(buf.get()), sizeof(char), len, fp) != len) {
			cerr << "unexpected end of file" << endl;
		}
		return BasicVersion::Part(type, string(buf.get()));
	}
	return BasicVersion::Part(type, "");
}

Version *
io::read_version(FILE *fp, const DBHeader &hdr)
{
	Version *v = new Version();

	// read masking
	MaskFlags::MaskType mask = io::read<MaskFlags::MaskType>(fp);
	v->maskflags.set(mask & MaskFlags::MaskType(0x0F));
	v->restrictFlags = (ExtendedVersion::Restrict(mask >> 4) & ExtendedVersion::Restrict(0x0F));
	v->full_keywords = io::read_hash_words(fp, hdr.keywords_hash);

	// read primary version part
	for(list<BasicVersion::Part>::size_type i = io::read<list<BasicVersion::Part>::size_type>(fp);
		i; --i) {
		v->m_parts.push_back(io::read_Part(fp));
	}

	v->slotname = io::read_hash_string(fp, hdr.slot_hash);
	v->overlay_key = io::read<Version::Overlay>(fp);

	io::read_hash_container(fp, hdr.iuse_hash, std::inserter(v->m_iuse, v->m_iuse.end()));
	// we can assume the list is ordered and unique, no sort_uniquify needed

	//v->save_maskflags(Version::SAVEMASK_FILE);// This is done in package_reader
	return v;
}

void
io::write_Part(FILE *fp, const BasicVersion::Part &n)
{
	io::write<string::size_type>(fp, n.second.size()*BasicVersion::max_type + n.first);
	if(n.second.size() > 0) {
		if(fp) {
			if(fwrite(static_cast<const void *>(n.second.c_str()),
						sizeof(char), n.second.size(), fp) != n.second.size()) {
				cerr << "write error" << endl;
			}
		}
		else
			counter += n.second.size();
	}
}

void
io::write_version(FILE *fp, const Version *v, const DBHeader &hdr)
{
	// write masking
	io::writeUChar(fp, (v->maskflags.get()) | (MaskFlags::MaskType(v->restrictFlags) << 4));

	// write full keywords unless small database is required
	io::write_hash_words(fp, hdr.keywords_hash, split_string(v->get_full_keywords()));

	// write m_primsplit
	io::write<list<BasicVersion::Part>::size_type>(fp, v->m_parts.size());

	for(list<BasicVersion::Part>::const_iterator it = v->m_parts.begin();
		it != v->m_parts.end();
		++it)
	{
		io::write_Part(fp, *it);
	}

	io::write_hash_string(fp, hdr.slot_hash, v->slotname);
	io::write<Version::Overlay>(fp, v->overlay_key);
	io::write_hash_words(fp, hdr.iuse_hash, v->m_iuse);
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
io::write_package_pure(FILE *fp, const Package &pkg, const DBHeader &hdr)
{
	io::write_string(fp, pkg.name);
	io::write_string(fp, pkg.desc);
	io::write_hash_string(fp, hdr.provide_hash, pkg.provide);
	io::write_string(fp, pkg.homepage);
	io::write_hash_string(fp, hdr.license_hash, pkg.licenses);
#if defined(NOT_FULL_USE)
	io::write_hash_words(fp, hdr.iuse_hash, pkg.coll_iuse);
#else
	io::write_hash_words(fp, hdr.iuse_hash, std::vector<std::string>());
#endif

	// write all version entries
	io::write<io::Versize>(fp, pkg.size());

	for(Package::const_iterator i = pkg.begin();
		i != pkg.end();
		++i)
	{
		io::write_version(fp, *i, hdr);
	}
}

void
io::write_package(FILE *fp, const Package &pkg, const DBHeader &hdr)
{
	counter = 0;
	write_package_pure(NULL, pkg, hdr);
	io::write<io::OffsetType>(fp, counter);
	write_package_pure(fp, pkg, hdr);
}

void
io::write_hash(FILE *fp, const StringHash& hash)
{
	StringHash::size_type e = hash.size();
	io::write<StringHash::size_type>(fp, e);
	for(StringHash::const_iterator i = hash.begin(); i != hash.end(); ++i)
		io::write_string(fp, *i);
}

void
io::read_hash(FILE *fp, StringHash& hash)
{
	hash.init(false);
	for(StringHash::size_type i = io::read<StringHash::size_type>(fp);
		i; --i)
		hash.store_string(io::read_string(fp));
	hash.finalize();
}

void
io::prep_header_hashs(DBHeader &hdr, const PackageTree &tree)
{
	hdr.provide_hash.init(true);
	hdr.license_hash.init(true);
	hdr.keywords_hash.init(true);
	hdr.slot_hash.init(true);
	hdr.iuse_hash.init(true);
	for(PackageTree::const_iterator ci = tree.begin(); ci != tree.end(); ++ci)
	{
		for(Category::iterator p = ci->begin();
			p != ci->end(); ++p)
		{
			hdr.provide_hash.hash_string(p->provide);
			hdr.license_hash.hash_string(p->licenses);
#if defined(NOT_FULL_USE)
			hdr.iuse_hash.hash_words(p->coll_iuse);
#endif
			for(Package::iterator v = p->begin();
				v != p->end(); ++v) {
				hdr.keywords_hash.hash_words(v->get_full_keywords());
				hdr.iuse_hash.hash_words(v->iuse());
				hdr.slot_hash.hash_string(v->slotname);
			}
		}
	}
	hdr.provide_hash.finalize();
	hdr.license_hash.finalize();
	hdr.keywords_hash.finalize();
	hdr.slot_hash.finalize();
	hdr.iuse_hash.finalize();
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
	io::write_hash(fp, hdr.provide_hash);
	io::write_hash(fp, hdr.license_hash);
	io::write_hash(fp, hdr.keywords_hash);
	io::write_hash(fp, hdr.iuse_hash);
	io::write_hash(fp, hdr.slot_hash);
}

bool
io::read_header(FILE *fp, DBHeader &hdr)
{
	hdr.version = io::read<DBHeader::DBVersion>(fp);
	if(!hdr.isCurrent())
		return false;

	hdr.size    = io::read<io::Catsize>(fp);

	for(Version::Overlay overlay_sz = io::read<Version::Overlay>(fp);
		overlay_sz; --overlay_sz) {
		string path = io::read_string(fp);
		hdr.addOverlay(OverlayIdent(path.c_str(), io::read_string(fp).c_str()));
	}
	io::read_hash(fp, hdr.provide_hash);
	io::read_hash(fp, hdr.license_hash);
	io::read_hash(fp, hdr.keywords_hash);
	io::read_hash(fp, hdr.iuse_hash);
	io::read_hash(fp, hdr.slot_hash);
	return true;
}

void
io::write_packagetree(FILE *fp, const PackageTree &tree, const DBHeader &hdr)
{
	for(PackageTree::const_iterator ci = tree.begin(); ci != tree.end(); ++ci)
	{
		// Write category-header followed by a list of the packages.
		io::write_category_header(fp, ci->name, io::Treesize(ci->size()));

		for(Category::iterator p = ci->begin();
			p != ci->end();
			++p)
		{
			// write package to fp
			io::write_package(fp, **p, hdr);
		}
	}
}

void
io::read_packagetree(FILE *fp, PackageTree &tree, const DBHeader &hdr)
{
	PackageReader reader(fp, hdr);
	Package *p = NULL;

	while(reader.next())
	{
		p = reader.release();
		tree[p->category].push_back(p);
	}
}
