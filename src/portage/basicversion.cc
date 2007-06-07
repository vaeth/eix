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

using namespace std;

static inline BasicVersion::Num
strtoNum(const char *str, char **s, int index)
{
#if defined(HAVE_STRTOULL)
	return BasicVersion::Num(strtoull(str, s, index));
#else
	return BasicVersion::Num(strtol(str, s, index));
#endif
}

static inline BasicVersion::Num
atoNum(const char *str)
{
	char *s;
	return strtoNum(str, &s, 10);
}

const char *Suffix::suffixlevels[]             = { "alpha", "beta", "pre", "rc", "", "p" };
const Suffix::Level Suffix::no_suffixlevel     = 4;
const Suffix::Level Suffix::suffix_level_count = sizeof(suffixlevels)/sizeof(char*);

void
Suffix::defaults()
{
	m_suffixlevel = no_suffixlevel;
	m_suffixnum   = 0;
}

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
				// I don't really understand why this wants a "char **", and
				// not a "const char **"
				char *tail;
				m_suffixnum = strtoNum(str, &tail, 10);
				*str_ref = const_cast<const char *>(tail);
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
	m_primsplit.clear();
	m_primarychar   = '\0';
	m_suffix.clear();
	m_gentoorevision = 0;
}

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
			char *s;
			m_gentoorevision = strtol(str, &s, 10);
			str = s;
		}
		else
		{
			m_gentoorevision = 0;
		}
		if(*str != '\0')
		{
			cerr << "Garbage at end of version string: " << str << endl;
		}
	}

	// Let's remove useless 0 at the end
	for(vector<BasicVersion::Num>::reverse_iterator ri = m_primsplit.rbegin();
		ri != m_primsplit.rend() && *ri == 0;
		ri++)
	{
		m_primsplit.pop_back();
	}
}

/** Compares the split m_primsplit numbers of another BasicVersion instances to itself. */
int BasicVersion::comparePrimary(const BasicVersion& b) const
{
	vector<BasicVersion::Num>::const_iterator ait = m_primsplit.begin();
	vector<BasicVersion::Num>::const_iterator bit = b.m_primsplit.begin();
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

bool BasicVersion::operator <  (const BasicVersion& right) const
{
	return compare(right) == -1;
}

bool BasicVersion::operator >  (const BasicVersion& right) const
{
	return compare(right) == 1;
}

bool BasicVersion::operator == (const BasicVersion& right) const
{
	return compare(right) == 0;
}

bool BasicVersion::operator != (const BasicVersion& right) const
{
	return compare(right) != 0;
}

bool BasicVersion::operator >= (const BasicVersion& right) const
{
	int x = compare(right);
	return (x == 0 || x == 1);
}

bool BasicVersion::operator <= (const BasicVersion& right) const
{
	int x = compare(right);
	return (x == 0 || x == -1);
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

	// The numbers are equal, but the strings might still be different,
	// e.g. 1.02 is different from 1.002.
	// In such a case, we simply compare the strings alphabetically.
	// This is not always what you want but at least reproducible.
	return strcmp(getFull(), basic_version.getFull());
}

const char *
BasicVersion::parsePrimary(const char *str)
{
	string buf;
	while(*str)
	{
		if(*str == '.')
		{
			m_primsplit.push_back(atoNum(buf.c_str()));
			buf.clear();
		}
		else if(isdigit(*str))
		{
			buf.push_back(*str);
		}
		else
		{
			break;
		}
		++str;
	}

	if(buf.size() > 0)
	{
		m_primsplit.push_back(atoNum(buf.c_str()));
	}

	if(isalpha(*str))
	{
		m_primarychar = *str++;
	}
	return str;
}

