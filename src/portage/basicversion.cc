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

#include <eixTk/regexp.h>

// The various suffixes in descending order of significance
static char* suffixlevels[] = { "_alpha", "_beta", "_pre", "_rc", "", "_p" };
static const char numsuffixlevels = 6;

/** This regular expression is used to split of primary version strings form the rest */
Regex regex_ver_primary("\\([^_-]\\+\\).*", 0);

/** Tests if the primary version string can be split into integers */
inline bool canSplitPrimary( string& primary )
{
	for( size_t i=0; i<primary.size(); i++ )
		if( !isdigit( primary[i]) && !(primary[i]=='.') ) return false;
	return true;
}

/** Split a version string into its components */
void BasicVersion::parseVersion(const char* str, unsigned int n) throw(ExBasic)
{
	regmatch_t rm[10];
	int len;
	char* workstr;
	char* tempstr;

	full = str;
	workstr = strndup(str, n);
	OOM_ASSERT(workstr);
	tempstr = workstr;
	suffixlevel = 4;
	suffixnum = 0;
	gentoorelease = 0;

	// get primary BasicVersion
	if( regexec( regex_ver_primary.get(), workstr, 2, rm, 0  ) == 0 )
	{
		len = rm[1].rm_eo - rm[1].rm_so;
		primary = string( &(workstr[rm[1].rm_so]), len);
		tempstr += rm[1].rm_eo;
	}
	else
	{
		free(workstr);
		throw( ExBasic("regexec(\"%s\") failed. Can't determine primary version.", full.c_str()) );
	}

	// try to split primary version if it only contains numbers and optional periods
	if( canSplitPrimary( primary ) )
	{
		size_t splitpos, lastsplitpos=0;
		string tempsplitstring;
		do
		{
			splitpos = primary.find_first_not_of("0123456789",lastsplitpos);
			tempsplitstring = primary.substr(lastsplitpos,splitpos-lastsplitpos);
			lastsplitpos = splitpos+1;
			int ver = strtol(tempsplitstring.c_str(),NULL,10);
			primsplit.push_back( ver );
		}	while( splitpos != string::npos );
	}

	// get (optional) gentoo release number
	char* gr = strstr(tempstr, "-r");
	if( (gr!=NULL) && (strlen(gr)>2) ) {
		*(gr) = (char)0;
		gr+=2;
		char* tailptr;
		gentoorelease=strtol(gr,&tailptr,10);
		if( strlen(tailptr)>0 ) {
			free(workstr);
			throw(ExBasic("Invalid gentoo release number in \"%s\"", full.c_str()));
		}
	}

	// get (optional) suffix
	if( tempstr[0] == '_' )
	{
		for( int i=0; i<numsuffixlevels; i++ ) {
			if( i==4 ) continue;
			if( strncmp( tempstr, suffixlevels[i], strlen(suffixlevels[i]) ) == 0 ) {
				suffixlevel = i;
				// get (optional) suffix number
				char* suffixnumstr = &(tempstr[ strlen(suffixlevels[(int)suffixlevel]) ]);
				if( strlen(suffixnumstr)!=0 )
				{
					char* tailptr = NULL;
					suffixnum = strtol( suffixnumstr, &tailptr, 10 );
					if( (suffixnum==0) && (strlen(tailptr)>0) ) {
						free(workstr);
						throw(ExBasic("Invalid suffix number in \"%s\"", full.c_str()));
					}
					break;
				}
				else
				{
					suffixnum = 0;
					break;
				}
			}
		}
	}

	// clean up
	free( workstr );
}

/** Constructor, do nothing */
BasicVersion::BasicVersion()
{
}

/** Constructor, calls BasicVersion::parseVersion( str ) */
BasicVersion::BasicVersion( const char* str ) throw(ExBasic)
{
	parseVersion(str);
}

/** Constructor, calls BasicVersion::parseVersion( str ) */
BasicVersion::BasicVersion(string str) throw(ExBasic)
{
	parseVersion(str.c_str());
}

/** Compares the split primary numbers of another BasicVersion instances to itself.
 * If the primary string(s) could not be split, it does a simple std::string.compare() */
int BasicVersion::comparePrimary(const BasicVersion& basic_version )
{
	size_t splits = min( primsplit.size(), basic_version.primsplit.size());
	/* If parseVersion was unable to split the primary version string into integers, do a simple string comparison */
	if( splits == 0 ) {
		return primary.compare( basic_version.primary );
	}
	/* Compare the splitted primary version numbers from left to basic_version. */
	for(size_t i = 0; i<splits; i++) {
		if( primsplit[i] < basic_version.primsplit[i])
			return -1;
		else if( primsplit[i] > basic_version.primsplit[i])
			return 1;
	}
	/* The one with the bigger amount of versionsplits is our winner */
	return ( - basic_version.primsplit.size() + primsplit.size());
}

/** Comparison operator */
bool BasicVersion::operator< ( const BasicVersion& right )
{
	int res = comparePrimary(right);

	if( res < 0 ) return true;
	if( res > 0 ) return false;

	if( suffixlevel < right.suffixlevel ) return true;
	if( suffixlevel > right.suffixlevel ) return false;
	if( suffixnum < right.suffixnum ) return true;
	if( suffixnum > right.suffixnum ) return false;
	if( gentoorelease < right.gentoorelease ) return true;
	return false;
}

/** Comparison operator */
bool BasicVersion::operator> ( const BasicVersion& right )
{
	int res = comparePrimary(right);
	if( res > 0 ) return true;
	if( res < 0 ) return false;

	if( suffixlevel > right.suffixlevel ) return true;
	if( suffixlevel < right.suffixlevel ) return false;
	if( suffixnum > right.suffixnum ) return true;
	if( suffixnum < right.suffixnum ) return false;
	if( gentoorelease > right.gentoorelease ) return true;
	return false;
}

/** Comparison operator */
bool BasicVersion::operator== ( const BasicVersion& right )
{
	int res = comparePrimary(right);

	if( res != 0 ) return false;
	if( suffixlevel != right.suffixlevel ) return false;
	if( suffixnum != right.suffixnum ) return false;
	if( gentoorelease != right.gentoorelease ) return false;
	return true;
}
