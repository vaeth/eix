// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <sys/types.h>

#include <cstdio>
#include <cstring>

#include <list>
#include <string>
#include <vector>

#include "database/header.h"
#include "database/io.h"
#include "database/package_reader.h"
#include "eixTk/auto_ptr.h"
#include "eixTk/diagnostics.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "portage/basicversion.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/keywords.h"
#include "portage/overlay.h"
#include "portage/package.h"
#include "portage/packagetree.h"
#include "portage/version.h"

using std::list;
using std::string;
using std::vector;

bool File::openread(const char *name)
{
	return ((fp = fopen(name, "rb")) != NULLPTR);
}

bool File::openwrite(const char *name)
{
	return ((fp = fopen(name, "wb")) != NULLPTR);
}

File::~File()
{
	if(likely(fp != NULLPTR)) {
		fclose(fp);
	}
}

bool File::seek(eix::OffsetType offset, int whence, std::string *errtext)
{
#ifdef HAVE_FSEEKO
	if(likely(fseeko(fp, offset, whence) == 0))
#else
	if(likely(fseek(fp, offset, whence) == 0))
#endif
		return true;
	if(errtext != NULLPTR) {
		*errtext = _("fseek failed");
	}
	return false;
}

eix::OffsetType
File::tell()
{
#ifdef HAVE_FSEEKO
	// We rely on autoconf whose documentation states:
	// All system with fseeko() also supply ftello()
	return ftello(fp);
#else
	// We want an off_t-addition, so we cast first to be safe:
	return ftell(fp);
#endif
}

bool
File::read_string_plain(char *s, string::size_type len, string *errtext)
{
	if(likely(read(s, len))) {
		return true;
	}
	readError(errtext);
	return false;
}

bool
File::write_string_plain(const string &str, string *errtext)
{
	if(likely(write(str))) {
		return true;
	}
	writeError(errtext);
	return false;
}

void
File::readError(string *errtext)
{
	if(errtext != NULLPTR) {
		*errtext = (feof(fp) ?
			_("error while reading from database: end of file") :
			_("error while reading from database"));
	}
}

void
File::writeError(string *errtext)
{
	if(errtext != NULLPTR) {
		*errtext = _("error while writing to database");
	}
}

bool
Database::readUChar(eix::UChar *c, string *errtext)
{
	int ch(getch());
	if(likely(ch != EOF)) {
		*c = eix::UChar(ch);
		return true;
	}
	readError(errtext);
	return false;
}

bool
Database::writeUChar(eix::UChar c, string *errtext)
{
	if(counting) {
		++counter;
	} else if(unlikely(!putch(c))) {
		writeError(errtext);
		return false;
	}
	return true;
}

bool
Database::write_string_plain(const string &str, string *errtext)
{
	if(counting) {
GCC_DIAG_OFF(sign-conversion)
		counter += str.size();
GCC_DIAG_ON(sign-conversion)
		return true;
	}
	return File::write_string_plain(str, errtext);
}

bool
Database::read_string(string *s, string *errtext)
{
	string::size_type len;
	if(unlikely(!read_num(&len, errtext))) {
		return false;
	}
	eix::auto_list<char> buf(new char[len + 1]);
	buf.get()[len] = 0;
	if(likely(read_string_plain(buf.get(), len, errtext))) {
		*s = buf.get();
		return true;
	}
	return false;
}

bool
Database::write_string(const string &str, string *errtext)
{
	return (likely(write_num(str.size(), errtext)) &&
		likely(write_string_plain(str, errtext)));
}

bool
Database::write_hash_words(const StringHash& hash, const vector<string>& words, string *errtext)
{
	if(unlikely(!write_num(words.size(), errtext))) {
		return false;
	}
	for(vector<string>::const_iterator i(words.begin()); likely(i != words.end()); ++i) {
		if(unlikely(!write_hash_string(hash, *i, errtext))) {
			return false;
		}
	}
	return true;
}

bool
Database::read_hash_words(const StringHash& hash, vector<string> *s, string *errtext)
{
	vector<string>::size_type e;
	if(unlikely(!read_num(&e, errtext))) {
		return false;
	}
	s->resize(e);
	for(vector<string>::size_type i(0); likely(i != e); ++i) {
		if(unlikely(!read_hash_string(hash, &((*s)[i]), errtext))) {
			return false;
		}
	}
	return true;
}

bool
Database::read_hash_words(const StringHash& hash, string *s, string *errtext)
{
	s->clear();
	vector<string>::size_type e;
	if(unlikely(!read_num(&e, errtext))) {
		return false;
	}
	for(; e; --e) {
		string r;
		if(unlikely(!read_hash_string(hash, &r, errtext))) {
			return false;
		}
		if(!s->empty())
			s->append(1, ' ');
		s->append(r);
	}
	return true;
}

bool
Database::read_Part(BasicPart *b, string *errtext)
{
	string::size_type len;
	if(unlikely(!read_num(&len, errtext))) {
		return false;
	}
	BasicPart::PartType type(BasicPart::PartType(len % BasicPart::max_type));
	len /= BasicPart::max_type;
	if(len != 0) {
		eix::auto_list<char> buf(new char[len + 1]);
		buf.get()[len] = 0;
		if(unlikely(!read_string_plain(buf.get(), len, errtext))) {
			return false;
		}
		*b = BasicPart(type, string(buf.get()));
		return true;
	}
	*b = BasicPart(type);
	return true;
}

bool
Database::read_iuse(const StringHash& hash, IUseSet *iuse, string *errtext)
{
	iuse->clear();
	eix::UNumber e;
	if(unlikely(!read_num(&e, errtext))) {
		return false;
	}
	for(; e; --e) {
		string s;
		if(unlikely(!read_hash_string(hash, &s, errtext))) {
			return false;
		}
		iuse->insert_fast(s);
	}
	return true;
}

bool
Database::read_version(Version *v, const DBHeader &hdr, string *errtext)
{
	// read masking
	MaskFlags::MaskType mask;
	if(unlikely(!read_num(&mask, errtext))) {
		return false;
	}
	v->maskflags.set(mask);
	if(unlikely(!readUChar(&(v->propertiesFlags), errtext))) {
		return false;
	}
	if(unlikely(!read_num(&(v->restrictFlags), errtext))) {
		return false;
	}
	if(unlikely(!read_hash_words(hdr.keywords_hash, &(v->full_keywords), errtext))) {
		return false;
	}

	// read primary version part
	list<BasicPart>::size_type i;
	if(unlikely(!read_num(&i, errtext))) {
		return false;
	}
	for(; likely(i != 0); --i) {
		BasicPart b;
		if(unlikely(!read_Part(&b, errtext))) {
			return false;
		}
		v->m_parts.push_back(b);
	}

	string fullslot;
	if(unlikely(!read_hash_string(hdr.slot_hash, &fullslot, errtext))) {
		return false;
	}
	v->set_slotname(fullslot);
	if(unlikely(!read_num(&(v->overlay_key), errtext))) {
		return false;
	}
	v->reponame = hdr.getOverlay(v->overlay_key).label;

	if(unlikely(!read_iuse(hdr.iuse_hash, &(v->iuse), errtext))) {
		return false;
	}

	if(hdr.use_depend) {
		if(unlikely(!read_depend(&(v->depend), hdr, errtext))) {
			return false;
		}
	}

	// v->save_maskflags(Version::SAVEMASK_FILE);  // This is done in package_reader
	return true;
}

bool
Database::write_Part(const BasicPart &n, string *errtext)
{
	const string &content(n.partcontent);
	if(unlikely(!write_num(content.size()*BasicPart::max_type + string::size_type(n.parttype), errtext))) {
		return false;
	}
	if(!content.empty()) {
		if(unlikely(!write_string_plain(content, errtext))) {
			return false;
		}
	}
	return true;
}

bool
Database::write_version(const Version *v, const DBHeader &hdr, string *errtext)
{
	// write masking
	if(unlikely(!writeUChar(v->maskflags.get(), errtext))) {
		return false;
	}
	if(unlikely(!writeUChar(v->propertiesFlags, errtext))) {
		return false;
	}
	if(unlikely(!write_num(v->restrictFlags, errtext))) {
		return false;
	}

	// write full keywords
	if(unlikely(!write_hash_words(hdr.keywords_hash, v->get_full_keywords(), errtext))) {
		return false;
	}

	// write m_primsplit
	if(unlikely(!write_num(v->m_parts.size(), errtext))) {
		return false;
	}

	for(list<BasicPart>::const_iterator it(v->m_parts.begin());
		likely(it != v->m_parts.end()); ++it) {
		if(unlikely(!write_Part(*it, errtext))) {
			return false;
		}
	}

	if(unlikely(!write_hash_string(hdr.slot_hash, v->get_shortfullslot(), errtext))) {
		return false;
	}
	if(unlikely(!write_num(v->overlay_key, errtext))) {
		return false;
	}
	if(unlikely(!write_hash_words(hdr.iuse_hash, v->iuse.asVector(), errtext))) {
		return false;
	}
	if(hdr.use_depend) {
		eix::OffsetType counter_save(counter);
		counter = 0;
		counting = true;
		write_depend(v->depend, hdr, NULLPTR);
		counting = false;
		eix::OffsetType counter_diff(counter);
		counter = counter_save;
		if(unlikely(!write_num(counter_diff, errtext))) {
			return false;
		}
		if(unlikely(!write_depend(v->depend, hdr, errtext))) {
			return false;
		}
	}
	return true;
}

bool
Database::read_depend(Depend *dep, const DBHeader &hdr, string *errtext)
{
	string::size_type len;
	if(unlikely(!read_num(&len, errtext))) {
		return false;
	}
	if(Depend::use_depend) {
		if(unlikely(!read_hash_words(hdr.depend_hash, &(dep->m_depend), errtext))) {
			return false;
		}
		if(unlikely(!read_hash_words(hdr.depend_hash, &(dep->m_rdepend), errtext))) {
			return false;
		}
		if(unlikely(!read_hash_words(hdr.depend_hash, &(dep->m_pdepend), errtext))) {
			return false;
		}
		if(hdr.version == 31) {
			dep->m_hdepend.clear();
		} else if(unlikely(!read_hash_words(hdr.depend_hash, &(dep->m_hdepend), errtext))) {
			return false;
		}
	} else {
		dep->clear();
GCC_DIAG_OFF(sign-conversion)
		if(unlikely(!seekabs(len, errtext))) {
			return false;
		}
GCC_DIAG_ON(sign-conversion)
	}
	return true;
}

bool
Database::write_depend(const Depend &dep, const DBHeader &hdr, string *errtext)
{
	return (likely(write_hash_words(hdr.depend_hash, dep.m_depend, errtext)) &&
		likely(write_hash_words(hdr.depend_hash, dep.m_rdepend, errtext)) &&
		likely(write_hash_words(hdr.depend_hash, dep.m_pdepend, errtext)) &&
		likely(write_hash_words(hdr.depend_hash, dep.m_hdepend, errtext)));
}

bool
Database::read_category_header(string *name, eix::Treesize *h, string *errtext)
{
	return (likely(read_string(name, errtext)) &&
		likely(read_num(h, errtext)));
}

bool
Database::write_category_header(const string &name, eix::Treesize size, string *errtext)
{
	return (likely(write_string(name, errtext)) &&
		likely(write_num(size, errtext)));
}


bool
Database::write_package_pure(const Package &pkg, const DBHeader &hdr, string *errtext)
{
	if(unlikely(!write_string(pkg.name, errtext))) {
		return false;
	}
	if(unlikely(!write_string(pkg.desc, errtext))) {
		return false;
	}
	if(unlikely(!write_string(pkg.homepage, errtext))) {
		return false;
	}
	if(unlikely(!write_hash_string(hdr.license_hash, pkg.licenses, errtext))) {
		return false;
	}

	// write all version entries
	if(unlikely(!write_num(pkg.size(), errtext))) {
		return false;
	}

	for(Package::const_iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(unlikely(!write_version(*i, hdr, errtext))) {
			return false;
		}
	}
	return true;
}

bool
Database::write_package(const Package &pkg, const DBHeader &hdr, string *errtext)
{
	eix::OffsetType counter_save(counter);
	counter = 0;
	counting = true;
	write_package_pure(pkg, hdr, NULLPTR);
	counting = false;
	eix::OffsetType counter_diff(counter);
	counter = counter_save;
	return (likely(write_num(counter_diff, errtext)) &&
		likely(write_package_pure(pkg, hdr, errtext)));
}

bool
Database::write_hash(const StringHash& hash, string *errtext)
{
	StringHash::size_type e(hash.size());
	if(unlikely(!write_num(e, errtext))) {
		return false;
	}
	for(StringHash::const_iterator i(hash.begin()); likely(i != hash.end()); ++i) {
		if(unlikely(!write_string(*i, errtext))) {
			return false;
		}
	}
	return true;
}

bool
Database::read_hash(StringHash *hash, string *errtext)
{
	hash->init(false);
	StringHash::size_type i;
	if(unlikely(!read_num(&i, errtext))) {
		return false;
	}
	for(; likely(i != 0); --i) {
		string s;
		if(unlikely(!read_string(&s, errtext))) {
			return false;
		}
		hash->store_string(s);
	}
	hash->finalize();
	return true;
}

void
Database::prep_header_hashs(DBHeader *hdr, const PackageTree &tree)
{
	hdr->license_hash.init(true);
	hdr->keywords_hash.init(true);
	hdr->slot_hash.init(true);
	hdr->iuse_hash.init(true);
	bool use_dep(Depend::use_depend);
	hdr->use_depend = use_dep;
	if(use_dep) {
		hdr->depend_hash.init(true);
	}
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);
		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			hdr->license_hash.hash_string(p->licenses);
			for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
				hdr->keywords_hash.hash_words(v->get_full_keywords());
				hdr->iuse_hash.hash_words(v->iuse.asVector());
				hdr->slot_hash.hash_string(v->get_shortfullslot());
				if(use_dep) {
					const Depend &dep(v->depend);
					hdr->depend_hash.hash_words(dep.m_depend);
					hdr->depend_hash.hash_words(dep.m_rdepend);
					hdr->depend_hash.hash_words(dep.m_pdepend);
					hdr->depend_hash.hash_words(dep.m_hdepend);
				}
			}
		}
	}
	hdr->license_hash.finalize();
	hdr->keywords_hash.finalize();
	hdr->slot_hash.finalize();
	hdr->iuse_hash.finalize();
	if(use_dep) {
		hdr->depend_hash.finalize();
	}
}

bool
Database::write_header(const DBHeader &hdr, string *errtext)
{
	if(unlikely(!write_string_plain(DBHeader::magic, errtext))) {
		return false;
	}
	if(unlikely(!write_num(DBHeader::current, errtext))) {
		return false;
	}
	if(unlikely(!write_num(hdr.size, errtext))) {
		return false;
	}

	if(unlikely(!write_num(hdr.countOverlays(), errtext))) {
		return false;
	}
	for(ExtendedVersion::Overlay i(0); likely(i != hdr.countOverlays()); ++i) {
		const OverlayIdent& overlay(hdr.getOverlay(i));
		if(unlikely(!write_string(overlay.path, errtext))) {
			return false;
		}
		if(unlikely(!write_string(overlay.label, errtext))) {
			return false;
		}
	}
	if(unlikely(!write_hash(hdr.license_hash, errtext))) {
		return false;
	}
	if(unlikely(!write_hash(hdr.keywords_hash, errtext))) {
		return false;
	}
	if(unlikely(!write_hash(hdr.iuse_hash, errtext))) {
		return false;
	}
	if(unlikely(!write_hash(hdr.slot_hash, errtext))) {
		return false;
	}

	if(unlikely(!write_num(hdr.world_sets.size(), errtext))) {
		return false;
	}
	for(vector<string>::const_iterator it(hdr.world_sets.begin());
		likely(it != hdr.world_sets.end()); ++it) {
		if(unlikely(!write_string(*it, errtext))) {
			return false;
		}
	}

	if(hdr.use_depend) {
		if(unlikely(!write_num(1, errtext))) {
			return false;
		}
		eix::OffsetType counter_save(counter);
		counter = 0;
		counting = true;
		write_hash(hdr.depend_hash, NULLPTR);
		counting = false;
		eix::OffsetType counter_diff(counter);
		counter = counter_save;
		return (likely(write_num(counter_diff, errtext)) &&
			likely(write_hash(hdr.depend_hash, errtext)));
	} else {
		return write_num(0, errtext);
	}
}

bool
Database::read_header(DBHeader *hdr, string *errtext)
{
	size_t magic_len(strlen(DBHeader::magic));
	eix::auto_list<char> buf(new char[magic_len + 1]);
	buf.get()[magic_len] = 0;
	if(unlikely(!read_string_plain(buf.get(), magic_len, errtext))) {
		return false;
	}
	if(unlikely(strcmp(DBHeader::magic, buf.get()) != 0)) {
		char c(buf.get()[0]);
		// Until version 30 the first char is the version:
GCC_DIAG_OFF(sign-conversion)
		hdr->version = (((c > 0) && (c <= 30)) ? c : 0);
GCC_DIAG_ON(sign-conversion)
	} else if(unlikely(!read_num(&(hdr->version), errtext))) {
		return false;
	}
	if(unlikely(!hdr->isCurrent())) {
		if(errtext != NULLPTR) {
			*errtext = eix::format((hdr->version > DBHeader::current) ?
			_("cachefile uses newer format %s (current is %s)") :
			_("cachefile uses obsolete format %s (current is %s)"))
			% hdr->version % DBHeader::current;
		}
		return false;
	}

	if(unlikely(!read_num(&(hdr->size), errtext))) {
		return false;
	}

	ExtendedVersion::Overlay overlay_sz;
	if(unlikely(!read_num(&(overlay_sz), errtext))) {
		return false;
	}
	for(; likely(overlay_sz != 0); --overlay_sz) {
		string path;
		if(unlikely(!read_string(&path, errtext))) {
			return false;
		}
		string ov;
		if(unlikely(!read_string(&ov, errtext))) {
			return false;
		}
		hdr->addOverlay(OverlayIdent(path.c_str(), ov.c_str()));
	}

	if(unlikely(!read_hash(&(hdr->license_hash), errtext))) {
		return false;
	}
	if(unlikely(!read_hash(&(hdr->keywords_hash), errtext))) {
		return false;
	}
	if(unlikely(!read_hash(&(hdr->iuse_hash), errtext))) {
		return false;
	}
	if(unlikely(!read_hash(&(hdr->slot_hash), errtext))) {
		return false;
	}

	vector<string>::size_type sets_sz;
	if(unlikely(!read_num(&sets_sz, errtext))) {
		return false;
	}
	for(; likely(sets_sz != 0); --sets_sz) {
		string s;
		if(unlikely(!read_string(&s, errtext))) {
			return false;
		}
		hdr->world_sets.push_back(s);
	}

	eix::UNumber use_dep_num;
	if(unlikely(!read_num(&use_dep_num, errtext))) {
		return false;
	}
	if((hdr->use_depend = (use_dep_num != 0))) {
		eix::OffsetType len;
		if(unlikely(!read_num(&len, errtext))) {
			return false;
		}
		if(Depend::use_depend) {
			if(unlikely(!read_hash(&(hdr->depend_hash), errtext))) {
				return false;
			}
		} else if(len != 0) {
			if(unlikely(!seekrel(len, errtext))) {
				return false;
			}
		}
	}
	return true;
}

bool
Database::write_packagetree(const PackageTree &tree, const DBHeader &hdr, string *errtext)
{
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);
		// Write category-header followed by a list of the packages.
		if(unlikely(!write_category_header(c->first, eix::Treesize(ci->size()), errtext))) {
			return false;
		}

		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			// write package to fp
			if(unlikely(!write_package(**p, hdr, errtext))) {
				return false;
			}
		}
	}
	return true;
}

bool
Database::read_packagetree(PackageTree *tree, const DBHeader &hdr, PortageSettings *ps, string *errtext)
{
	PackageReader reader(this, hdr, ps);
	while(reader.nextCategory()) {
		Category &cat((*tree)[reader.category()]);
		while(reader.nextPackage()) {
			cat.push_back(reader.release());
		}
	}
	const char *c(reader.get_errtext());
	if(likely(c == NULLPTR)) {
		return true;
	}
	if(errtext != NULLPTR) {
		*errtext = c;
	}
	return false;
}
