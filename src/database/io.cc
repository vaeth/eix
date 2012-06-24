// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "io.h"
#include <database/header.h>
#include <database/package_reader.h>
#include <database/types.h>
#include <eixTk/auto_ptr.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <eixTk/stringutils.h>
#include <portage/basicversion.h>
#include <portage/depend.h>
#include <portage/extendedversion.h>
#include <portage/keywords.h>
#include <portage/overlay.h>
#include <portage/package.h>
#include <portage/packagetree.h>
#include <portage/version.h>

#include <list>
#include <string>
#include <vector>

#include <cstdio>

using namespace std;

io::OffsetType io::counter;

namespace io {
/// Read a number with leading zero's
static bool
read_Part(BasicPart &b, FILE *fp, string *errtext);

inline static bool
read_string_plain(char *s, string::size_type len, FILE *fp, string *errtext)
{
	if(likely(fread(s, sizeof(char), len, fp) == len)) {
		return true;
	}
	readError(fp, errtext);
	return false;
}

inline static bool
write_string_plain(const string &str, FILE *fp, string *errtext)
{
	if(fp == NULLPTR) {
		io::counter += str.size();
		return true;
	}
	if(likely(fwrite(static_cast<const void *>(str.c_str()), sizeof(char), str.size(), fp) == str.size())) {
		return true;
	}
	writeError(errtext);
	return false;
}

/// Write a number with leading zero's
static bool write_Part(const BasicPart &n, FILE *fp, string *errtext);
}

void
io::readError(FILE *fp, string *errtext)
{
	if(errtext != NULLPTR) {
		*errtext = (feof(fp) ?
			_("error while reading from database: end of file") :
			_("error while reading from database"));
	}
}

void
io::writeError(string *errtext)
{
	if(errtext != NULLPTR) {
		*errtext = _("error while writing to database");
	}
}

/** Read a string */
bool
io::read_string(string &s, FILE *fp, string *errtext)
{
	string::size_type len;
	if(unlikely(!io::read_num(len, fp, errtext))) {
		return false;
	}
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	if(likely(io::read_string_plain(buf.get(), len, fp, errtext))) {
		s = buf.get();
		return true;
	}
	return false;
}

/** Write a string */
bool
io::write_string(const string &str, FILE *fp, string *errtext)
{
	return (likely(io::write_num(str.size(), fp, errtext)) &&
		likely(io::write_string_plain(str, fp, errtext)));
}

bool
io::write_hash_words(const StringHash& hash, const vector<string>& words, FILE *fp, string *errtext)
{
	if(unlikely(!io::write_num(words.size(), fp, errtext))) {
		return false;
	}
	for(vector<string>::const_iterator i(words.begin()); likely(i != words.end()); ++i) {
		if(unlikely(!io::write_hash_string(hash, *i, fp, errtext))) {
			return false;
		}
	}
	return true;
}

bool
io::read_hash_words(const StringHash& hash, vector<string> &s, FILE *fp, string *errtext)
{
	vector<string>::size_type e;
	if(unlikely(!io::read_num(e, fp, errtext))) {
		return false;
	}
	s.resize(e);
	for(vector<string>::size_type i(0); likely(i != e); ++i) {
		if(unlikely(!io::read_hash_string(hash, s[i], fp, errtext))) {
			return false;
		}
	}
	return true;
}

bool
io::read_hash_words(const StringHash& hash, string &s, FILE *fp, string *errtext)
{
	s.clear();
	vector<string>::size_type e;
	if(unlikely(!io::read_num(e, fp, errtext))) {
		return false;
	}
	for(; e; --e) {
		string r;
		if(unlikely(!io::read_hash_string(hash, r, fp, errtext))) {
			return false;
		}
		if(!s.empty())
			s.append(1, ' ');
		s.append(r);
	}
	return true;
}

static bool
io::read_Part(BasicPart &b, FILE *fp, string *errtext)
{
	string::size_type len;
	if(unlikely(!io::read_num(len, fp, errtext))) {
		return false;
	}
	BasicPart::PartType type(BasicPart::PartType(len % BasicPart::max_type));
	len /= BasicPart::max_type;
	if(len != 0) {
		eix::auto_list<char> buf(new char[len + 1]);
		buf.get()[len] = 0;
		if(unlikely(!io::read_string_plain(buf.get(), len, fp, errtext))) {
			return false;
		}
		b = BasicPart(type, string(buf.get()));
		return true;
	}
	b = BasicPart(type);
	return true;
}

bool
io::read_iuse(const StringHash& hash, IUseSet &iuse, FILE *fp, string *errtext)
{
	iuse.clear();
	io::UNumber e;
	if(unlikely(!io::read_num(e, fp, errtext))) {
		return false;
	}
	for(; e; --e) {
		string s;
		if(unlikely(!io::read_hash_string(hash, s, fp, errtext))) {
			return false;
		}
		iuse.insert_fast(s);
	}
	return true;
}

bool
io::read_version(Version *&v, const DBHeader &hdr, FILE *fp, string *errtext)
{
	v = new Version();

	// read masking
	MaskFlags::MaskType mask;
	if(unlikely(!io::read_num(mask, fp, errtext))) {
		return false;
	}
	v->maskflags.set(mask);
	if(unlikely(!io::readUChar(v->propertiesFlags, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::read_num(v->restrictFlags, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::read_hash_words(hdr.keywords_hash, v->full_keywords, fp, errtext))) {
		return false;
	}

	// read primary version part
	list<BasicPart>::size_type i;
	if(unlikely(!io::read_num(i, fp, errtext))) {
		return false;
	}
	for(; likely(i != 0); --i) {
		BasicPart b;
		if(unlikely(!io::read_Part(b, fp, errtext))) {
			return false;
		}
		v->m_parts.push_back(b);
	}

	string fullslot;
	if(unlikely(!io::read_hash_string(hdr.slot_hash, fullslot, fp, errtext))) {
		return false;
	}
	v->set_slotname(fullslot);
	if(unlikely(!io::read_num(v->overlay_key, fp, errtext))) {
		return false;
	}
	v->reponame = hdr.getOverlay(v->overlay_key).label;

	if(unlikely(!io::read_iuse(hdr.iuse_hash, v->iuse, fp, errtext))) {
		return false;
	}

	if(hdr.use_depend) {
		if(unlikely(!io::read_depend(v->depend, hdr, fp, errtext))) {
			return false;
		}
	}

	//v->save_maskflags(Version::SAVEMASK_FILE);// This is done in package_reader
	return true;
}

static bool
io::write_Part(const BasicPart &n, FILE *fp, string *errtext)
{
	const string &content(n.partcontent);
	if(unlikely(!io::write_num(content.size()*BasicPart::max_type + string::size_type(n.parttype), fp, errtext))) {
		return false;
	}
	if(!content.empty()) {
		if(unlikely(!io::write_string_plain(content, fp, errtext))) {
			return false;
		}
	}
	return true;
}

bool
io::write_version(const Version *v, const DBHeader &hdr, FILE *fp, string *errtext)
{
	// write masking
	if(unlikely(!io::writeUChar(v->maskflags.get(), fp, errtext))) {
		return false;
	}
	if(unlikely(!io::writeUChar(v->propertiesFlags, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_num(v->restrictFlags, fp, errtext))) {
		return false;
	}

	// write full keywords
	if(unlikely(!io::write_hash_words(hdr.keywords_hash, v->get_full_keywords(), fp, errtext))) {
		return false;
	}

	// write m_primsplit
	if(unlikely(!io::write_num(v->m_parts.size(), fp, errtext))) {
		return false;
	}

	for(list<BasicPart>::const_iterator it(v->m_parts.begin());
		likely(it != v->m_parts.end()); ++it) {
		if(unlikely(!io::write_Part(*it, fp, errtext))) {
			return false;
		}
	}

	if(unlikely(!io::write_hash_string(hdr.slot_hash, v->get_shortfullslot(), fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_num(v->overlay_key, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_hash_words(hdr.iuse_hash, v->iuse.asVector(), fp, errtext))) {
		return false;
	}
	if(hdr.use_depend) {
		io::OffsetType counter_save(io::counter);
		io::counter = 0;
		io::write_depend(v->depend, hdr, NULLPTR, NULLPTR);
		io::OffsetType counter_diff(io::counter);
		io::counter = counter_save;
		if(unlikely(!io::write_num(counter_diff, fp, errtext))) {
			return false;
		}
		if(unlikely(!io::write_depend(v->depend, hdr, fp, errtext))) {
			return false;
		}
	}
	return true;
}

bool
io::read_depend(Depend &dep, const DBHeader &hdr, FILE *fp, string *errtext)
{
	string::size_type len;
	if(unlikely(!io::read_num(len, fp, errtext))) {
		return false;
	}
	if(Depend::use_depend) {
		if(unlikely(!io::read_hash_words(hdr.depend_hash, dep.m_depend, fp, errtext))) {
			return false;
		}
		if(unlikely(!io::read_hash_words(hdr.depend_hash, dep.m_rdepend, fp, errtext))) {
			return false;
		}
		if(unlikely(!io::read_hash_words(hdr.depend_hash, dep.m_pdepend, fp, errtext))) {
			return false;
		}
	}
	else {
		dep.clear();
#ifdef HAVE_FSEEKO
		fseeko(fp, len, SEEK_CUR);
#else
		fseek(fp, len, SEEK_CUR);
#endif
	}
	return true;
}

bool
io::write_depend(const Depend &dep, const DBHeader &hdr, FILE *fp, string *errtext)
{
	return (likely(io::write_hash_words(hdr.depend_hash, dep.m_depend, fp, errtext)) &&
		likely(io::write_hash_words(hdr.depend_hash, dep.m_rdepend, fp, errtext)) &&
		likely(io::write_hash_words(hdr.depend_hash, dep.m_pdepend, fp, errtext)));
}

bool
io::read_category_header(string &name, io::Treesize &h, FILE *fp, string *errtext)
{
	return (likely(io::read_string(name, fp, errtext)) &&
		likely(io::read_num(h, fp, errtext)));
}

bool
io::write_category_header(const string &name, io::Treesize size, FILE *fp, string *errtext)
{
	return (likely(io::write_string(name, fp, errtext)) &&
		likely(io::write_num(size, fp, errtext)));
}


bool
io::write_package_pure(const Package &pkg, const DBHeader &hdr, FILE *fp, string *errtext)
{
	if(unlikely(!io::write_string(pkg.name, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_string(pkg.desc, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_string(pkg.homepage, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_hash_string(hdr.license_hash, pkg.licenses, fp, errtext))) {
		return false;
	}

	// write all version entries
	if(unlikely(!io::write_num(pkg.size(), fp, errtext))) {
		return false;
	}

	for(Package::const_iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(unlikely(!io::write_version(*i, hdr, fp, errtext))) {
			return false;
		}
	}
	return true;
}

bool
io::write_package(const Package &pkg, const DBHeader &hdr, FILE *fp, string *errtext)
{
	io::OffsetType counter_save(io::counter);
	io::counter = 0;
	io::write_package_pure(pkg, hdr, NULLPTR, NULLPTR);
	io::OffsetType counter_diff(io::counter);
	io::counter = counter_save;
	return (likely(io::write_num(counter_diff, fp, errtext)) &&
		likely(io::write_package_pure(pkg, hdr, fp, errtext)));
}

bool
io::write_hash(const StringHash& hash, FILE *fp, string *errtext)
{
	StringHash::size_type e(hash.size());
	if(unlikely(!io::write_num(e, fp, errtext))) {
		return false;
	}
	for(StringHash::const_iterator i(hash.begin()); likely(i != hash.end()); ++i) {
		if(unlikely(!io::write_string(*i, fp, errtext))) {
			return false;
		}
	}
	return true;
}

bool
io::read_hash(StringHash& hash, FILE *fp, string *errtext)
{
	hash.init(false);
	StringHash::size_type i;
	if(unlikely(!io::read_num(i, fp, errtext))) {
		return false;
	}
	for(; likely(i != 0); --i) {
		string s;
		if(unlikely(!io::read_string(s, fp, errtext))) {
			return false;
		}
		hash.store_string(s);
	}
	hash.finalize();
	return true;
}

void
io::prep_header_hashs(DBHeader &hdr, const PackageTree &tree)
{
	hdr.license_hash.init(true);
	hdr.keywords_hash.init(true);
	hdr.slot_hash.init(true);
	hdr.iuse_hash.init(true);
	bool use_dep(Depend::use_depend);
	hdr.use_depend = use_dep;
	if(use_dep) {
		hdr.depend_hash.init(true);
	}
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);
		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			hdr.license_hash.hash_string(p->licenses);
			for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
				hdr.keywords_hash.hash_words(v->get_full_keywords());
				hdr.iuse_hash.hash_words(v->iuse.asVector());
				hdr.slot_hash.hash_string(v->get_shortfullslot());
				if(use_dep) {
					const Depend &dep(v->depend);
					hdr.depend_hash.hash_words(dep.m_depend);
					hdr.depend_hash.hash_words(dep.m_rdepend);
					hdr.depend_hash.hash_words(dep.m_pdepend);
				}
			}
		}
	}
	hdr.license_hash.finalize();
	hdr.keywords_hash.finalize();
	hdr.slot_hash.finalize();
	hdr.iuse_hash.finalize();
	if(use_dep) {
		hdr.depend_hash.finalize();
	}
}

bool
io::write_header(const DBHeader &hdr, FILE *fp, string *errtext)
{
	if(unlikely(!io::write_string_plain(DBHeader::magic, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_num(DBHeader::current, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_num(hdr.size, fp, errtext))) {
		return false;
	}

	if(unlikely(!io::write_num(hdr.countOverlays(), fp, errtext))) {
		return false;
	}
	for(ExtendedVersion::Overlay i(0); likely(i != hdr.countOverlays()); ++i) {
		const OverlayIdent& overlay(hdr.getOverlay(i));
		if(unlikely(!io::write_string(overlay.path, fp, errtext))) {
			return false;
		}
		if(unlikely(!io::write_string(overlay.label, fp, errtext))) {
			return false;
		}
	}
	if(unlikely(!io::write_hash(hdr.license_hash, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_hash(hdr.keywords_hash, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_hash(hdr.iuse_hash, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::write_hash(hdr.slot_hash, fp, errtext))) {
		return false;
	}

	if(unlikely(!io::write_num(hdr.world_sets.size(), fp, errtext))) {
		return false;
	}
	for(vector<string>::const_iterator it(hdr.world_sets.begin());
		likely(it != hdr.world_sets.end()); ++it) {
		if(unlikely(!io::write_string(*it, fp, errtext))) {
			return false;
		}
	}

	if(hdr.use_depend) {
		if(unlikely(!io::write_num(1, fp, errtext))) {
			return false;
		}
		io::OffsetType counter_save(io::counter);
		io::counter = 0;
		io::write_hash(hdr.depend_hash, NULLPTR, NULLPTR);
		io::OffsetType counter_diff(io::counter);
		io::counter = counter_save;
		return (likely(io::write_num(counter_diff, fp, errtext)) &&
			likely(io::write_hash(hdr.depend_hash, fp, errtext)));
	}
	else {
		return io::write_num(0, fp, errtext);
	}
}

bool
io::read_header(DBHeader &hdr, FILE *fp, string *errtext)
{
	string::size_type magic_len(DBHeader::magic.size());
	eix::auto_list<char> buf(new char[magic_len + 1]);
	buf.get()[magic_len] = 0;
	if(unlikely(!io::read_string_plain(buf.get(), magic_len, fp, errtext))) {
		return false;
	}
	if(unlikely(DBHeader::magic != buf.get())) {
		char c(buf.get()[0]);
		// Until version 30 the first char is the version:
		hdr.version = (((c > 0) && (c <= 30)) ? c : 0);
	}
	else if(unlikely(!io::read_num(hdr.version, fp, errtext))) {
		return false;
	}
	if(unlikely(!hdr.isCurrent())) {
		if(errtext != NULLPTR) {
			*errtext = eix::format((hdr.version > DBHeader::current) ?
			_("cachefile uses newer format %s (current is %s)") :
			_("cachefile uses obsolete format %s (current is %s)"))
			% hdr.version % DBHeader::current;
		}
		return false;
	}

	if(unlikely(!io::read_num(hdr.size, fp, errtext))) {
		return false;
	}

	ExtendedVersion::Overlay overlay_sz;
	if(unlikely(!io::read_num(overlay_sz, fp, errtext))) {
		return false;
	}
	for(; likely(overlay_sz != 0); --overlay_sz) {
		string path;
		if(unlikely(!io::read_string(path, fp, errtext))) {
			return false;
		}
		string ov;
		if(unlikely(!io::read_string(ov, fp, errtext))) {
			return false;
		}
		hdr.addOverlay(OverlayIdent(path.c_str(), ov.c_str()));
	}

	if(unlikely(!io::read_hash(hdr.license_hash, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::read_hash(hdr.keywords_hash, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::read_hash(hdr.iuse_hash, fp, errtext))) {
		return false;
	}
	if(unlikely(!io::read_hash(hdr.slot_hash, fp, errtext))) {
		return false;
	}

	vector<string>::size_type sets_sz;
	if(unlikely(!io::read_num(sets_sz, fp, errtext))) {
		return false;
	}
	for(; likely(sets_sz != 0); --sets_sz) {
		string s;
		if(unlikely(!io::read_string(s, fp, errtext))) {
			return false;
		}
		hdr.world_sets.push_back(s);
	}

	io::UNumber use_dep_num;
	if(unlikely(!io::read_num(use_dep_num, fp, errtext))) {
		return false;
	}
	if((hdr.use_depend = (use_dep_num != 0))) {
		io::OffsetType len;
		if(unlikely(!io::read_num(len, fp, errtext))) {
			return false;
		}
		if(Depend::use_depend) {
			if(unlikely(!io::read_hash(hdr.depend_hash, fp, errtext))) {
				return false;
			}
		}
		else if(len != 0) {
#ifdef HAVE_FSEEKO
			if(unlikely(fseeko(fp, len, SEEK_CUR) != 0))
#else
			if(unlikely(fseek(fp, len, SEEK_CUR) != 0))
#endif
			{
				if(errtext != NULLPTR) {
					*errtext = _("fseek failed");
				}
				return false;
			}
		}
	}
	return true;
}

bool
io::write_packagetree(const PackageTree &tree, const DBHeader &hdr, FILE *fp, string *errtext)
{
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);
		// Write category-header followed by a list of the packages.
		if(unlikely(!io::write_category_header(c->first, io::Treesize(ci->size()), fp, errtext))) {
			return false;
		}

		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			// write package to fp
			if(unlikely(!io::write_package(**p, hdr, fp, errtext))) {
				return false;
			}
		}
	}
	return true;
}

bool
io::read_packagetree(PackageTree &tree, const DBHeader &hdr, PortageSettings *ps, FILE *fp, string *errtext)
{
	PackageReader reader(fp, hdr, ps);
	while(reader.nextCategory()) {
		Category &cat = tree[reader.category()];
		while(reader.nextPackage()) {
			cat.push_back(reader.release());
		}
	}
	const char *c(reader.get_errtext());
	if(likely(c == NULLPTR))
		return true;
	if(errtext != NULLPTR) {
		*errtext = c;
	}
	return false;
}
