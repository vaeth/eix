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

const char *BasicVersion::suffixlevels[]     = { "alpha", "beta", "pre", "rc", "", "p" };
const char  BasicVersion::no_suffixlevel     = 4;
const int   BasicVersion::suffix_level_count = sizeof(suffixlevels)/sizeof(char*);

BasicVersion::BasicVersion(const char *str)
{
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
	m_suffixlevel   = no_suffixlevel;
	m_suffixnum     = 0;
	m_gentoorevision = 0;
}

void
BasicVersion::parseVersion(const char *str, int n)
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
		str = parseSuffix(str);
		if(*str != '\0')
		{
			cerr << "Garbage at end of version string: " << str << endl;
		}
	}

	// Let's remove useless 0 at the end
	for(vector<unsigned short>::reverse_iterator ri = m_primsplit.rbegin();
		ri != m_primsplit.rend() && *ri == 0;
		ri++)
	{
		m_primsplit.pop_back();
	}
}

/** Compares the split m_primsplit numbers of another BasicVersion instances to itself. */
int BasicVersion::comparePrimary(const BasicVersion& basic_version) const
{
	int splits = min(m_primsplit.size(), basic_version.m_primsplit.size());

	/* Compare the splitted m_primsplit version numbers from left to basic_version. */
	for(int i = 0; i<splits; i++) {
		if(m_primsplit[i] < basic_version.m_primsplit[i])
			return -1;
		else if(m_primsplit[i] > basic_version.m_primsplit[i])
			return 1;
	}
	/* The one with the bigger amount of versionsplits is our winner */
	int x = (- basic_version.m_primsplit.size() + m_primsplit.size());

	if(x > 0)
		return 1;

	if(x < 0)
		return -1;
	
	return 0;
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
BasicVersion::compare(const BasicVersion &basic_version) const
{
	int ret = comparePrimary(basic_version);
	if(ret != 0) return ret;

	if( m_primarychar < basic_version.m_primarychar ) return -1;
	if( m_primarychar > basic_version.m_primarychar ) return  1;

	if( m_suffixlevel < basic_version.m_suffixlevel ) return -1;
	if( m_suffixlevel > basic_version.m_suffixlevel ) return  1;

	if( m_suffixnum < basic_version.m_suffixnum ) return -1;
	if( m_suffixnum > basic_version.m_suffixnum ) return  1;

	if( m_gentoorevision < basic_version.m_gentoorevision ) return -1;
	if( m_gentoorevision > basic_version.m_gentoorevision ) return  1;

	return 0;
}

const char *
BasicVersion::parsePrimary(const char *str)
{
	string buf;
	while(*str)
	{
		if(*str == '.')
		{
			m_primsplit.push_back(atoi(buf.c_str()));
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
		m_primsplit.push_back(atoi(buf.c_str()));
	}

	if(isalpha(*str))
	{
		m_primarychar = *str++;
	}
	return str;
}

const char *
BasicVersion::parseSuffix(const char *str)
{
	if(*str == '_')
	{
		++str;
		for(int i = 0; i < suffix_level_count; ++i)
		{
			if(i != no_suffixlevel
			   && strncmp(suffixlevels[i], str, strlen(suffixlevels[i])) == 0)
			{
				m_suffixlevel = i;
				str += strlen(suffixlevels[i]);
				// get suffix-level number .. "_pre123"
				// I don't really understand why this wants a "char **", and
				// not a "const char **"
				m_suffixnum = strtol(str, (char **)&str, 10); 
				break;
			}
		}
	}
	else
	{
		m_suffixlevel = no_suffixlevel;
	}

	// get optional gentoo revision
	if(!strncmp("-r", str, 2))
	{
		str += 2;
		m_gentoorevision = strtol(str, (char **)&str, 10);
	}
	else
	{
		m_gentoorevision = 0;
	}
	return str;
}
