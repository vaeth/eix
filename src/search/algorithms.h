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

/* FNM_CASEFOLD is a gnu extension .. */
#if !defined _GNU_SOURCE
#define _GNU_SOURCE
#endif /* !defined _GNU_SOURCE */

#include <fnmatch.h>

#include <eixTk/levenshtein.h>
#include <eixTk/regexp.h>

#include <portage/package.h>

/* Check if we have FNM_CASEFOLD ..
 * fnmatch(3) tells me that this is a GNU extensions */
#if defined FNM_CASEFOLD
#define FNMATCH_FLAGS FNM_CASEFOLD
#else
#define FNMATCH_FLAGS 0
#define FNMATCH_FLAGS FNM_CASEFOLD
#endif /* defined FNM_CASEFOLD */

#define UNUSED(p) ((void)(p))

/** That's how every Algorithm will look like. */
class BaseAlgorithm {

	protected:
		std::string search_string;

	public:
		virtual void setString(std::string s) {
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
		void setString(std::string s) {
			search_string = s;
			re.compile(search_string.c_str());
		}

		bool operator () (const char *s, Package *p) {
			UNUSED(s); UNUSED(p);
			return !regexec(re.get(), s, 0, NULL, 0);
		}
};

/** Exact-string-matching, use strcmp to test for a match. */
class ESMAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return !strcmp(search_string.c_str(), s);
		}
};

/** Store distance to searchstring in Package and sort out packages with a
 * higher distance than max_levenshteindistance. */
class FuzzyAlgorithm : public BaseAlgorithm {

	protected:
		int max_levenshteindistance;

		/** FIXME: We need to have a package->levenshtein mapping that we can
		 * access from the static FuzzyAlgorithm::compare.
		 * I really don't know how to do this .. */
		static std::map<std::string, int> levenshtein_map;

	public:
		FuzzyAlgorithm(int max) {
			max_levenshteindistance = max;
		}

		bool operator () (const char *s, Package *p) {
			int  d  = get_levenshtein_distance(search_string.c_str(), s);
			bool ok = (d <= max_levenshteindistance);
			if(ok)
			{
				levenshtein_map[p->category + "/" + p->name] = d;
			}
			return ok;
		}

		static bool compare(Package *p1, Package *p2)  {
			return (levenshtein_map[p1->category + "/" + p1->name]
					< levenshtein_map[p2->category + "/" + p2->name]);
		}

		static bool sort_by_levenshtein() {
			return levenshtein_map.size() > 0;
		}
};

/** Use fnmatch to test if the package matches. */
class WildcardAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return !fnmatch(search_string.c_str(), (char *)s, FNM_CASEFOLD);
		}
};

#endif /* __ALGORITHMS_H__ */
