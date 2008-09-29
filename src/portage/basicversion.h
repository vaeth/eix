// vim:set noet cinoptions=g0,t0,^-2 sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __BASICVERSION_H__
#define __BASICVERSION_H__

#include <string>
#include <list>

#include <database/types.h>
#include <eixTk/stringutils.h>

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

class ExtendedVersion : public BasicVersion
{
	public:
		typedef uint16_t Restrict;
		static const Restrict // order according to frequency...
			RESTRICT_NONE           = 0x0000,
			RESTRICT_BINCHECKS      = 0x0001,
			RESTRICT_STRIP          = 0x0002,
			RESTRICT_TEST           = 0x0004,
			RESTRICT_USERPRIV       = 0x0008,
			RESTRICT_INSTALLSOURCES = 0x0010,
			RESTRICT_FETCH          = 0x0020,
			RESTRICT_MIRROR         = 0x0040,
			RESTRICT_PRIMARYURI     = 0x0080,
			RESTRICT_BINDIST        = 0x0100;

		typedef io::UChar Properties;
		static const Properties // order according to frequency...
			PROPERTIES_NONE        = 0x00,
			PROPERTIES_INTERACTIVE = 0x01,
			PROPERTIES_LIVE        = 0x02,
			PROPERTIES_VIRTUAL     = 0x04,
			PROPERTIES_SET         = 0x08;

		Restrict restrictFlags;
		Properties propertiesFlags;

		/** The slot, the version represents.
		    For saving space, the default "0" is always stored as "" */
		std::string slotname;

		ExtendedVersion(const char *str = NULL) : BasicVersion(str)
		{ restrictFlags = RESTRICT_NONE; slotname.clear(); }

		static Restrict calcRestrict(const std::string& str);
		static Properties calcProperties(const std::string& str);

		void set_restrict(const std::string& str)
		{ restrictFlags = calcRestrict(str); }

		void set_properties(const std::string& str)
		{ propertiesFlags = calcProperties(str); }

		std::string getSlotAppendix(bool colon) const
		{
			if(slotname.empty())
				return "";
			if(colon)
				return std::string(":") + slotname;
			return std::string("(") + slotname + ")";
		}

		std::string getFullSlotted(bool colon, const std::string& intermediate = "") const
		{ return std::string(getFull()) + intermediate + getSlotAppendix(colon); }
};

#endif /* __BASICVERSION_H__ */
