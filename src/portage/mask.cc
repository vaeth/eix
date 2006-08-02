/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* we use strndup */
#if !defined _GNU_SOURCE
#define _GNU_SOURCE
#endif /* !defined _GNU_SOURCE */

#include "mask.h"
#include <eixTk/stringutils.h> /* to get strndup on macos */
#include <portage/keywords.h>
#include <portage/package.h>
#include <portage/version.h>

#include <eixTk/regexp.h>
#include <eixTk/exceptions.h>

#include <iostream>

using namespace std;

/** Data-driven programming is nice :).
 * Must be in order .. no previous operator is allowed to be a prefix of a
 * following operator. */
Mask::OperatorTable Mask::operators[] = {
	{ "<=", Mask::maskOpLessEqual },
	{ "<" , Mask::maskOpLess },
	{ ">=", Mask::maskOpGreaterEqual },
	{ ">" , Mask::maskOpGreater },
	{ "=" , Mask::maskOpEqual },
	{ "~" , Mask::maskOpRevisions },
	{ ""  , Mask::maskOpAll /* this must be the last one */ }
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
	int i = 0;
	for(; operators[i].str != NULL; ++i)
	{
		if(strncmp(str, operators[i].str,
		           strlen(operators[i].str)) == 0)
		{
			m_operator = operators[i].op;
			break;
		}
	}

	// Skip the operator-part
	str += strlen(operators[i].str);

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

	// Get the rest (name-version|name)
	if(m_operator != maskOpAll)
	{
		// There must be a version somewhere
		p = ExplodeAtom::get_start_of_version(str);

		if(p == NULL)
		{
			throw ExBasic("You have a operator "
			              "but we can't find a version-part.");
		}

		m_name = string(str, (p - 1) - str);
		str = p;

		// Check for wildcard-version
		const char *wildcard = strchr(str, '*');

		if(wildcard && wildcard[1] != '\0')
		{
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
			m_full = string(str, strlen(str) - 1);
		}
		else
		{
			parseVersion(str);
		}
	}
	else
	{
		// Everything else is the package-name
		m_name = string(str);
	}
}

/** Tests if the mask applies to a Version.
 * @param name name of package (NULL if shall not be tested)
 * @param category category of package (NULL if shall not be tested)
 * @return true if applies. */
bool
Mask::test(BasicVersion *bv) const
{
	switch(m_operator)
	{
		case maskOpAll:
			return true;

		case maskOpGlob:
			return strncmp(m_full.c_str(), bv->getFull(),
					m_full.size()) == 0;

		case maskOpLess:
			return (*bv < static_cast<BasicVersion>(*this));

		case maskOpLessEqual:
			return (*bv <= static_cast<BasicVersion>(*this));

		case maskOpEqual:
			return (*bv == static_cast<BasicVersion>(*this));

		case maskOpGreaterEqual:
			return (*bv >= static_cast<BasicVersion>(*this));

		case maskOpGreater:
			return (*bv > static_cast<BasicVersion>(*this));

		case maskOpRevisions:
			return (comparePrimary(*bv) == 0
			        && (getPrimarychar() == bv->getPrimarychar() )
			        && (getSuffixlevel() == bv->getSuffixlevel() )
			        && (getSuffixnum() == bv->getSuffixnum() ) );
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
Mask::checkMask(Package& pkg, const bool check_category, const bool check_name, Keywords::Redundant check)
{
	if((check_name && pkg.name.c_str() != m_name)
		|| (check_category && pkg.category.c_str() != m_category))
		return;

	bool rvalue = false;
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
	if(strcmp(pkg.name.c_str(), m_name.c_str()) != 0)
		return false;
	if(strcmp(pkg.category.c_str(), m_category.c_str()) != 0)
		return false;
	for(Package::iterator i = pkg.begin();
		i != pkg.end();
		++i)
	{
		if(test(*i))
			return true;
	}
	return false;
}

/** Sets the stability & masked members of ve according to the mask
 * @param ve Version instance to be set */
void Mask::apply(Version *ve, Keywords::Redundant check)
{
	if(	(m_type == maskAllowedByProfile
			&& ve->isProfileMask())      /* Won't change anything cause already masked by profile */
		|| (m_type == maskInSystem
			&& ve->isSystem()
			&& ve->isProfileMask()))
		return;

	switch(m_type) {
		case maskUnmask:
			if(!test(ve))
				break;
			if(check & Keywords::RED_DOUBLE_UNMASK)
			{
				if(ve->wanted_unmasked())
					ve->set_redundant(Keywords::RED_DOUBLE_UNMASK);
				ve->set_wanted_unmasked();
			}
			if(ve->isPackageMask())
			{
				*ve &= ~Keywords::PACKAGE_MASK;
				if(check & Keywords::RED_UNMASK)
					ve->set_was_unmasked();
			}
			break;
		case maskMask:
			if(!test(ve))
				break;
			if(check & Keywords::RED_DOUBLE_MASK)
			{
				if(ve->wanted_masked())
					ve->set_redundant(Keywords::RED_DOUBLE_MASK);
				ve->set_wanted_masked();
			}
			if(!ve->isPackageMask())
			{
				*ve |= Keywords::PACKAGE_MASK;
				if(check & Keywords::RED_MASK)
					ve->set_was_masked();
			}
			break;
		case maskInSystem:
			if( test(ve) )
				*ve |= Keywords::SYSTEM_PACKAGE;
			else
				*ve |= Keywords::PROFILE_MASK;
			break;
		case maskAllowedByProfile:
			if(!test(ve))
				*ve |= Keywords::PROFILE_MASK;
			break;
		case maskTypeNone:
			break;
	}
	return;
}
