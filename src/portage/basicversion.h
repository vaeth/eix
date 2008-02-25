// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __BASICVERSION_H__
#define __BASICVERSION_H__

/* When this is defined, we store additionally the full versiontext
   in the database.
   This takes more space, but is faster, of course, and we need not take care
   to save data in such a way that the versiontext can be constructed
   (e.g. omitted -r, trailing .0's, trailing garbage). */
/*#define SAVE_VERSIONTEXT 1*/

#include <string>
#include <vector>
#include <iostream>

#include <database/io.h>
#include <eixTk/stringutils.h>

class LeadNum
{
	protected:
		std::string m_text;

		bool m_is_zero, m_is_magic;

		void set_flags();

		// The following two methods are exclusively meant for io:
		const char *represent() const
		{
			const char *m_magic = "-";
			if(m_is_magic)
				return m_magic;
			return m_text.c_str();
		}
		LeadNum(const char *str) : m_text(str)
		{
			if(m_text == "-") {
				set_magic();
				return;
			}
			set_flags();
		}

	public:
		friend void    io::write_LeadNum(FILE *fp, const LeadNum &n);
		friend LeadNum io::read_LeadNum(FILE *fp);

		LeadNum()
		{ }

		LeadNum(bool ismagic) : m_is_magic(ismagic)
		{ m_is_zero = true; m_text.clear(); }

		LeadNum(const std::string &str) : m_text(str)
		{ set_flags(); }

		const char *parse(const char *str);

		void clear()
		{ m_text.clear(); m_is_zero = true; m_is_magic = false; }

		/// Note that also magic should be zero..
		bool is_zero() const
		{ return m_is_zero; }

		bool leadzero() const
		{ return (*(m_text.c_str()) == '0'); }

		const char *c_str() const
		{ return m_text.c_str(); }

		std::string::size_type size() const
		{ return m_text.size(); }

		/// Set a magic 0 value which is smaller than any allowed value.
		/// Observe that compare and iszero must be treated correspondingly.
		void set_magic()
		{ m_is_zero = m_is_magic = true; m_text.clear(); }

		bool is_magic() const
		{ return m_is_magic; }

		/// Compare two LeadNums.
		static int compare(const LeadNum& left, const LeadNum &right);
};

// Short compare-stuff
inline bool operator <  (const LeadNum& left, const LeadNum& right) 
{ return (LeadNum::compare(left, right) < 0); }
inline bool operator >  (const LeadNum& left, const LeadNum& right) 
{ return (LeadNum::compare(left, right) > 0); }
inline bool operator == (const LeadNum& left, const LeadNum& right)
{ return (LeadNum::compare(left, right) == 0); }
inline bool operator != (const LeadNum& left, const LeadNum& right)
{ return (LeadNum::compare(left, right) != 0); }
inline bool operator >= (const LeadNum& left, const LeadNum& right)
{ return (LeadNum::compare(left, right) >= 0); }
inline bool operator <= (const LeadNum& left, const LeadNum& right)
{ return (LeadNum::compare(left, right) <= 0); }

class Suffix
{
	protected:
		typedef  io::UChar Level;
		Level m_suffixlevel;

		LeadNum m_suffixnum;

		Suffix(Level suffixlevel, LeadNum suffixnum) :
		m_suffixlevel(suffixlevel), m_suffixnum(suffixnum)
		{ }

	private:
		/** Suffixes allowed by portage (_preX, _pX, _alphaX, ..). */
		static const char *suffixlevels[];
		/** Index in suffixlevels where versions without a index are located. */
		static const Level no_suffixlevel;
		/** Number of elements in suffixlevels. */
		static const Level suffix_level_count;

	public:
		friend void     io::write_version(FILE *fp, const Version *v);
		friend Version *io::read_version(FILE *fp);

		Suffix()
		{ defaults(); }

		void defaults();
		bool parse(const char **str_ref);

		int  compare(const Suffix &b) const;

		Level getLevel() const
		{ return m_suffixlevel; }

		LeadNum getNum() const
		{ return m_suffixnum; }

#if !defined(SAVE_VERSIONTEXT)
		std::string toString() const;
#endif
};

/** Parse and represent a portage version-string. */
class BasicVersion
{
	public:
		typedef io::UChar Primchar;

		/** Parse the version-string pointed to by str.
		 * If str is NULL, no parsing is done. */
		BasicVersion(const char *str = NULL);

		/** Preset everything with defaults. */
		void defaults();

		/** Parse the version-string pointed to by str. */
		void parseVersion(const char *str, size_t n = 0);

		/// Compare the split m_primsplit numbers
		static int comparePrimary(const BasicVersion& left, const BasicVersion& right);

		/// Compare the split m_suffixes
		static int compareSuffix(const BasicVersion& right, const BasicVersion& left);

		/// Compare all except gentoo revisions
		static int compareTilde(const BasicVersion& right, const BasicVersion& left);

		/// Compare the m_full version.
		static int compare(const BasicVersion& right, const BasicVersion& left);

		// Getters for protected members
		Primchar getPrimarychar() const
		{ return m_primarychar; }

		LeadNum getGentooRevision() const
		{ return m_gentoorevision; }

		const char *getFull() const
		{
#if !defined(SAVE_VERSIONTEXT)
			if(m_full.empty())
				(const_cast<BasicVersion*>(this))->calc_full();
#endif
			return m_full.c_str();
		}

#if !defined(SAVE_VERSIONTEXT)
		void calc_full();
#endif

		const std::vector<Suffix> &getSuffix() const
		{ return m_suffix; }

	protected:
		/** The m_full version-string. */
		std::string            m_full;

#if !defined(SAVE_VERSIONTEXT)
		/** Garbage at end of version-string */
		std::string            m_garbage;
#endif
		/** Splitted m_primsplit-version. */
		std::vector<LeadNum>   m_primsplit;

		/** Optional one-character suffix of m_primsplit. */
		Primchar               m_primarychar;

		/** Splitted suffices */
		std::vector<Suffix>    m_suffix;

		/** The optional gentoo-revision. */
		LeadNum                m_gentoorevision;

		/** The optional prefix-portage subrevision. */
		LeadNum                m_subrevision;

		/** Parse the m_primsplit-part of a version-string.
		 * Return pointer to the end of the m_primsplit-version.
		 * Thus, if this returns a pointer to '\0', there is nothing more to parse. */
		const char *parsePrimary(const char *str);
	private:
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
		typedef io::UChar Restrict;

		static const Restrict
			RESTRICT_NONE   = 0x00,
			RESTRICT_FETCH  = 0x01,
			RESTRICT_MIRROR = 0x02;

		Restrict restrictFlags;

		/** The slot, the version represents.
		    For saving space, the default "0" is always stored as "" */
		std::string slotname;

		ExtendedVersion(const char *str = NULL) : BasicVersion(str)
		{ restrictFlags = RESTRICT_NONE; slotname.clear(); }

		static Restrict calcRestrict(const std::string& str);

		void set_restrict(const std::string& str)
		{ restrictFlags = calcRestrict(str); }

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
