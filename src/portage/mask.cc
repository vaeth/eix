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
#include <eixTk/unused.h>
#include <portage/extendedversion.h>
#include <portage/keywords.h>
#include <portage/package.h>
#include <portage/version.h>

#include <string>

#include <cstddef>
#include <cstring>

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

	// If :... is appended, mark the slot part;
	// if [...] is appended (possibly after :...), remove it
	const char *end(strrchr(str, ':'));
	m_test_slot = bool(end);
	if(m_test_slot) {
		const char *usestart(strrchr(end + 1, '['));
		if((usestart != NULL) && strchr(usestart + 1, ']')) {
			if(usestart > end)
				m_slotname = string(end + 1, usestart - end - 1);
			else
				m_slotname.clear();
		}
		else
			m_slotname = (end + 1);
		// Interpret Slot "0" as empty slot (as internally always)
		if(m_slotname == "0")
			m_slotname.clear();
	}
	else {
		end = strrchr(str, '[');
		if(end && ! strchr(end + 1, ']'))
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
			if(unlikely((wildcard[1] != '\0') ||
				unlikely((end != NULL) && (wildcard + 1 == end)))) {
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
		if(m_slotname != ev->slotname)
			return false;
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

eix::ptr_list<Version>
Mask::match(Package &pkg) const
{
	eix::ptr_list<Version> ret;
	for(Package::iterator it(pkg.begin()); likely(it != pkg.end()); ++it) {
		if(test(*it)) {
			ret.push_back(*it);
		}
	}
	return ret;
}

/** Sets the stability members of all version in package according to the mask.
 * @param pkg            package you want tested
 * @param check          Redundancy checks which should apply */
void
Mask::checkMask(Package& pkg, Keywords::Redundant check)
{
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		apply(*i, check);
	}
}

void
PKeywordMask::checkMask(Package &pkg, Keywords::Redundant check ATTRIBUTE_UNUSED)
{
	UNUSED(check);
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(test(*i)) {
			i->modify_effective_keywords(keywords);
		}
	}
}

void
SetMask::checkMask(Package& pkg, Keywords::Redundant check ATTRIBUTE_UNUSED)
{
	UNUSED(check);
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(i->is_in_set(m_set)) // No need to check: Already in set
			continue;
		if(!test(*i))
			continue;
		i->add_to_set(m_set);
	}
}

bool
Mask::ismatch(Package& pkg)
{
	if (pkg.name != m_name || pkg.category != m_category)
		return false;
	for(Package::iterator i(pkg.begin()); likely(i != pkg.end()); ++i) {
		if(test(*i))
			return true;
	}
	return false;
}

/** Sets the stability & masked members of ve according to the mask
 * @param ve Version instance to be set */
void Mask::apply(Version *ve, Keywords::Redundant check)
{
	switch(m_type) {
		case maskUnmask:
			if(!test(ve))
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
			if(!test(ve))
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
			if( ve->maskflags.isSystem() && ve->maskflags.isProfileMask())	/* Won't change anything cause already masked by profile */
				break;
			if( test(ve) )
				ve->maskflags.setbits(MaskFlags::MASK_SYSTEM);
			else
				ve->maskflags.setbits(MaskFlags::MASK_PROFILE);
			break;
		case maskInWorld:
			if( ve->maskflags.isWorld() )
				break;
			if( test(ve) )
				ve->maskflags.setbits(MaskFlags::MASK_WORLD);
			break;
		case maskAllowedByProfile:
			if( ve->maskflags.isProfileMask())	/* Won't change anything cause already masked by profile */
				break;
			if(!test(ve))
				ve->maskflags.setbits(MaskFlags::MASK_PROFILE);
			break;
		case maskTypeNone:
			break;
		default:
			break;
	}
}
