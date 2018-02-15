// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "portage/mask.h"
#include <config.h>  // IWYU pragma: keep

#include <fnmatch.h>

#include <cstring>

#include <string>

#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "portage/basicversion.h"
#include "portage/extendedversion.h"
#include "portage/keywords.h"
#include "portage/package.h"
#include "portage/version.h"

using std::string;

// Data-driven programming is nice :).
// Must be in order .. no previous operator is allowed to be a prefix of a
// following operator.
//
// The priority is still modified, depending on whether version-wildard,
// slot and/or repository are specified. The result is similar to
// best_match_to_list() from portage's portage/dep/__init__.py
// Note that wildcards (extended syntax) have alway higher priority due to
// our treatmeant in mask_list.h; we only distinguish for identical masks.
// Thus, for example, we have the priority order
// */* */*::foo */bar */bar::foo cat/pkg cat/pkg::foo
// which deviates from that of portage which orders extended+repo lower:
// */* */bar */*::foo */bar::foo cat/pkg cat/pkg::foo
static const struct OperatorTable {
	const char *str;
	eix::TinyUnsigned len;
	Mask::Operator op;
	eix::TinyUnsigned priority;
} operators[] = { {
		"<=", 2, Mask::maskOpLessEqual, 16
	}, {
		"<" , 1, Mask::maskOpLess, 16
	}, {
		">=", 2, Mask::maskOpGreaterEqual, 16
	}, {
		">" , 1, Mask::maskOpGreater, 16
	}, {
		"=" , 1, Mask::maskOpEqual, 40
	}, {
		"~" , 1, Mask::maskOpRevisions, 32
	}, {
		"@" , 1, Mask::maskIsSet, 8
	}, {
		""  , 0, Mask::maskOpAll, 0  // this must be the last one
	}
};

/**
Split a "mask string" into its components
@return whether/which error occurred
@param str_mask the string to be dissected
@param errtext contains error message if not 0 and not parseOK
@param accept_garbage passed to parseVersion if appropriate
**/
BasicVersion::ParseResult Mask::parseMask(const char *str, string *errtext, eix::SignedBool accept_garbage, const char *default_repo) {
	// determine comparison operator
	if(unlikely(m_type == maskPseudomask)) {
		m_operator = maskOpEqual;
	} else {
		for(const OperatorTable *curr = operators; ; ++curr) {
			eix::TinyUnsigned len(curr->len);
			if(unlikely(len == 0)) {
				// no operator
				m_operator = curr->op;
				priority = curr->priority;
				break;
			}
			if(unlikely(std::strncmp(str, curr->str, len) == 0)) {
				m_operator = curr->op;
				priority = curr->priority;
				// Skip the operator-part
				str += std::strlen(curr->str);
				if(unlikely(m_operator == maskIsSet)) {
					m_category = SET_CATEGORY;
					m_name = str;
					return parsedOK;
				}
				break;
			}
		}
	}

	// Get the category
	const char *p(str);
	string::size_type l(0);
	for(; likely(*p != '/'); ++l, ++p) {
		if(unlikely(*p == '\0')) {
			*errtext = (_("cannot read category"));
			return parsedError;
		}
	}
	m_category = string(str, l);

	// Skip category-part
	str = p + 1;

	// If :... is appended, mark the slot part,
	// and if :: occurs, mark the repository part.
	// If [...] is appended (possibly after : or ::), remove it
	const char *end(std::strchr(str, ':'));
	if(end != NULLPTR) {
		string *dest;
		const char *source(end + 1);
		if((*source) != ':') {
			m_test_slot = true;
			const char *slot_end(std::strchr(source, ':'));
			if(unlikely(slot_end != NULLPTR)) {
GCC_DIAG_OFF(sign-conversion)
				m_slotname.assign(source, slot_end - source);
GCC_DIAG_ON(sign-conversion)
				if(unlikely(slot_end[1] != ':')) {
					*errtext = _("repository name must be separated with \"::\" (one \":\" is missing)");
					return parsedError;
				}
				source = slot_end + 2;
				priority += 3;  // Slot + Repository specified
				dest = &m_reponame;
			} else {
				m_reponame.clear();
				priority += 2;  // Slot (but no Repository) specified
				dest = &m_slotname;
			}
		} else {
			++source;
			m_test_slot = false;
			priority += 1;  // Repository (but no Slot) specified
			m_slotname.clear();
			dest = &m_reponame;
		}
		const char *usestart(std::strchr(source, '['));
		if((usestart != NULLPTR) && (std::strchr(usestart + 1, ']') == NULLPTR)) {
GCC_DIAG_OFF(sign-conversion)
			dest->assign(source, usestart - source);
GCC_DIAG_ON(sign-conversion)
		} else {
			dest->assign(source);
		}
		m_test_reponame = !(m_reponame.empty());
		if(slot_subslot(&m_slotname, &m_subslotname)) {
			m_test_subslot = m_test_slot;
		} else {
			m_test_subslot = false;
		}
	} else {
		m_test_slot = m_test_reponame = false;
		m_slotname.clear();
		m_subslotname.clear();
		m_reponame.clear();
		end = std::strchr(str, '[');
		if((end != NULLPTR) && (std::strchr(end + 1, ']') == NULLPTR))
			end = NULLPTR;
	}
	if((!m_test_reponame) && (default_repo != NULLPTR)) {
		m_test_reponame = true;
		m_reponame.assign(default_repo);
	}

	// Get the rest (name-version|name)
	bool have_version(false);
	if((m_operator != maskOpAll) || (m_type == maskMarkOptional)) {
		// Is there a version somewhere
		p = ExplodeAtom::get_start_of_version(str, true);

		if(unlikely((p == NULLPTR) || ((end != NULLPTR) && (p >= end)))) {
			if(unlikely(m_operator != maskOpAll)) {  // maskMarkOptional without explicit "="
				*errtext = ((m_type != maskPseudomask) ?
					_("operator without a version part") :
					_("version specification is missing"));
				return parsedError;
			}
		} else {
			have_version = true;
		}
	}

	if(have_version) {
GCC_DIAG_OFF(sign-conversion)
		m_name = string(str, (p - 1) - str);
GCC_DIAG_ON(sign-conversion)

		// Check for wildcard-version
		const char *wildcard((m_type != maskPseudomask) ? std::strchr(p, '*') : NULLPTR);
		if(unlikely((unlikely(wildcard != NULLPTR)) &&
			(likely((end == NULLPTR) || (wildcard <= end))))) {
			if(unlikely(m_operator == maskOpAll)) {  // maskMarkOptional without explicit "="
				m_operator = maskOpEqual;
				priority += 24;
			} else if(m_operator == maskOpEqual) {
				priority -= 16;
			}
			if(unlikely(m_operator != maskOpEqual)) {
				*errtext = _("a wildcard is only valid with the = operator");
				return parsedError;
			}
			if(unlikely(((wildcard + 1 != end)
					|| (end == NULLPTR)) &&
				((end != NULLPTR) || (wildcard[1] != '\0')))) {
				// Wildcard is not the last symbol:
				m_operator = maskOpGlobExt;
				if(end != NULLPTR) {
					m_glob.assign(p, end);
				} else {
					m_glob.assign(p);
				}
				return parsedOK;
			} else {
				m_operator = maskOpGlob;
			}
			end = wildcard;
		}
GCC_DIAG_OFF(sign-conversion)
		BasicVersion::ParseResult r(parseVersion(((end != NULLPTR) ? string(p, end - p) : p), errtext, accept_garbage));
GCC_DIAG_ON(sign-conversion)
		if(likely(m_operator != maskOpAll)) {
			return r;
		}
		// maskMarkOptional without explicit "="
		if(likely(r == BasicVersion::parsedOK)) {
			m_operator = maskOpEqual;
			return r;
		}
	}
	// Everything else is the package-name
GCC_DIAG_OFF(sign-conversion)
	m_name = ((end != NULLPTR) ? string(str, end - str) : str);
GCC_DIAG_ON(sign-conversion)
	return parsedOK;
}

void Mask::to_package(Package *p) const {
	p->category = m_category;
	p->name = m_name;
	Version *v(new Version);
	v->assign_basic_version(*this);
	v->slotname = m_slotname;
	v->subslotname = m_subslotname;
	v->reponame = m_reponame;
	p->addVersion(v);
}

/**
Test if the mask applies to a Version.
@return true if applies.
**/
bool Mask::test(const ExtendedVersion *ev) const {
	if(m_test_slot) {
		if(m_slotname != ev->slotname) {
			return false;
		}
		if(m_test_subslot) {
			if(m_subslotname != ev->subslotname) {
				return false;
			}
		}
	}
	if(!m_reponame.empty()) {
		if(m_reponame != ev->reponame) {
			return false;
		}
	}
	switch(m_operator) {
		case maskOpAll:
			return true;

		case maskOpGlob:
			return (BasicVersion::compare_right_maybe_shorter(*ev, *this) == 0);

		case maskOpGlobExt:
			return (fnmatch(m_glob.c_str(), ev->getFull().c_str(), 0) == 0);

		case maskOpLess:
			return (BasicVersion::compare(*this, *ev) > 0);

		case maskOpLessEqual:
			return (BasicVersion::compare(*this, *ev) >= 0);

		case maskOpEqual:
			return (BasicVersion::compare(*this, *ev) == 0);

		case maskOpGreaterEqual:
			return (BasicVersion::compare(*this, *ev) <= 0);

		case maskOpGreater:
			return (BasicVersion::compare(*this, *ev) < 0);

		case maskOpRevisions:
			return (BasicVersion::compareTilde(*this, *ev) == 0);

		// case maskIsSet: // makes no sense
		default:
			break;
	}
	return false;
}

void Mask::match(Matches *m, Package *pkg) const {
	for(Package::iterator it(pkg->begin()); likely(it != pkg->end()); ++it) {
		if(test(*it)) {
			m->PUSH_BACK(*it);
		}
	}
}

bool Mask::have_match(const Package& pkg) const {
	for(Package::const_iterator it(pkg.begin()); likely(it != pkg.end()); ++it) {
		if(test(*it)) {
			return true;
		}
	}
	return false;
}

/**
Set the stability members of all version in package according to the mask.
@param pkg            package you want tested
@param check          Redundancy checks which should apply
**/
void Mask::checkMask(Package *pkg, Keywords::Redundant check) const {
	for(Package::iterator i(pkg->begin()); likely(i != pkg->end()); ++i) {
		apply(*i, true, check);
	}
}

void KeywordMask::applyItem(Package *pkg) const {
	for(Package::iterator i(pkg->begin()); likely(i != pkg->end()); ++i) {
		if(test(*i)) {
			applyItem(*i);
		}
	}
}

void PKeywordMask::applyItem(Package *pkg) const {
	for(Package::iterator i(pkg->begin()); likely(i != pkg->end()); ++i) {
		if(test(*i)) {
			applyItem(*i);
		}
	}
}

void SetMask::applyItem(Package *pkg) const {
	for(Package::iterator i(pkg->begin()); likely(i != pkg->end()); ++i) {
		if(i->is_in_set(m_set))  // No need to check: Already in set
			continue;
		if(!test(*i))
			continue;
		i->add_to_set(m_set);
	}
}

bool Mask::ismatch(const Package& pkg) const {
	if((fnmatch(m_name.c_str(), pkg.name.c_str(), 0) != 0) ||
		(fnmatch(m_category.c_str(), pkg.category.c_str(), 0) != 0))
		return false;
	for(Package::const_iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(test(*i))
			return true;
	}
	return false;
}

/**
Set the stability & masked members of ve according to the mask
@param ve         Version instance to be set
@param do_test    set conditionally or unconditionally
@param check      check these for changes
**/
void Mask::apply(Version *ve, bool do_test, Keywords::Redundant check) const {
	switch(m_type) {
		case maskUnmask:
			if(do_test && (!test(ve))) {
				break;
			}
			if(check & Keywords::RED_IN_UNMASK) {
				ve->set_redundant(Keywords::RED_IN_UNMASK);
			}
			if(check & Keywords::RED_DOUBLE_UNMASK) {
				if(ve->wanted_unmasked()) {
					ve->set_redundant(Keywords::RED_DOUBLE_UNMASK);
				}
				ve->set_wanted_unmasked();
			}
			if(ve->maskflags.isPackageMask()) {
				ve->maskflags.clearbits(MaskFlags::MASK_PACKAGE);
				if(check & Keywords::RED_UNMASK) {
					ve->set_was_unmasked();
				}
			}
			break;
		case maskMask:
			if(do_test && (!test(ve))) {
				break;
			}
			if(check & Keywords::RED_IN_MASK) {
				ve->set_redundant(Keywords::RED_IN_MASK);
			}
			if(check & Keywords::RED_DOUBLE_MASK) {
				if(ve->wanted_masked())
					ve->set_redundant(Keywords::RED_DOUBLE_MASK);
				ve->set_wanted_masked();
			}
			if(!ve->maskflags.isPackageMask()) {
				ve->maskflags.setbits(MaskFlags::MASK_PACKAGE);
				if(check & Keywords::RED_MASK) {
					ve->set_was_masked();
				}
			}
			ve->add_reason(comments);
			break;
		case maskInSystem:
			if(ve->maskflags.isSystem() && ve->maskflags.isProfileMask()) {  /* Won't change anything */
				break;
			}
			if((!do_test) || test(ve)) {
				ve->maskflags.setbits(MaskFlags::MASK_SYSTEM);
			} else {
				ve->maskflags.setbits(MaskFlags::MASK_PROFILE);
			}
			break;
		case maskInWorld:
			if(ve->maskflags.isWorld()) {
				break;
			}
			if((!do_test) || test(ve)) {
				ve->maskflags.setbits(MaskFlags::MASK_WORLD);
			}
			break;
		case maskInProfile:
			if(ve->maskflags.isProfile()) {  /* Won't change anything */
				break;
			}
			if((!do_test) || test(ve)) {
				ve->maskflags.setbits(MaskFlags::MASK_IN_PROFILE);
			}
			break;
		case maskMark:
		case maskMarkOptional:
			if(do_test && (!test(ve))) {
				break;
			}
			ve->maskflags.setbits(MaskFlags::MASK_MARKED);
			// break;
		// case maskPseudomask:
		// case maskTypeNone:
		default:
			break;
	}
}
