#include "basicversion.h"

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
	full.clear();
	primsplit.clear();
	primarychar   = '\0';
	suffixlevel   = no_suffixlevel;
	suffixnum = 0;
	gentoorelease     = 0;
}

void
BasicVersion::parseVersion(const char *str, int n)
{
	defaults();
	if(n > 0)
	{
		full = string(str, n);
	}
	else
	{
		full = string(str);
	}
	str = parse_primary(full.c_str());
	if(*str)
	{
		str = parse_suffix(str);
		if(*str != '\0')
		{
			cerr << "Garbage at end of version string: " << str << endl;
		}
	}
}

const string&
BasicVersion::toString() const
{
	return full;
}

/** Compares the split primsplit numbers of another BasicVersion instances to itself. */
int BasicVersion::comparePrimary(const BasicVersion& basic_version) const
{
	int splits = min(primsplit.size(), basic_version.primsplit.size());

	/* Compare the splitted primsplit version numbers from left to basic_version. */
	for(int i = 0; i<splits; i++) {
		if(primsplit[i] < basic_version.primsplit[i])
			return -1;
		else if(primsplit[i] > basic_version.primsplit[i])
			return 1;
	}
	/* The one with the bigger amount of versionsplits is our winner */
	return (- basic_version.primsplit.size() + primsplit.size());
}

bool BasicVersion::operator <  (const BasicVersion& right) const
{
	int res = comparePrimary(right);

	if( res < 0 ) return true;
	if( res > 0 ) return false;

	if( primarychar < right.primarychar ) return true;
	if( primarychar > right.primarychar ) return false;

	if( suffixlevel < right.suffixlevel ) return true;
	if( suffixlevel > right.suffixlevel ) return false;
	if( suffixnum < right.suffixnum ) return true;
	if( suffixnum > right.suffixnum ) return false;
	if( gentoorelease < right.gentoorelease ) return true;
	return false;
}

bool BasicVersion::operator >  (const BasicVersion& right) const
{
	int res = comparePrimary(right);
	if( res > 0 ) return true;
	if( res < 0 ) return false;

	if( primarychar > right.primarychar ) return true;
	if( primarychar < right.primarychar ) return false;
	if( suffixlevel > right.suffixlevel ) return true;
	if( suffixlevel < right.suffixlevel ) return false;
	if( suffixnum > right.suffixnum ) return true;
	if( suffixnum < right.suffixnum ) return false;
	if( gentoorelease > right.gentoorelease ) return true;
	return false;
}

bool BasicVersion::operator == (const BasicVersion& right) const
{
	int res = comparePrimary(right);

	if( res != 0 ) return false;
	if( primarychar != right.primarychar ) return false;
	if( suffixlevel != right.suffixlevel ) return false;
	if( suffixnum != right.suffixnum ) return false;
	if( gentoorelease != right.gentoorelease ) return false;
	return true;
}

bool BasicVersion::operator != (const BasicVersion& right) const
{
	return !(*this == right);
}

bool BasicVersion::operator >= (const BasicVersion& right) const
{
	return ( (*this>right) || (*this==right) );
}

bool BasicVersion::operator <= (const BasicVersion& right) const
{
	return ( (*this<right) || (*this==right) );
}

const char *
BasicVersion::parse_primary(const char *str)
{
	string buf;
	while(*str)
	{
		if(*str == '.')
		{
			primsplit.push_back(atoi(buf.c_str()));
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
		primsplit.push_back(atoi(buf.c_str()));
	}

	if(isalpha(*str))
	{
		primarychar = *str++;
	}
	return str;
}

const char *
BasicVersion::parse_suffix(const char *str)
{
	if(*str == '_')
	{
		++str;
		for(int i = 0; i < suffix_level_count; ++i)
		{
			if(i != no_suffixlevel && !strncmp(suffixlevels[i], str, strlen(suffixlevels[i])))
			{
				suffixlevel = i;
				str += strlen(suffixlevels[i]);
				// get suffix-level number .. "_pre123"
				// I don't really understand why this wants a "char **", and not a "const char **"
				suffixnum = strtol(str, (char **)&str, 10); 
				break;
			}
		}
	}
	else
	{
		suffixlevel = no_suffixlevel;
	}

	// get optional gentoo revision
	if(!strncmp("-r", str, 2))
	{
		str += 2;
		gentoorelease = strtol(str, (char **)&str, 10);
	}
	else
	{
		gentoorelease = 0;
	}
	return str;
}
