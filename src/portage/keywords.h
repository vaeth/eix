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

#ifndef __STABILITY_H__
#define __STABILITY_H__

#include <eixTk/stringutils.h>
#include <vector>

using namespace std;

class Keywords {
	public:
		typedef char Type;
		static const Type
			KEY_MISSINGKEYWORD,
			KEY_STABLE        , /*  ARCH */
			KEY_UNSTABLE      , /* ~ARCH */
			KEY_MINUSASTERISK , /*  -*   */
			KEY_MINUSKEYWORD  , /* -ARCH */
			KEY_ALL           ,
			PACKAGE_MASK      ,
			PROFILE_MASK      ,
			SYSTEM_PACKAGE    ;

	protected:
		Type _mask;

	public:
		Keywords(Type t = KEY_MINUSKEYWORD) {
			_mask = t;
		}

		static Type get_type(string arch, string keywords) {
			Type mask = KEY_MISSINGKEYWORD;
			vector<string> arr_keywords = split_string(keywords);
			for(vector<string>::iterator it = arr_keywords.begin(); it != arr_keywords.end(); ++it) {
				switch((*it)[0]) {
					case '~':
						if(it->substr(1) == arch) {
							mask |= KEY_UNSTABLE;
						}
						break;
					case '-':
						if(it->substr(1) == "*") {
							mask |= KEY_MINUSASTERISK;
						}
						else if(it->substr(1) == arch) {
							mask |= KEY_MINUSKEYWORD;
						}
						break;
					default:
						if(!(mask & KEY_STABLE) && *it == arch) {
							mask |= KEY_STABLE;
						}
						break;
				}
			}
			return mask;
		}

		void set(string arch, string keywords) {
			set(get_type(arch, keywords));
		}
			
		void set(Type t) {
			_mask = t;
		}

		Type get() {
			return _mask;
		}

		/** Return true if version is marked stable. */
		bool isStable()               { return _mask & KEY_STABLE; }
		/** Return true if version is unstable. */
		bool isUnstable()             { return _mask & KEY_UNSTABLE; }
		/** Return true if version is masked by -* keyword. */
		bool isMinusAsterisk()        { return _mask & KEY_MINUSASTERISK; }
		/** Return true if version is masked by -keyword. */
		bool isMinusKeyword()         { return _mask & KEY_MINUSKEYWORD; }
		/** Return true if version is masked by missing keyword. */
		bool isMissingKeyword()       { return _mask == KEY_MISSINGKEYWORD; }

		bool isHardMasked()  { return isPackageMask() || isProfileMask(); }
		/** Return true if version is masked by profile. */
		bool isProfileMask() { return _mask & PROFILE_MASK; }
		/** Return true if version is masked by a package.mask. */
		bool isPackageMask() { return _mask & PACKAGE_MASK; }
		/** Return true if version is part of a package that is a system-package. */
		bool isSystem()      { return _mask & SYSTEM_PACKAGE; }

		void operator |= (const Keywords::Type &t)     { _mask |= t; }
		void operator &= (const Keywords::Type &t)     { _mask &= t; }

		void print() {
			printf("< %c%c%c%c%c%c%c >\n",
					(_mask & KEY_STABLE) ? '#' : '.',
					(_mask & KEY_UNSTABLE) ? '~' : '.',
					(_mask & KEY_MINUSASTERISK) ? '*' : '.',
					(_mask & KEY_MINUSKEYWORD) ? '-' : '.',
					(_mask & PACKAGE_MASK) ? 'M' : '.',
					(_mask & PROFILE_MASK) ? 'P' : '.',
					(_mask & SYSTEM_PACKAGE) ? 'S' : '.');
		}

#if 0
		string recreateString(string &arch) {
			string ret;
			if(_mask & KEY_STABLE) {
				ret.append(arch);
			}
			if(_mask & KEY_UNSTABLE) {
				ret.append(" ~");
				ret.append(arch);
			}
			if(_mask & KEY_MINUSASTERISK) {
				ret.append(" -*");
			}
			if(_mask & KEY_MINUSKEYWORD) {
				ret.append(" -");
				ret.append(arch);
			}
			return trim(ret);
		}
#endif
};

#if 0
TYPE getStability(vector<string> accepted_keywords, string &keywords);
#endif

#endif /* __STABILITY_H__ */
