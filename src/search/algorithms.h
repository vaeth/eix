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

#ifndef __ALGORITHMS_H__
#define __ALGORITHMS_H__

#include <fnmatch.h>

/** That's how every Algorithm will look like. */
class BaseAlgorithm {

	protected:
		string search_string;

	public:
		virtual void setString(string s) {
			search_string = s;
		}

		virtual ~BaseAlgorithm() {
			// Nothin' to see here, please move along
		}

		virtual bool operator () (const char *s, Package *p) = 0;
};

/** Use regex to test strings for a match. */
class RegexAlgorithm : public BaseAlgorithm {

	protected:
		Regex re;

	public:
		void setString(string s) {
			search_string = s;
			re.compile(search_string.c_str());
		}

		bool operator () (const char *s, Package *p) {
			return !regexec(re.get(), s, 0, NULL, 0);
		}
};

/** Exact-string-matching, use strcmp to test for a match. */
class ESMAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			return !strcmp(search_string.c_str(), s);
		}
};

/** Store distance to searchstring in Package and sort out packages with a
 * higher distance than max_levenshteindistance. */
class FuzzyAlgorithm : public BaseAlgorithm {

	protected:
		int max_levenshteindistance;

	public:
		FuzzyAlgorithm(int max) {
			max_levenshteindistance = max;
		}

		bool operator () (const char *s, Package *p) {
			int  d  = get_levenshtein_distance(search_string.c_str(), s);
			bool ok = (d <= max_levenshteindistance);
			if(ok)
			{
				// FIXME: BAH
			}
			return ok;
		}
		
		void sort(vector<Package*>::iterator &begin, vector<Package*>::iterator &end)  {
#if 0
			algorithm::sort(begin, end, FuzzyAlgorithm::comparator);
#endif
			// FIXME: BAH
		}
};

/** Use fnmatch to test if the package matches. */
class WildcardAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			return !fnmatch(search_string.c_str(), (char *)s, FNM_CASEFOLD);
		}
};

#endif /* __ALGORITHMS_H__ */
