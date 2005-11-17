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

#ifndef __VERSIONS_H__
#define __VERSIONS_H__

#include <eixTk/exceptions.h>

#include <vector>
#include <string>
#include <stdio.h>

using namespace std;

/** The BasicVersion class provides proper version number sorting (gentoo specific).
 * @see http://www.gentoo.org/proj/en/devrel/handbook/handbook.xml?part=2&chap=1#doc_chap2 */
class BasicVersion {

	public:
		friend class Mask;

	protected:
		/** Split a version string into its components */
		void parseVersion(const char* str, unsigned int n) throw(ExBasic);

		/** Split a version string into its components */
		void parseVersion(const char* str) throw(ExBasic) {
			parseVersion(str, strlen(str));
		}

		/* the following version variables are listed in descending significance */
		string primary;                   /**< version string, e.g. "1.3.5" */
		vector<unsigned short> primsplit; /**< primary string splitted into e.g. {1,3,5} [can be empty] */
		char suffixlevel;                 /**< suffix level, see char** suffix_levels */
		unsigned int suffixnum;           /**< trailing number after suffix, e.g. pre20041220 -> "20041220" */
		unsigned char gentoorelease;      /**< gentoo-specific release number */
		string full;                      /**< full version string */

	public:
		/** Default constructor, does nothing */
		BasicVersion();
		/** Constructor, calls BasicVersion::parseVersion( str ) */
		BasicVersion( const char* str ) throw(ExBasic);
		/** Constructor, calls BasicVersion::parseVersion( str ) */
		BasicVersion( string str ) throw(ExBasic);

		virtual ~BasicVersion() {
		}

		string &toString() {
			return full;
		}

		/** Compares the split primary numbers of two BasicVersion instances.
		 * If the primary string(s) could not be split, it does a simple std::string.compare() */
		int comparePrimary(const BasicVersion& basic_version);

		/** Comparison operator */
		virtual bool operator< ( const BasicVersion& right );
		virtual bool operator> ( const BasicVersion& right );
		virtual bool operator==( const BasicVersion& right );
		virtual bool operator!=( const BasicVersion& right ) {
			return !(*this == right);
		}

		virtual bool operator>= ( const BasicVersion& right ) {
			return ( (*this>right) || (*this==right) );
		}

		virtual bool operator<= ( const BasicVersion& right ) {
			return ( (*this<right) || (*this==right) );
		}

		friend ostream& operator<< (ostream& os, BasicVersion& e) {
			os << e.toString(); 
			return os;
		}
};


#endif /* __VERSIONS_H__ */
