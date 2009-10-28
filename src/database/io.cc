// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "io.h"
#include <config.h>
#include <database/header.h>
#include <database/package_reader.h>
#include <database/types.h>
#include <eixTk/auto_ptr.h>
#include <eixTk/exceptions.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <portage/basicversion.h>
#include <portage/extendedversion.h>
#include <portage/keywords.h>
#include <portage/overlay.h>
#include <portage/package.h>
#include <portage/packagetree.h>
#include <portage/version.h>

#include <list>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdio>

using namespace std;

io::OffsetType io::counter;

namespace io {
/// Read a number with leading zero's
static BasicPart read_Part(FILE *fp);

/// Write a number with leading zero's
static void write_Part(FILE *fp, const BasicPart &n);
}

/** Read a string */
string
io::read_string(FILE *fp)
{
	string::size_type len(io::read<string::size_type>(fp));
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	if(unlikely(fread(static_cast<void *>(buf.get()), sizeof(char), len, fp) != len)) {
		if (feof(fp))
			throw ExBasic(_("error while reading from database: end of file"));
		else
			throw SysError(_("error while reading from database"));
	}
	return string(buf.get());
}

/** Write a string */
void
io::write_string(FILE *fp, const string &str)
{
	io::write<string::size_type>(fp, str.size());
	if(fp != NULL) {
		if(unlikely(fwrite(static_cast<const void *>(str.c_str()), sizeof(char), str.size(), fp) != str.size())) {
			throw SysError(_("error while writing to database"));
		}
	}
	else
		counter += str.size();
}

void
io::write_hash_words(FILE *fp, const StringHash& hash, const vector<string>& words)
{
	io::write<vector<string>::size_type>(fp, words.size());
	for(vector<string>::const_iterator i(words.begin()); likely(i != words.end()); ++i)
		io::write_hash_string(fp, hash, *i);
}

string
io::read_hash_words(FILE *fp, const StringHash& hash)
{
	string r;
	for(vector<string>::size_type e(io::read<vector<string>::size_type>(fp));
		likely(e != 0); --e) {
		if(!r.empty())
			r.append(1, ' ');
		r.append(io::read_hash_string(fp, hash));
	}
	return r;
}

static BasicPart
io::read_Part(FILE *fp)
{
	string::size_type len(io::read<string::size_type>(fp));
	BasicPart::PartType type(BasicPart::PartType(len % BasicPart::max_type));
	len /= BasicPart::max_type;
	if(len != 0) {
		eix::auto_list<char> buf(new char[len + 1]);
		buf.get()[len] = 0;
		if(unlikely(fread(static_cast<void *>(buf.get()), sizeof(char), len, fp) != len)) {
			if (feof(fp))
				throw ExBasic(_("error while reading from database: end of file"));
			else
				throw SysError(_("error while reading from database"));
		}
		return BasicPart(type, string(buf.get()));
	}
	return BasicPart(type);
}

void
io::read_iuse(FILE *fp, const StringHash& hash, IUseSet &iuse)
{
	iuse.clear();
	for(io::UNumber e = io::read<io::UNumber>(fp); e; --e)
		iuse.insert_fast(io::read_hash_string(fp, hash));
}

Version *
io::read_version(FILE *fp, const DBHeader &hdr)
{
	Version *v(new Version());

	// read masking
	MaskFlags::MaskType mask(io::read<MaskFlags::MaskType>(fp));
	v->maskflags.set(mask & MaskFlags::MASK_SAVE);
	v->propertiesFlags = io::readUChar(fp);
	v->restrictFlags   = io::read<ExtendedVersion::Restrict>(fp);
	v->full_keywords   = io::read_hash_words(fp, hdr.keywords_hash);

	// read primary version part
	for(list<BasicPart>::size_type i(io::read<list<BasicPart>::size_type>(fp));
		likely(i != 0); --i) {
		v->m_parts.push_back(io::read_Part(fp));
	}

	v->slotname = io::read_hash_string(fp, hdr.slot_hash);
	v->overlay_key = io::read<Version::Overlay>(fp);

	io::read_iuse(fp, hdr.iuse_hash, v->iuse);

	//v->save_maskflags(Version::SAVEMASK_FILE);// This is done in package_reader
	return v;
}

static void
io::write_Part(FILE *fp, const BasicPart &n)
{
	const string &content = n.partcontent;
	io::write<string::size_type>(fp, content.size()*BasicPart::max_type + string::size_type(n.parttype));
	if(!content.empty()) {
		if(fp != NULL) {
			if(unlikely(fwrite(static_cast<const void *>(content.c_str()),
						sizeof(char), content.size(), fp) != content.size())) {
				throw SysError(_("error while writing to database"));
			}
		}
		else
			counter += content.size();
	}
}

void
io::write_version(FILE *fp, const Version *v, const DBHeader &hdr)
{
	// write masking
	io::writeUChar(fp, (v->maskflags.get()) & MaskFlags::MASK_SAVE);
	io::writeUChar(fp, v->propertiesFlags);
	io::write<ExtendedVersion::Restrict>(fp, v->restrictFlags);

	// write full keywords
	io::write_hash_words(fp, hdr.keywords_hash, v->get_full_keywords());

	// write m_primsplit
	io::write<list<BasicPart>::size_type>(fp, v->m_parts.size());

	for(list<BasicPart>::const_iterator it(v->m_parts.begin());
		likely(it != v->m_parts.end()); ++it) {
		io::write_Part(fp, *it);
	}

	io::write_hash_string(fp, hdr.slot_hash, v->slotname);
	io::write<Version::Overlay>(fp, v->overlay_key);
	io::write_hash_words(fp, hdr.iuse_hash, v->iuse.asVector());
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
	io::write_hash_words(fp, hdr.provide_hash, pkg.provide);
	io::write_string(fp, pkg.homepage);
	io::write_hash_string(fp, hdr.license_hash, pkg.licenses);
#ifdef NOT_FULL_USE
	io::write_hash_words(fp, hdr.iuse_hash, pkg.coll_iuse);
#else
	io::write_hash_words(fp, hdr.iuse_hash, std::vector<std::string>());
#endif

	// write all version entries
	io::write<io::Versize>(fp, pkg.size());

	for(Package::const_iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
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
	StringHash::size_type e(hash.size());
	io::write<StringHash::size_type>(fp, e);
	for(StringHash::const_iterator i(hash.begin()); likely(i != hash.end()); ++i) {
		io::write_string(fp, *i);
	}
}

void
io::read_hash(FILE *fp, StringHash& hash)
{
	hash.init(false);
	for(StringHash::size_type i(io::read<StringHash::size_type>(fp));
		likely(i != 0); --i) {
		hash.store_string(io::read_string(fp));
	}
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
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);
		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			hdr.provide_hash.hash_words(p->provide);
			hdr.license_hash.hash_string(p->licenses);
#ifdef NOT_FULL_USE
			hdr.iuse_hash.hash_words(p->coll_iuse);
#endif
			for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
				hdr.keywords_hash.hash_words(v->get_full_keywords());
				hdr.iuse_hash.hash_words(v->iuse.asVector());
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
	for(Version::Overlay i(0); likely(i != hdr.countOverlays()); ++i) {
		const OverlayIdent& overlay = hdr.getOverlay(i);
		io::write_string(fp, overlay.path);
		io::write_string(fp, overlay.label);
	}
	io::write_hash(fp, hdr.provide_hash);
	io::write_hash(fp, hdr.license_hash);
	io::write_hash(fp, hdr.keywords_hash);
	io::write_hash(fp, hdr.iuse_hash);
	io::write_hash(fp, hdr.slot_hash);

	io::write<vector<string>::size_type>(fp, hdr.world_sets.size());
	for(vector<string>::const_iterator it(hdr.world_sets.begin());
		likely(it != hdr.world_sets.end()); ++it)
		io::write_string(fp, *it);
}

bool
io::read_header(FILE *fp, DBHeader &hdr)
{
	hdr.version = io::read<DBHeader::DBVersion>(fp);
	if(unlikely(!hdr.isCurrent()))
		return false;

	hdr.size    = io::read<io::Catsize>(fp);

	for(Version::Overlay overlay_sz(io::read<Version::Overlay>(fp));
		likely(overlay_sz != 0); --overlay_sz) {
		string path(io::read_string(fp));
		hdr.addOverlay(OverlayIdent(path.c_str(), io::read_string(fp).c_str()));
	}

	io::read_hash(fp, hdr.provide_hash);
	io::read_hash(fp, hdr.license_hash);
	io::read_hash(fp, hdr.keywords_hash);
	io::read_hash(fp, hdr.iuse_hash);
	io::read_hash(fp, hdr.slot_hash);

	for(vector<string>::size_type sets_sz(io::read<vector<string>::size_type>(fp));
		likely(sets_sz != 0); --sets_sz)
		hdr.world_sets.push_back(io::read_string(fp));
	return true;
}

void
io::write_packagetree(FILE *fp, const PackageTree &tree, const DBHeader &hdr)
{
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);

		// Write category-header followed by a list of the packages.
		io::write_category_header(fp, c->first, io::Treesize(ci->size()));

		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			// write package to fp
			io::write_package(fp, **p, hdr);
		}
	}
}

void
io::read_packagetree(FILE *fp, PackageTree &tree, const DBHeader &hdr, PortageSettings *ps)
{
	PackageReader reader(fp, hdr, ps);
	while(reader.nextCategory()) {
		Category &cat = tree[reader.category()];
		while (reader.nextPackage()) {
			cat.push_back(reader.release());
		}
	}
}
