// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_SEARCH_ALGORITHMS_H_
#define SRC_SEARCH_ALGORITHMS_H_ 1

#include <map>
#include <string>

#include "eixTk/regexp.h"
#include "eixTk/unused.h"
#include "search/levenshtein.h"

class Package;
class matchtree;

/**
That's how every algorithm will look like.
**/
class BaseAlgorithm {
		friend class matchtree;

	protected:
		std::string search_string;
		bool have_simplified;

		virtual bool can_simplify() {
			return true;
		}

	public:
		virtual void setString(const std::string& s) {
			search_string = s;
			have_simplified = false;
		}

		virtual ~BaseAlgorithm() {
		}

		ATTRIBUTE_NONNULL((2)) virtual bool operator()(const char *s, Package *p) const = 0;

		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package *p, bool simplify);
};

/**
Use regex to test strings for a match.
**/
class RegexAlgorithm : public BaseAlgorithm {
	protected:
		Regex re;

		bool can_simplify() {
			return false;
		}

	public:
		void setString(const std::string& s) {
			search_string = s;
			re.compile(search_string.c_str(), REG_ICASE);
		}

		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, ATTRIBUTE_UNUSED Package *p) const {
			UNUSED(p);
			return re.match(s);
		}
};

/**
exact string matching
**/
class ExactAlgorithm : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) ATTRIBUTE_PURE bool operator()(const char *s, ATTRIBUTE_UNUSED Package *p) const;
};

/**
substring matching
**/
class SubstringAlgorithm : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, ATTRIBUTE_UNUSED Package *p) const {
			UNUSED(p);
			return (std::string(s).find(search_string) != std::string::npos);
		}
};

/**
begin-of-string matching
**/
class BeginAlgorithm : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) ATTRIBUTE_PURE bool operator()(const char *s, ATTRIBUTE_UNUSED Package *p) const;
};

/**
end-of-string matching
**/
class EndAlgorithm : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) ATTRIBUTE_PURE bool operator()(const char *s, ATTRIBUTE_UNUSED Package *p) const;
};

/**
Store distance to searchstring in Package and sort out packages with a
higher distance than max_levenshteindistance.
**/
class FuzzyAlgorithm : public BaseAlgorithm {
	protected:
		Levenshtein max_levenshteindistance;

		/**
		FIXME: We need to have a package->levenshtein mapping that we can
		access from the static FuzzyAlgorithm::compare.
		I really don't know how to do this ..
		**/
		static std::map<std::string, Levenshtein> *levenshtein_map;

		bool can_simplify() {
			return false;
		}

	public:
		explicit FuzzyAlgorithm(Levenshtein max) : max_levenshteindistance(max) {
		}

		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package *p) const;

		ATTRIBUTE_NONNULL_ static bool compare(Package *p1, Package *p2);

		static bool sort_by_levenshtein() {
			return (!levenshtein_map->empty());
		}

		static void init_static();
};

/**
Use fnmatch to test if the package matches.
**/
class PatternAlgorithm : public BaseAlgorithm {
	protected:
		bool can_simplify() {
			return false;
		}

	public:
		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, ATTRIBUTE_UNUSED Package *p) const;
};

#endif  // SRC_SEARCH_ALGORITHMS_H_
