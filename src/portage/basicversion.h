#ifndef __BASICVERSION_H__
#define __BASICVERSION_H__

#include <iostream>
#include <string>
#include <vector>

using namespace std;

/** Parse and represent a portage version-string. */
class BasicVersion
{
	public:
		/** Suffixes allowed by portage (_preX, _pX, _alphaX, ..). */
		static const char *suffixlevels[];
		/** Index in suffixlevels where versions without a index are located. */
		static const char no_suffixlevel;
		/** Number of elements in suffixlevels. */
		static const int  suffix_level_count;

		/** Parse the version-string pointed to by str.
		 * If str is NULL, no parsing is done. */
		BasicVersion(const char *str = NULL);

		/** Preset everything with defaults. */
		void defaults();
		
		/** Parse the version-string pointed to by str. */
		void parseVersion(const char *str, int n = 0);

		/** Compares the split primsplit numbers of another BasicVersion instances to itself. */
		int comparePrimary(const BasicVersion& basic_version) const;

		const string& toString() const;

		bool operator <  (const BasicVersion& right) const;
		bool operator >  (const BasicVersion& right) const;
		bool operator == (const BasicVersion& right) const;
		bool operator != (const BasicVersion& right) const;
		bool operator >= (const BasicVersion& right) const;
		bool operator <= (const BasicVersion& right) const;

		unsigned char get_primarychar() const;
		unsigned char get_suffixlevel() const;
		unsigned int  get_suffixnum() const;
		unsigned char get_gentoorelease() const;

		friend ostream& operator<< (ostream& os, BasicVersion& e);

	protected:
		/** The full version-string. */
		string                 full;
		/** Splitted primsplit-version. */
		vector<unsigned short> primsplit;
		/** Optional one-character suffix of primsplit. */
		unsigned char          primarychar;

		/** Index of optional suffix in suffixlevels. */
		unsigned char          suffixlevel;
		/** BasicVersion of suffix. */
		unsigned int           suffixnum;

		/** The optional gentoo-revision. */
		unsigned char          gentoorelease;

		/** Parse the primsplit-part of a version-string.
		 * Return pointer to the end of the primsplit-version.
		 * Thus, if this returns a pointer to '\0', there is nothing more to parse. */
		const char *parse_primary(const char *str);

		/** Parse everything that is not the primsplit-part of a version-string.
		 * All prefixes and other stuff. */
		const char *parse_suffix(const char *str);

	private:
};

inline unsigned char 
BasicVersion::get_primarychar() const
{
	return primarychar;
}

inline unsigned char
BasicVersion::get_suffixlevel() const
{
	return suffixlevel;
}

inline unsigned int
BasicVersion::get_suffixnum() const
{
	return suffixnum;
}

inline unsigned char
BasicVersion::get_gentoorelease() const
{
	return gentoorelease;
}

inline ostream& 
operator<< (ostream& os, BasicVersion& e)
{
	return (os << e.toString());
}

#endif /* __BASICVERSION_H__ */
