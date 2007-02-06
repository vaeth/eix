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

#include <string>
#include <vector>
#include <database/io.h>

class Suffix
{
	protected:
		typedef  io::Char Level;
		static const unsigned short Levelsize = io::Charsize;
		Level m_suffixlevel;

		typedef  io::Long Num;
		static const unsigned short Numsize = io::Longsize;
		Num      m_suffixnum;

		Suffix(Level suffixlevel, Num suffixnum) :
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

		Num getNum() const
		{ return m_suffixnum; }
};

/** Parse and represent a portage version-string. */
class BasicVersion
{
	public:
		typedef io::Char Primchar;
		static const unsigned short Primcharsize = io::Charsize;

		typedef io::Short Gentoorevision;
		static const unsigned short Gentoorevisionsize = io::Shortsize;

		typedef io::Long Num;
		static const unsigned short Numsize = io::Longsize;

		/** The slot, the version represents.
		    For saving space, the default "0" is always stored as "" */
		std::string slot;

		/** Parse the version-string pointed to by str.
		 * If str is NULL, no parsing is done. */
		BasicVersion(const char *str = NULL);

		/** Preset everything with defaults. */
		void defaults();

		/** Parse the version-string pointed to by str. */
		void parseVersion(const char *str, int n = 0);

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
		bool operator <  (const BasicVersion& right) const;
		bool operator >  (const BasicVersion& right) const;
		bool operator == (const BasicVersion& right) const;
		bool operator != (const BasicVersion& right) const;
		bool operator >= (const BasicVersion& right) const;
		bool operator <= (const BasicVersion& right) const;

		// Getters for protected members
		Primchar getPrimarychar() const
		{ return m_primarychar; }
		Gentoorevision getGentooRevision() const
		{ return m_gentoorevision; }
		const char   *getFull() const
		{ return m_full.c_str(); }
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

		/** Splitted m_primsplit-version. */
		std::vector<Num>       m_primsplit;

		/** Optional one-character suffix of m_primsplit. */
		unsigned char          m_primarychar;

		/** Splitted suffices */
		std::vector<Suffix>    m_suffix;

		/** The optional gentoo-revision. */
		Gentoorevision         m_gentoorevision;

		/** Parse the m_primsplit-part of a version-string.
		 * Return pointer to the end of the m_primsplit-version.
		 * Thus, if this returns a pointer to '\0', there is nothing more to parse. */
		const char *parsePrimary(const char *str);
	private:
};

#endif /* __BASICVERSION_H__ */
