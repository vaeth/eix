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

#ifndef __BASICVERSION_H__
#define __BASICVERSION_H__

#include <string>
#include <vector>

using namespace std;

/** Parse and represent a portage version-string. */
class BasicVersion {
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

		/** Compares the split m_primsplit numbers of another BasicVersion instances to itself. */
		int comparePrimary(const BasicVersion& basic_version) const;

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
		unsigned char getPrimarychar() const
		{ return m_primarychar; }
		unsigned char getSuffixlevel() const
		{ return m_suffixlevel; }
		unsigned int  getSuffixnum() const
		{ return m_suffixnum; }
		unsigned char getGentooRevision() const
		{ return m_gentoorevision; }
		const char   *getFull() const
		{ return m_full.c_str(); }

	protected:
		/** The m_full version-string. */
		string                 m_full;
		/** Splitted m_primsplit-version. */
		vector<unsigned short> m_primsplit;
		/** Optional one-character suffix of m_primsplit. */
		unsigned char          m_primarychar;

		/** Index of optional suffix in suffixlevels. */
		unsigned char          m_suffixlevel;
		/** BasicVersion of suffix. */
		unsigned int           m_suffixnum;

		/** The optional gentoo-revision. */
		unsigned char          m_gentoorevision;

		/** Parse the m_primsplit-part of a version-string.
		 * Return pointer to the end of the m_primsplit-version.
		 * Thus, if this returns a pointer to '\0', there is nothing more to parse. */
		const char *parsePrimary(const char *str);

		/** Parse everything that is not the m_primsplit-part of a version-string.
		 * All prefixes and other stuff. */
		const char *parseSuffix(const char *str);

	private:
};

#endif /* __BASICVERSION_H__ */
