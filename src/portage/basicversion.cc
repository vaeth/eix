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

#include "basicversion.h"

#include <iostream>
#include <sstream>

using namespace std;

void
LeadNum::set_flags()
{
	m_is_magic = false;
	for(std::string::const_iterator it = m_text.begin();
		it != m_text.end(); ++it) {
		if(*it != '0') {
			m_is_zero = false;
			return;
		}
	}
	m_is_zero = true;
}

const char *
LeadNum::parse(const char *str)
{
	m_is_magic = false;
	const char *s = str;
	bool is_zero = true;
	char c = *s;
	while(isdigit(c)) {
		if(c != '0')
			is_zero = false;
		c = *(++s);
	}
	m_is_zero = is_zero;
	m_text = string(str, s);
	return s;
}

int
LeadNum::compare(const LeadNum &right) const
{
	/* If you modify this, do not forget that the magic value must be
	*  the smallest one. */

	// We change the order for speed: Common cases first, if possible.
	if(m_is_zero) {
		if(!right.m_is_zero)
			return -1;
		// both values are 0:
		if(m_is_magic) {
			if(right.m_is_magic)
				return 0;
			return -1;
		}
		if(right.m_is_magic)
			return 1;
		string::size_type l = size();
		string::size_type r = right.size();
		if(l == r)
			return 0;
		// "" < "0" < "00" < "000" < ...
		if(l < r)
			return -1;
		return 1;
	}
	if(right.m_is_zero)
		return 1;

	// both values are nonzero:
	if(leadzero()) {
		if(right.leadzero())
			return strcmp(c_str(), right.c_str());
		return -1;
	}
	if(right.leadzero())
		return 1;

	// both values are without leading zero:
	string::size_type l = size();
	string::size_type r = right.size();
	if(l == r)
		return strcmp(c_str(), right.c_str());
	if(l < r)
		return -1;
	return 1;
}

const char *Suffix::suffixlevels[]             = { "alpha", "beta", "pre", "rc", "", "p" };
const Suffix::Level Suffix::no_suffixlevel     = 4;
const Suffix::Level Suffix::suffix_level_count = sizeof(suffixlevels)/sizeof(char*);

void
Suffix::defaults()
{
	m_suffixlevel = no_suffixlevel;
	m_suffixnum.clear();
}

#if !defined(SAVE_VERSIONTEXT)
string
Suffix::toString() const
{
	string ret = "_";
	ret.append(suffixlevels[m_suffixlevel]);
	ret.append(m_suffixnum.c_str());
	return ret;
}
#endif

bool
Suffix::parse(const char **str_ref)
{
	const char *str = *str_ref;
	if(*str == '_')
	{
		++str;
		for(Level i = 0; i != suffix_level_count; ++i)
		{
			if(i != no_suffixlevel
			   && strncmp(suffixlevels[i], str, strlen(suffixlevels[i])) == 0)
			{
				m_suffixlevel = i;
				str += strlen(suffixlevels[i]);
				// get suffix-level number .. "_pre123"
				*str_ref = m_suffixnum.parse(str);
				return true;
			}
		}
	}
	defaults();
	return false;
}

int
Suffix::compare(const Suffix &b) const
{
	if( m_suffixlevel < b.m_suffixlevel ) return -1;
	if( m_suffixlevel > b.m_suffixlevel ) return  1;

	if( m_suffixnum < b.m_suffixnum ) return -1;
	if( m_suffixnum > b.m_suffixnum ) return  1;

	return 0;
}

BasicVersion::BasicVersion(const char *str)
{
	slot = "";
	if(str)
	{
		parseVersion(str);
	}
}

void
BasicVersion::defaults()
{
	m_full.clear();
#if !defined(SAVE_VERSIONTEXT)
	m_garbage.clear();
#endif
	m_primsplit.clear();
	m_primarychar   = '\0';
	m_suffix.clear();
	m_gentoorevision.clear();
}

#if !defined(SAVE_VERSIONTEXT)
void
BasicVersion::calc_full()
{
	m_full.empty();
	bool first = true;
	for(vector<LeadNum>::const_iterator it = m_primsplit.begin();
		it != m_primsplit.end(); ++it) {
		if(!first) {
			m_full.append(".");
		}
		first = false;
		m_full.append(it->c_str());
	}
	if(m_primarychar)
		m_full.append(string(1,m_primarychar));
	for(vector<Suffix>::const_iterator i = m_suffix.begin();
		i != m_suffix.end(); ++i) {
		m_full.append(i->toString());
	}
	if(!m_gentoorevision.is_magic()) {
		m_full.append("-r");
		m_full.append(m_gentoorevision.c_str());
	}
	m_full.append(m_garbage);
}
#endif

void
BasicVersion::parseVersion(const char *str, size_t n)
{
	defaults();
	if(n > 0)
	{
		m_full = string(str, n);
	}
	else
	{
		m_full = string(str);
	}
	str = parsePrimary(m_full.c_str());
	if(*str)
	{
		Suffix curr_suffix;
		while(curr_suffix.parse(&str))
			m_suffix.push_back(curr_suffix);

		// get optional gentoo revision
		if(!strncmp("-r", str, 2))
		{
			str += 2;
			str = m_gentoorevision.parse(str);
		}
		else
		{
			m_gentoorevision.set_magic();
		}
		if(*str != '\0')
		{
#if !defined(SAVE_VERSIONTEXT)
			m_garbage = str;
#endif
			cerr << "Garbage at end of version string: " << str << endl;
		}
	}
	else
	{
		m_gentoorevision.set_magic();
	}

#if defined(SAVE_VERSIONTEXT)
	// Let's remove useless 0 at the end
	for(vector<LeadNum>::reverse_iterator ri = m_primsplit.rbegin();
		ri != m_primsplit.rend() && ri->iszero();
		ri++)
	{
		m_primsplit.pop_back();
	}
#endif
}

/** Compares the split m_primsplit numbers of another BasicVersion instances to itself. */
int BasicVersion::comparePrimary(const BasicVersion& b) const
{
	vector<LeadNum>::const_iterator ait = m_primsplit.begin();
	vector<LeadNum>::const_iterator bit = b.m_primsplit.begin();
	for( ; (ait != m_primsplit.end()) && (bit != b.m_primsplit.end());
		++ait, ++bit)
	{
		if(*ait < *bit)
			return -1;
		if(*ait > *bit)
			return 1;
	}
	/* The one with the bigger amount of versionsplits is our winner */
	if(ait != m_primsplit.end())
		return 1;
	if(bit != b.m_primsplit.end())
		return -1;
	return 0;
}

/** Compares the split m_suffixes of another BasicVersion instances to itself. */
int BasicVersion::compareSuffix(const BasicVersion& b) const
{
	vector<Suffix>::const_iterator ait = m_suffix.begin();
	vector<Suffix>::const_iterator bit = b.m_suffix.begin();
	for( ; (ait != m_suffix.end()) && (bit != b.m_suffix.end());
		++ait, ++bit)
	{
		int ret = ait->compare(*bit);
		if(ret)
			return ret;
	}
	static const Suffix empty;
	const Suffix &aref = (ait == m_suffix.end()) ? empty : *ait;
	const Suffix &bref = (bit == b.m_suffix.end()) ? empty : *bit;
	return aref.compare(bref);
}

int
BasicVersion::compare_tilde(const BasicVersion &basic_version) const
{
	int ret = comparePrimary(basic_version);
	if(ret) return ret;

	if( m_primarychar < basic_version.m_primarychar ) return -1;
	if( m_primarychar > basic_version.m_primarychar ) return  1;

	return compareSuffix(basic_version);
}

int
BasicVersion::compare(const BasicVersion &basic_version) const
{
	int ret = compare_tilde(basic_version);
	if(ret) return ret;

	if( m_gentoorevision < basic_version.m_gentoorevision ) return -1;
	if( m_gentoorevision > basic_version.m_gentoorevision ) return  1;

#if defined(SAVE_VERSIONTEXT)
	// The numbers are equal, but the strings might still be different,
	// e.g. because of garbage or removed trailing ".0"s.
	// In such a case, we simply compare the strings alphabetically.
	// This is not always what you want but at least reproducible.
	return strcmp(getFull(), basic_version.getFull());
#else
	return strcmp(m_garbage.c_str(), basic_version.m_garbage.c_str());
#endif
}

const char *
BasicVersion::parsePrimary(const char *str)
{
	string buf;
	while(*str)
	{
		if(*str == '.') {
			m_primsplit.push_back(LeadNum(buf));
			buf.clear();
		}
		else if(isdigit(*str))
			buf.push_back(*str);
		else
			break;
		++str;
	}

	if(!buf.empty())
		m_primsplit.push_back(LeadNum(buf));

	if(isalpha(*str))
		m_primarychar = *str++;
	return str;
}

