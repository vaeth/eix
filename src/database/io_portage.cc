// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include "database/io.h"
#include <config.h>  // IWYU pragma: keep

#include <string>

#include "database/header.h"
#include "database/package_reader.h"
#include "eixTk/auto_array.h"
#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "portage/basicversion.h"
#include "portage/depend.h"
#include "portage/extendedversion.h"
#include "portage/keywords.h"
#include "portage/overlay.h"
#include "portage/package.h"
#include "portage/packagetree.h"
#include "portage/version.h"

using std::string;

#define WRITE_COUNTER(f) do { \
	eix::OffsetType counter_save(counter); \
	counter = 0; \
	bool counting_save(counting); \
	counting = true; \
	f; \
	counting = counting_save; \
	eix::OffsetType counter_diff(counter); \
	counter = counter_save; \
	if(unlikely(!write_num(counter_diff, errtext))) { \
		return false; \
	} \
} while(0)

bool Database::read_Part(BasicPart *b, string *errtext) {
	string::size_type len;
	if(unlikely(!read_num(&len, errtext))) {
		return false;
	}
	BasicPart::PartType type(BasicPart::PartType(len % BasicPart::max_type));
	len /= BasicPart::max_type;
	if(len != 0) {
		eix::auto_array<char> buf(new char[len + 1]);
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

bool Database::read_iuse(const StringHash& hash, IUseSet *iuse, string *errtext) {
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

bool Database::read_version(Version *v, const DBHeader& hdr, string *errtext) {
	// read EAPI
	if(likely(hdr.version >= 36)) {
		string eapi;
		if(likely(read_hash_string(hdr.eapi_hash, &eapi, errtext))) {
			v->eapi.assign(eapi);
		} else {
			return false;
		}
	}

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
	BasicVersion::PartsType::size_type i;
	if(unlikely(!read_num(&i, errtext))) {
		return false;
	}
	v->m_parts.reserve(i);
	for(; likely(i != 0); --i) {
		BasicPart b;
		if(unlikely(!read_Part(&b, errtext))) {
			return false;
		}
		v->m_parts.PUSH_BACK(MOVE(b));
	}

	string fullslot;
	if(unlikely(!read_hash_string(hdr.slot_hash, &fullslot, errtext))) {
		return false;
	}
	v->set_slotname(fullslot);
	if(unlikely(!read_num(&(v->overlay_key), errtext))) {
		return false;
	}
	const OverlayIdent& overlay(hdr.getOverlay(v->overlay_key));
	v->reponame = overlay.label;
	v->priority = overlay.priority;

	if(unlikely(!read_iuse(hdr.iuse_hash, &(v->iuse), errtext))) {
		return false;
	}
	if(hdr.use_required_use) {
		if(Version::use_required_use) {
			if(unlikely(!read_hash_words(hdr.iuse_hash, &(v->required_use), errtext))) {
				return false;
			}
		} else if(unlikely(!read_hash_words(errtext))) {
			return false;
		}
	} else {
		v->required_use.clear();
	}
	if(hdr.use_depend) {
		if(unlikely(!read_depend(&(v->depend), hdr, errtext))) {
			return false;
		}
	}

	// v->save_maskflags(Version::SAVEMASK_FILE);  // This is done in package_reader
	return true;
}

bool Database::write_Part(const BasicPart& n, string *errtext) {
	const string& content(n.partcontent);
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

bool Database::write_version(const Version *v, const DBHeader& hdr, string *errtext) {
	// write EAPI
	if(unlikely(!write_hash_string(hdr.eapi_hash, v->eapi.get(), errtext))) {
		return false;
	}

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

	for(BasicVersion::PartsType::const_iterator it(v->m_parts.begin());
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
	if(hdr.use_required_use) {
		if(unlikely(!write_hash_words(hdr.iuse_hash, v->required_use, errtext))) {
			return false;
		}
	}
	if(hdr.use_depend) {
		WRITE_COUNTER(write_depend(v->depend, hdr, NULLPTR));
		if(unlikely(!write_depend(v->depend, hdr, errtext))) {
			return false;
		}
	}
	return true;
}

bool Database::read_depend(Depend *dep, const DBHeader& hdr, string *errtext) {
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
			dep->m_bdepend.clear();
		} else if(unlikely(!read_hash_words(hdr.depend_hash, &(dep->m_bdepend), errtext))) {
			return false;
		}
		dep->obsolete = (hdr.version <= 32);
	} else {
		dep->clear();
GCC_DIAG_OFF(sign-conversion)
		if(unlikely(!seekrel(len, errtext))) {
			return false;
		}
GCC_DIAG_ON(sign-conversion)
	}
	return true;
}

bool Database::write_depend(const Depend& dep, const DBHeader& hdr, string *errtext) {
	return (likely(write_hash_words(hdr.depend_hash, dep.m_depend, errtext)) &&
		likely(write_hash_words(hdr.depend_hash, dep.m_rdepend, errtext)) &&
		likely(write_hash_words(hdr.depend_hash, dep.m_pdepend, errtext)) &&
		likely(write_hash_words(hdr.depend_hash, dep.m_bdepend, errtext)));
}

bool Database::read_category_header(string *name, eix::Treesize *h, string *errtext) {
	return (likely(read_string(name, errtext)) &&
		likely(read_num(h, errtext)));
}

bool Database::write_category_header(const string& name, eix::Treesize size, string *errtext) {
	return (likely(write_string(name, errtext)) &&
		likely(write_num(size, errtext)));
}

bool Database::write_package_pure(const Package& pkg, const DBHeader& hdr, string *errtext) {
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

bool Database::write_package(const Package& pkg, const DBHeader& hdr, string *errtext) {
	WRITE_COUNTER(write_package_pure(pkg, hdr, NULLPTR));
	return write_package_pure(pkg, hdr, errtext);
}

bool Database::write_hash(const StringHash& hash, string *errtext) {
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

void Database::prep_header_hashs(DBHeader *hdr, const PackageTree& tree) {
	hdr->eapi_hash.init(true);
	hdr->license_hash.init(true);
	hdr->keywords_hash.init(true);
	hdr->slot_hash.init(true);
	hdr->iuse_hash.init(true);
	bool use_dep(Depend::use_depend);
	hdr->use_depend = use_dep;
	if(use_dep) {
		hdr->depend_hash.init(true);
	}
	bool use_required_use(Version::use_required_use);
	hdr->use_required_use = use_required_use;
	for(PackageTree::const_iterator c(tree.begin()); likely(c != tree.end()); ++c) {
		Category *ci(c->second);
		for(Category::iterator p(ci->begin()); likely(p != ci->end()); ++p) {
			hdr->license_hash.hash_string(p->licenses);
			for(Package::iterator v(p->begin()); likely(v != p->end()); ++v) {
				hdr->eapi_hash.hash_string(v->eapi.get());
				hdr->keywords_hash.hash_words(v->get_full_keywords());
				hdr->iuse_hash.hash_words(v->iuse.asVector());
				if(use_required_use) {
					hdr->iuse_hash.hash_words(v->required_use);
				}
				hdr->slot_hash.hash_string(v->get_shortfullslot());
				if(use_dep) {
					const Depend& dep(v->depend);
					hdr->depend_hash.hash_words(dep.m_depend);
					hdr->depend_hash.hash_words(dep.m_rdepend);
					hdr->depend_hash.hash_words(dep.m_pdepend);
					hdr->depend_hash.hash_words(dep.m_bdepend);
				}
			}
		}
	}
	hdr->eapi_hash.finalize();
	hdr->license_hash.finalize();
	hdr->keywords_hash.finalize();
	hdr->slot_hash.finalize();
	hdr->iuse_hash.finalize();
	if(use_dep) {
		hdr->depend_hash.finalize();
	}
}

bool Database::write_header(const DBHeader& hdr, string *errtext) {
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
	if(unlikely(!write_hash(hdr.eapi_hash, errtext))) {
		return false;
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
	for(WordVec::const_iterator it(hdr.world_sets.begin());
		likely(it != hdr.world_sets.end()); ++it) {
		if(unlikely(!write_string(*it, errtext))) {
			return false;
		}
	}

	DBHeader::SaveBitmask save_bitmask(DBHeader::SAVE_BITMASK_NONE);
	if(hdr.use_depend) {
		save_bitmask |= DBHeader::SAVE_BITMASK_DEP;
	}
	if(hdr.use_required_use) {
		save_bitmask |= DBHeader::SAVE_BITMASK_REQUIRED_USE;
	}
	if(unlikely(!write_num(save_bitmask, errtext))) {
		return false;
	}
	if(!hdr.use_depend) {
		return true;
	}
	WRITE_COUNTER(write_hash(hdr.depend_hash, NULLPTR));
	return write_hash(hdr.depend_hash, errtext);
}

bool Database::write_packagetree(const PackageTree& tree, const DBHeader& hdr, string *errtext) {
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

#if 0
bool Database::read_packagetree(PackageTree *tree, const DBHeader& hdr, PortageSettings *ps, string *errtext) {
	PackageReader reader(this, hdr, ps);
	while(reader.nextCategory()) {
		Category& cat((*tree)[reader.category()]);
		while(reader.nextPackage()) {
			cat.addPackage(reader.release());
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
#endif
