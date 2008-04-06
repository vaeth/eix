// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

/* we use strndup */
#if !defined _GNU_SOURCE
#define _GNU_SOURCE
#endif /* !defined _GNU_SOURCE */

#include "mask.h"
#include <eixTk/stringutils.h> /* to get strndup on macos */
#include <portage/keywords.h>
#include <portage/package.h>
#include <portage/version.h>

#include <eixTk/exceptions.h>

#include <iostream>

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
	for(unsigned int i = 0;
		i < sizeof(operators) / sizeof(OperatorTable);
		++i)
	{
		if(strncmp(str, operators[i].str, operators[i].len) == 0)
		{
			m_operator = operators[i].op;
			// Skip the operator-part
			str += strlen(operators[i].str);
			break;
		}
	}

	// Get the category
	const char *p = str;
	while(*p != '/')
	{
		if(*p == '\0')
		{
			throw ExBasic("Can't read category.");
		}
		++p;
	}
	m_category = string(str, p - str);

	// Skip category-part
	str = p + 1;

	// If :... is appended, mark the slot part;
	const char *end = strrchr(str, ':');
	m_test_slot = bool(end);
	if(m_test_slot) {
		m_slotname = (end + 1);
		// Interpret Slot "0" as empty slot (as internally always)
		if(m_slotname == "0")
			m_slotname.clear();
	}

	// Get the rest (name-version|name)
	if(m_operator != maskOpAll)
	{
		// There must be a version somewhere
		p = ExplodeAtom::get_start_of_version(str);

		if((!p) || (end && (p >= end)))
		{
			throw ExBasic("You have a operator but we can't find a version-part.");
		}

		m_name = string(str, (p - 1) - str);
		str = p;

		// Check for wildcard-version
		const char *wildcard = strchr(str, '*');
		if(wildcard && end && (wildcard >= end))
			wildcard = NULL;

		if(wildcard && wildcard[1] != '\0')
		{
			if(!(end && ((wildcard + 1) == end)))
				throw ExBasic("A '*' is only valid at the end of a version-string.");
		}

		// Only the = operator can have a wildcard-version
		if(m_operator != maskOpEqual && wildcard)
		{
			// A finer error-reporting
			if(m_operator != maskOpRevisions)
			{
				throw ExBasic("A wildcard is only valid with the = operator.");
			}
			else
			{
				throw ExBasic(
						"A wildcard is only valid with the = operator.\n"
						"Portage would also accept something like ~app-shells/bash-3*,\n"
						"but behave just like ~app-shells/bash-3."
					);
			}
		}

		if(wildcard)
		{
			m_operator = maskOpGlob;
			m_cached_full = string(str, wildcard);
		}
		else
		{
			if(end)
				parseVersion(string(str, end - str));
			else
				parseVersion(str);
		}
	}
	else
	{
		// Everything else is the package-name
		if(end)
			m_name = string(str, end - str);
		else
			m_name = string(str);
	}
}

/** Tests if the mask applies to a Version.
 * @param name name of package (NULL if shall not be tested)
 * @param category category of package (NULL if shall not be tested)
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
				const std::string& s1(getFull());
				const std::string& s2(ev->getFull());
				const std::string::size_type start = s1.find_first_not_of('0');

				/* If the version part contains only zeros, everything is
				 * masked. http://bugs.gentoo.org/show_bug.cgi?id=216483 */
				if (start == std::string::npos)
					return true;

				const std::string::size_type total = s1.size() - start;
				return s2.compare(s2.find_first_not_of('0'), total, s1, start, total) == 0;
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

		default:
			break;
	}
	return false; // Never reached
}

eix::ptr_list<Version>
Mask::match(Package &pkg) const
{
	eix::ptr_list<Version> ret;
	for(Package::iterator it = pkg.begin();
		it != pkg.end();
		++it)
	{
		if(test(*it))
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

/** Sets the stability members of all version in package according to the mask.
 * @param pkg            package you want tested
 * @param check_name     true if name should be tested */
void
Mask::checkMask(Package& pkg, Keywords::Redundant check)
{
	for(Package::iterator i = pkg.begin();
		i != pkg.end();
		++i)
	{
		apply(*i, check);
	}
	return;
}

bool
Mask::ismatch(Package& pkg)
{
	if (pkg.name != m_name || pkg.category != m_category)
		return false;
	for(Package::iterator i = pkg.begin(); i != pkg.end(); ++i)
		if(test(*i))
			return true;
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
	return;
}
