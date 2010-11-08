// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "mask.h"
#include <eixTk/exceptions.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/stringutils.h>
#include <portage/extendedversion.h>
#include <portage/keywords.h>
#include <portage/package.h>
#include <portage/version.h>

#include <string>

#include <cstddef>
#include <cstring>
#include <fnmatch.h>

using namespace std;

/** Data-driven programming is nice :).
 * Must be in order .. no previous operator is allowed to be a prefix of a
 * following operator. */
static const struct OperatorTable {
	const char *str;
	unsigned char len;
	Mask::Operator op;
} operators[] = {
	{ "<=", 2, Mask::maskOpLessEqual },
	{ "<" , 1, Mask::maskOpLess },
	{ ">=", 2, Mask::maskOpGreaterEqual },
	{ ">" , 1, Mask::maskOpGreater },
	{ "=" , 1, Mask::maskOpEqual },
	{ "~" , 1, Mask::maskOpRevisions },
	{ "@" , 1, Mask::maskIsSet },
	{ ""  , 0, Mask::maskOpAll /* this must be the last one */ }
};

/** Constructor. */
Mask::Mask(const char *str, Type type)
{
	m_type = type;
	parseMask(str);
}

/** split a "mask string" into its components */
void
Mask::parseMask(const char *str) throw(ExBasic)
{
	// determine comparison operator
	for(unsigned int i(0); ; ++i) {
		unsigned char len = operators[i].len;
		if(unlikely(len == 0)) {
			// no operator
			m_operator = operators[i].op;
			break;
		}
		if(unlikely(strncmp(str, operators[i].str, len) == 0)) {
			m_operator = operators[i].op;
			// Skip the operator-part
			str += strlen(operators[i].str);
			if(unlikely(m_operator == maskIsSet)) {
				m_category = SET_CATEGORY;
				m_name = str;
				return;
			}
			break;
		}
	}

	// Get the category
	const char *p(str);
	string::size_type l(0);
	for(; likely(*p != '/'); ++l, ++p) {
		if(unlikely(*p == '\0')) {
			throw ExBasic(_("Can't read category."));
		}
	}
	m_category = string(str, l);

	// Skip category-part
	str = p + 1;

	// If :... is appended, mark the slot part,
	// and if :: occurs, mark the repository part.
	// If [...] is appended (possibly after : or ::), remove it
	const char *end(strchr(str, ':'));
	if(end != NULL) {
		string *dest;
		const char *source(end + 1);
		if((*source) != ':') {
			m_test_slot = true;
			const char *slot_end(strchr(source, ':'));
			if(unlikely(slot_end != NULL)) {
				m_slotname.assign(source, slot_end - source);
				if(unlikely(slot_end[1] != ':')) {
					throw ExBasic(_("Repository name must be separated with :: (one : is missing)"));
				}
				source = slot_end + 2;
				dest = &m_reponame;
			}
			else {
				m_reponame.clear();
				dest = &m_slotname;
			}
		}
		else {
			++source;
			m_test_slot = false;
			m_slotname.clear();
			dest = &m_reponame;
		}
		const char *usestart(strchr(source, '['));
		if((usestart != NULL) && ! strchr(usestart + 1, ']')) {
			dest->assign(source, usestart - source);
		}
		else {
			dest->assign(source);
		}
		m_test_reponame = !(m_reponame.empty());
		// Interpret Slot "0" as empty slot (as internally always)
		if(m_slotname == "0") {
			m_slotname.clear();
		}
	}
	else {
		m_test_slot = m_test_reponame = false;
		m_slotname.clear();
		m_reponame.clear();
		end = strchr(str, '[');
		if((end != NULL) && ! strchr(end + 1, ']'))
			end = NULL;
	}

	// Get the rest (name-version|name)
	if(m_operator != maskOpAll)
	{
		// There must be a version somewhere
		p = ExplodeAtom::get_start_of_version(str);

		if(unlikely((p == NULL) || ((end != NULL) && (p >= end)))) {
			throw ExBasic(_("You have a operator but we can't find a version-part."));
		}

		m_name = string(str, (p - 1) - str);
		str = p;

		// Check for wildcard-version
		const char *wildcard(strchr(str, '*'));
		if(unlikely((unlikely(wildcard != NULL)) &&
			(likely((end == NULL) || (wildcard <= end))))) {
			if(unlikely((wildcard[1] != '\0') &&
				// The following is also valid if end=NULL
				(wildcard + 1 != end))) {
				throw ExBasic(_("A '*' is only valid at the end of a version-string."));
			}
			// Only the = operator can have a wildcard-version
			if(unlikely(m_operator != maskOpEqual)) {
				// A finer error-reporting
				if(m_operator != maskOpRevisions) {
					throw ExBasic(_("A wildcard is only valid with the = operator."));
				}
				else {
					throw ExBasic(_(
						"A wildcard is only valid with the = operator.\n"
						"Portage would also accept something like ~app-shells/bash-3*,\n"
						"but behave just like ~app-shells/bash-3."));
				}
			}
			m_operator = maskOpGlob;
			m_cached_full = string(str, wildcard);
		}
		else {
			if(end != NULL)
				parseVersion(string(str, end - str));
			else
				parseVersion(str);
		}
	}
	else
	{
		// Everything else is the package-name
		if(end != NULL)
			m_name = string(str, end - str);
		else
			m_name = str;
	}
}

/** Tests if the mask applies to a Version.
 * @return true if applies. */
bool
Mask::test(const ExtendedVersion *ev) const
{
	if(m_test_slot) {
		if(m_slotname != ev->slotname) {
			return false;
		}
	}
	if(!m_reponame.empty()) {
		if(m_reponame != ev->reponame) {
			return false;
		}
	}
	switch(m_operator)
	{
		case maskOpAll:
			return true;

		case maskOpGlob:
			{
				// '=*' operator has to remove leading zeros
				// see match_from_list in portage_dep.py
				const std::string& my_string(getFull());
				const std::string& version_string(ev->getFull());

				std::string::size_type my_start(my_string.find_first_not_of('0'));
				std::string::size_type version_start(version_string.find_first_not_of('0'));

				/* Otherwise, if a component has a leading zero, any trailing
				 * zeroes in that component are stripped (if this makes the
				 * component empty, proceed as if it were 0 instead), and the
				 * components are compared using a stringwise comparison.
				 */

				if (my_start == std::string::npos)
					my_start = my_string.size() - 1;
				else if(!isdigit(my_string[my_start], localeC))
					my_start -= 1;

				if (version_start == std::string::npos)
					version_start = version_string.size() - 1;
				else if(!isdigit(version_string[version_start], localeC))
					version_start -= 1;

				const std::string::size_type total(my_string.size() - my_start);
				return version_string.compare(version_start, total, my_string, my_start, total) == 0;
			}

		case maskOpLess:
			return BasicVersion::compare(*this, *ev) > 0;

		case maskOpLessEqual:
			return BasicVersion::compare(*this, *ev) >= 0;

		case maskOpEqual:
			return BasicVersion::compare(*this, *ev) == 0;

		case maskOpGreaterEqual:
			return BasicVersion::compare(*this, *ev) <= 0;

		case maskOpGreater:
			return BasicVersion::compare(*this, *ev) < 0;

		case maskOpRevisions:
			return BasicVersion::compareTilde(*this, *ev) == 0;

		case maskIsSet: // makes no sense
		default:
			break;
	}
	return false;
}

void
Mask::match(Matches &m, Package &pkg) const
{
	for(Package::iterator it(pkg.begin()); likely(it != pkg.end()); ++it) {
		if(test(*it)) {
			m.push_back(*it);
		}
	}
}

bool
Mask::have_match(Package &pkg) const
{
	for(Package::iterator it(pkg.begin()); likely(it != pkg.end()); ++it) {
		if(test(*it)) {
			return true;
		}
	}
	return false;
}

/** Sets the stability members of all version in package according to the mask.
 * @param pkg            package you want tested
 * @param check          Redundancy checks which should apply */
void
Mask::checkMask(Package& pkg, Keywords::Redundant check) const
{
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		apply(*i, true, false, check);
	}
}

/** Sets the stability member of all versions in virtual package according to the mask. */
void
Mask::applyVirtual(Package& pkg) const
{
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		apply(*i, false, true, Keywords::RED_NOTHING);
	}
}

void
KeywordMask::applyItem(Package &pkg) const
{
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(test(*i)) {
			applyItem(*i);
		}
	}
}

void
PKeywordMask::applyItem(Package &pkg) const
{
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(test(*i)) {
			applyItem(*i);
		}
	}
}

void
SetMask::applyItem(Package& pkg) const
{
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(i->is_in_set(m_set)) // No need to check: Already in set
			continue;
		if(!test(*i))
			continue;
		i->add_to_set(m_set);
	}
}

bool
Mask::ismatch(Package& pkg) const
{
	if (fnmatch(m_name.c_str(), pkg.name.c_str(), 0) ||
		fnmatch(m_category.c_str(), pkg.category.c_str(), 0))
		return false;
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(test(*i))
			return true;
	}
	return false;
}

/** Sets the stability & masked members of ve according to the mask
 * @param ve         Version instance to be set
 * @param do_test    set conditionally or unconditionally
 * @param is_virtual the version matches only as a virtual name
 * @param check      check these for changes */
void Mask::apply(Version *ve, bool do_test, bool is_virtual, Keywords::Redundant check) const
{
	switch(m_type) {
		case maskUnmask:
			if(is_virtual || (do_test && (!test(ve))))
				break;
			if(check & Keywords::RED_IN_UNMASK)
				ve->set_redundant(Keywords::RED_IN_UNMASK);
			if(check & Keywords::RED_DOUBLE_UNMASK)
			{
				if(ve->wanted_unmasked())
					ve->set_redundant(Keywords::RED_DOUBLE_UNMASK);
				ve->set_wanted_unmasked();
			}
			if(ve->maskflags.isPackageMask())
			{
				ve->maskflags.clearbits(MaskFlags::MASK_PACKAGE);
				if(check & Keywords::RED_UNMASK)
					ve->set_was_unmasked();
			}
			break;
		case maskMask:
			if(is_virtual || (do_test && (!test(ve))))
				break;
			if(check & Keywords::RED_IN_MASK)
				ve->set_redundant(Keywords::RED_IN_MASK);
			if(check & Keywords::RED_DOUBLE_MASK)
			{
				if(ve->wanted_masked())
					ve->set_redundant(Keywords::RED_DOUBLE_MASK);
				ve->set_wanted_masked();
			}
			if(!ve->maskflags.isPackageMask())
			{
				ve->maskflags.setbits(MaskFlags::MASK_PACKAGE);
				if(check & Keywords::RED_MASK)
					ve->set_was_masked();
			}
			break;
		case maskInSystem:
			if(is_virtual) {
				if(ve->maskflags.isVirtualSystem()) {
					break;
				}
				if((!do_test) || test(ve)) {
					ve->maskflags.setbits(MaskFlags::MASK_VIRTUAL_SYSTEM);
				}
				break;
			}
			if(ve->maskflags.isSystem() && ve->maskflags.isProfileMask())	/* Won't change anything cause already masked by profile */
				break;
			if((!do_test) || test(ve)) {
				ve->maskflags.setbits(MaskFlags::MASK_SYSTEM);
			}
			else {
				ve->maskflags.setbits(MaskFlags::MASK_PROFILE);
			}
			break;
		case maskInWorld:
			if(is_virtual) {
				if(ve->maskflags.isVirtualWorld()) {
					break;
				}
				if((!do_test) && test(ve))
					ve->maskflags.setbits(MaskFlags::MASK_VIRTUAL_WORLD);
				break;
			}
			if(ve->maskflags.isWorld()) {
				break;
			}
			if((!do_test) || test(ve)) {
				ve->maskflags.setbits(MaskFlags::MASK_WORLD);
			}
			break;
		case maskAllowedByProfile:
			if(is_virtual || ve->maskflags.isProfileMask())	/* Won't change anything cause already masked by profile */
				break;
			if(do_test && (!test(ve)))
				ve->maskflags.setbits(MaskFlags::MASK_PROFILE);
		//	break;
		// case maskTypeNone:
		default:
			break;
	}
}
