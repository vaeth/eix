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
#include <database/io.h>

class LeadNum
{
	protected:
		typedef  io::Long Num;
		static const unsigned short Numsize = io::Longsize;
		Num      m_num;

		typedef  io::Char Lead;
		static const unsigned short Leadsize = io::Charsize;
		Lead     m_lead;

		static Num strtoNum(const char *str, char **s, int index);
	public:
		friend void    io::write_LeadNum(FILE *fp, const LeadNum &n);
		friend LeadNum io::read_LeadNum(FILE *fp);

		LeadNum()
		{ }

		LeadNum(const char *str)
		{ parse(str); }

		const char *parse(const char *str);

		void  clear()
		{ m_num = 0; m_lead = 0; }

		/// Note that also magic should be zero..
		bool iszero()
		{ return (m_num == 0); }

#if !defined(SAVE_VERSIONTEXT)
		std::string toString() const;
#endif

		/// set a magic value which is smaller than any allowed value.
		/// Observe that compare and iszero must be treated correspondingly.
		void set_magic()
		{ m_lead = 0xff; m_num = 0; }

		bool is_magic()
		{ return ((m_lead == 0xff) && (m_num == 0)); }

		/// Compare two LeadNums.
		int compare(const LeadNum &right) const;

		// Short compare-stuff
		bool operator <  (const LeadNum& right) const
		{ return (compare(right) < 0); }
		bool operator >  (const LeadNum& right) const
		{ return (compare(right) > 0); }
		bool operator == (const LeadNum& right) const
		{ return (compare(right) == 0); }
		bool operator != (const LeadNum& right) const
		{ return (compare(right) != 0); }
		bool operator >= (const LeadNum& right) const
		{ return (compare(right) >= 0); }
		bool operator <= (const LeadNum& right) const
		{ return (compare(right) <= 0); }
};

class Suffix
{
	protected:
		typedef  io::Char Level;
		static const unsigned short Levelsize = io::Charsize;
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
		friend void     io::write_version(FILE *fp, const Version *v, bool small);
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
		typedef io::Char Primchar;
		static const unsigned short Primcharsize = io::Charsize;

		/** The slot, the version represents.
		    For saving space, the default "0" is always stored as "" */
		std::string slot;

		/** Parse the version-string pointed to by str.
		 * If str is NULL, no parsing is done. */
		BasicVersion(const char *str = NULL);

		/** Preset everything with defaults. */
		void defaults();

		/** Parse the version-string pointed to by str. */
		void parseVersion(const char *str, size_t n = 0);

		/** Compares the split m_primsplit numbers of another BasicVersion
		    instances to itself. */
		int comparePrimary(const BasicVersion& basic_version) const;

		/** Compares the split m_suffixes of another BasicVersion
		    instances to itself. */
		int compareSuffix(const BasicVersion& b) const;

		/// Compare all except gentoo revisions
		int compare_tilde(const BasicVersion &basic_version) const;

		/// Compare the m_full version.
		int compare(const BasicVersion &basic_version) const;

		// Short compare-stuff
		bool operator <  (const BasicVersion& right) const
		{ return (compare(right) < 0); }
		bool operator >  (const BasicVersion& right) const
		{ return (compare(right) > 0); }
		bool operator == (const BasicVersion& right) const
		{ return (compare(right) == 0); }
		bool operator != (const BasicVersion& right) const
		{ return (compare(right) != 0); }
		bool operator >= (const BasicVersion& right) const
		{ return (compare(right) >= 0); }
		bool operator <= (const BasicVersion& right) const
		{ return (compare(right) <= 0); }

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

		std::string getSlotAppendix (bool colon) const
		{
			if(slot.length())
			{
				if(colon)
					return std::string(":") + slot;
				return std::string("(") + slot + ")";
			}
			return "";
		}

		std::string getFullSlotted (bool colon, const std::string& intermediate = "") const
		{ return std::string(getFull()) + intermediate + getSlotAppendix(colon); }
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

		/** Parse the m_primsplit-part of a version-string.
		 * Return pointer to the end of the m_primsplit-version.
		 * Thus, if this returns a pointer to '\0', there is nothing more to parse. */
		const char *parsePrimary(const char *str);
	private:
};

#endif /* __BASICVERSION_H__ */
