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

#include <config.h>  // IWYU pragma: keep

#include <string>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/regexp.h"
#include "eixTk/unordered_map.h"
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

		virtual bool can_simplify() const {
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
class RegexAlgorithm FINAL : public BaseAlgorithm {
	protected:
		Regex re;

		bool can_simplify() const OVERRIDE {
			return false;
		}

	public:
		void setString(const std::string& s) OVERRIDE {
			search_string = s;
			re.compile(search_string.c_str(), REG_ICASE);
		}

		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package * /* p */) const OVERRIDE {
			return re.match(s);
		}
};

/**
Use case sensitive regex to test strings for a match.
**/
class RegexCaseAlgorithm FINAL : public BaseAlgorithm {
	protected:
		Regex re;

		bool can_simplify() const OVERRIDE {
			return false;
		}

	public:
		void setString(const std::string& s) OVERRIDE {
			search_string = s;
			re.compile(search_string.c_str(), 0);
		}

		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package * /* p */) const OVERRIDE {
			return re.match(s);
		}
};

/**
exact string matching
**/
class ExactAlgorithm FINAL : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) ATTRIBUTE_PURE bool operator()(const char *s, Package * /* p */) const OVERRIDE;
};

/**
substring matching
**/
class SubstringAlgorithm FINAL : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package * /* p */) const OVERRIDE {
			return (std::string(s).find(search_string) != std::string::npos);
		}
};

/**
begin-of-string matching
**/
class BeginAlgorithm FINAL : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) ATTRIBUTE_PURE bool operator()(const char *s, Package * /* p */) const OVERRIDE;
};

/**
end-of-string matching
**/
class EndAlgorithm FINAL : public BaseAlgorithm {
	public:
		ATTRIBUTE_NONNULL((2)) ATTRIBUTE_PURE bool operator()(const char *s, Package * /* p */) const OVERRIDE;
};

/**
Store distance to searchstring in Package and sort out packages with a
higher distance than max_levenshteindistance.
**/
class FuzzyAlgorithm FINAL : public BaseAlgorithm {
	protected:
		Levenshtein max_levenshteindistance;

		/**
		FIXME: We need to have a package->levenshtein mapping that we can
		access from the static FuzzyAlgorithm::compare.
		I really don't know how to do this ..
		**/
		typedef UNORDERED_MAP<std::string, Levenshtein> LevenshteinMap;
		static LevenshteinMap *levenshtein_map;

		bool can_simplify() const OVERRIDE {
			return false;
		}

	public:
		explicit FuzzyAlgorithm(Levenshtein max) : max_levenshteindistance(max) {
		}

		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package *p) const OVERRIDE;

		ATTRIBUTE_NONNULL_ static bool compare(Package *p1, Package *p2);

		static bool sort_by_levenshtein() {
			return (!levenshtein_map->empty());
		}

		static void init_static();
};

/**
Use fnmatch to test if the package matches.
**/
class PatternAlgorithm FINAL : public BaseAlgorithm {
	protected:
		bool can_simplify() const OVERRIDE {
			return false;
		}

	public:
		ATTRIBUTE_NONNULL((2)) bool operator()(const char *s, Package * /* p */) const OVERRIDE;
};

#endif  // SRC_SEARCH_ALGORITHMS_H_
