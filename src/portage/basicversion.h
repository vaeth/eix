// vim:set noet cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__BASICVERSION_H__
#define EIX__BASICVERSION_H__ 1

#include <eixTk/stringutils.h>

#include <list>

/** Parse and represent a portage version-string. */
class BasicVersion
{
public:
	enum PartType
	{
		garbage,
		alpha,
		beta,
		pre,
		rc,
		revision,
		inter_rev,
		patch,
		character,
		primary,
		first
	};

	// This must be larger than PartType elements and should be a power of 2.
	static const std::string::size_type max_type = 32;

	typedef std::pair<PartType,std::string> Part;

	/// Parse the version-string pointed to by str. If str is NULL, no parsing is done.
	BasicVersion(const char *str = NULL);

	virtual ~BasicVersion() { }

	/// Parse the version-string pointed to by str.
	void parseVersion(const std::string& str);

	/// Compare all except gentoo revisions
	static int compareTilde(const BasicVersion& right, const BasicVersion& left);

	/// Compare the m_cached_full version.
	static int compare(const BasicVersion& right, const BasicVersion& left);

	const std::string& getFull() const
	{
		if(this->m_cached_full.empty())
			this->rebuild();
		return this->m_cached_full;
	}

	void rebuild() const;

protected:
	/// The full version-string. This is usually only used as a cache for the
	/// result of rebuild. That's why this is mutable
	mutable std::string m_cached_full;

	/// Splitted m_primsplit-version.
	std::list< Part > m_parts;

	static int compare(const Part& left, const Part& right);
};

// Short compare-stuff
inline bool operator <  (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) < 0; }
inline bool operator >  (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) > 0; }
inline bool operator == (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) == 0; }
inline bool operator != (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) != 0; }
inline bool operator >= (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) >= 0; }
inline bool operator <= (const BasicVersion& left, const BasicVersion& right)
{ return BasicVersion::compare(left, right) <= 0; }

#endif /* EIX__BASICVERSION_H__ */
