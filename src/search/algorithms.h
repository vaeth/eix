// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__ALGORITHMS_H__
#define EIX__ALGORITHMS_H__ 1

#include <search/levenshtein.h>
#include <eixTk/regexp.h>

#include <portage/package.h>

#include <fnmatch.h>

/* Check if we have FNM_CASEFOLD ..
 * fnmatch(3) tells that this is a GNU extension.
 * However, we do not #define _GNU_SOURCE but instead make sure to
 * #include <config.h> (at least implicitly) */
#ifdef FNM_CASEFOLD
#define FNMATCH_FLAGS FNM_CASEFOLD
#else
#define FNMATCH_FLAGS 0
#endif

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
		RegexAlgorithm()
		{ }

		void setString(std::string s) {
			search_string = s;
			re.compile(search_string.c_str(), REG_ICASE);
		}

		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return re.match(s);
		}
};

/** exact string matching */
class ExactAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return !strcmp(search_string.c_str(), s);
		}
};

/** substring matching */
class SubstringAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return (std::string(s).find(search_string) != std::string::npos);
		}
};

/** begin-of-string matching */
class BeginAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return !strncmp(search_string.c_str(), s, search_string.size());
		}
};

/** end-of-string matching */
class EndAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			size_t l = strlen(s);
			std::string::size_type sl = search_string.size();
			if(l < sl)
				return false;
			return !strcmp(search_string.c_str(), s + (l - sl));
		}
};

/** Store distance to searchstring in Package and sort out packages with a
 * higher distance than max_levenshteindistance. */
class FuzzyAlgorithm : public BaseAlgorithm {

	protected:
		unsigned int max_levenshteindistance;

		/** FIXME: We need to have a package->levenshtein mapping that we can
		 * access from the static FuzzyAlgorithm::compare.
		 * I really don't know how to do this .. */
		static std::map<std::string, unsigned int> levenshtein_map;

	public:
		FuzzyAlgorithm(unsigned int max) {
			max_levenshteindistance = max;
		}

		bool operator () (const char *s, Package *p) {
			unsigned int  d  = get_levenshtein_distance(search_string.c_str(), s);
			bool ok = (d <= max_levenshteindistance);
			if(ok)
			{
				if(p)
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
class PatternAlgorithm : public BaseAlgorithm {

	public:
		bool operator () (const char *s, Package *p) {
			UNUSED(p);
			return !fnmatch(search_string.c_str(), s, FNMATCH_FLAGS);
		}
};

#endif /* EIX__ALGORITHMS_H__ */
