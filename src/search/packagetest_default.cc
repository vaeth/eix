// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "search/packagetest.h"
#include <config.h>

#include <cstdlib>

#include <string>
#include <vector>

#include "eixTk/assert.h"
#include "eixTk/dialect.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/regexp.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/unordered_map.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "search/algorithms.h"
#include "search/nowarn.h"

using std::string;
using std::vector;

static void init_match_field_map();
static void init_match_algorithm_map();

typedef UNORDERED_MAP<string, PackageTest::MatchField> MatchFieldMap;
typedef UNORDERED_MAP<string, PackageTest::MatchAlgorithm> MatchAlgorithmMap;

static MatchFieldMap *static_match_field_map = NULLPTR;
static MatchAlgorithmMap *static_match_algorithm_map = NULLPTR;

static void init_match_field_map() {
	eix_assert_static(static_match_field_map == NULLPTR);
	static_match_field_map = new MatchFieldMap;
	MatchFieldMap& match_field_map(*static_match_field_map);
	match_field_map["NAME"]           = PackageTest::NAME;
	match_field_map["name"]           = PackageTest::NAME;
	match_field_map["DESCRIPTION"]    = PackageTest::DESCRIPTION;
	match_field_map["description"]    = PackageTest::DESCRIPTION;
	match_field_map["LICENSE"]        = PackageTest::LICENSE;
	match_field_map["license"]        = PackageTest::LICENSE;
	match_field_map["CATEGORY"]       = PackageTest::CATEGORY;
	match_field_map["category"]       = PackageTest::CATEGORY;
	match_field_map["CATEGORY_NAME"]  = PackageTest::CATEGORY_NAME;
	match_field_map["CATEGORY-NAME"]  = PackageTest::CATEGORY_NAME;
	match_field_map["CATEGORY/NAME"]  = PackageTest::CATEGORY_NAME;
	match_field_map["category_name"]  = PackageTest::CATEGORY_NAME;
	match_field_map["category-name"]  = PackageTest::CATEGORY_NAME;
	match_field_map["category/name"]  = PackageTest::CATEGORY_NAME;
	match_field_map["HOMEPAGE"]       = PackageTest::HOMEPAGE;
	match_field_map["homepage"]       = PackageTest::HOMEPAGE;
	match_field_map["IUSE"]           = PackageTest::IUSE;
	match_field_map["USE"]            = PackageTest::IUSE;
	match_field_map["iuse"]           = PackageTest::IUSE;
	match_field_map["use"]            = PackageTest::IUSE;
	match_field_map["WITH_USE"]              = PackageTest::USE_ENABLED;
	match_field_map["WITH-USE"]              = PackageTest::USE_ENABLED;
	match_field_map["INSTALLED_WITH_USE"]    = PackageTest::USE_ENABLED;
	match_field_map["INSTALLED-WITH-USE"]    = PackageTest::USE_ENABLED;
	match_field_map["with_use"]              = PackageTest::USE_ENABLED;
	match_field_map["with-use"]              = PackageTest::USE_ENABLED;
	match_field_map["installed_with_use"]    = PackageTest::USE_ENABLED;
	match_field_map["installed-with-use"]    = PackageTest::USE_ENABLED;
	match_field_map["WITHOUT_USE"]           = PackageTest::USE_DISABLED;
	match_field_map["WITHOUT-USE"]           = PackageTest::USE_DISABLED;
	match_field_map["INSTALLED_WITHOUT_USE"] = PackageTest::USE_DISABLED;
	match_field_map["INSTALLED-WITHOUT-USE"] = PackageTest::USE_DISABLED;
	match_field_map["without_use"]           = PackageTest::USE_DISABLED;
	match_field_map["without-use"]           = PackageTest::USE_DISABLED;
	match_field_map["installed_without_use"] = PackageTest::USE_DISABLED;
	match_field_map["installed-without-use"] = PackageTest::USE_DISABLED;
	match_field_map["SET"]            = PackageTest::SET;
	match_field_map["set"]            = PackageTest::SET;
	match_field_map["EAPI"]           = PackageTest::EAPI;
	match_field_map["eapi"]           = PackageTest::EAPI;
	match_field_map["INSTALLED_EAPI"] = PackageTest::INST_EAPI;
	match_field_map["INSTALLED-EAPI"] = PackageTest::INST_EAPI;
	match_field_map["INSTALLEDEAPI"]  = PackageTest::INST_EAPI;
	match_field_map["installed_eapi"] = PackageTest::INST_EAPI;
	match_field_map["installed-eapi"] = PackageTest::INST_EAPI;
	match_field_map["instelledeapi"]  = PackageTest::INST_EAPI;
	match_field_map["SLOT"]           = PackageTest::SLOT;
	match_field_map["slot"]           = PackageTest::SLOT;
	match_field_map["FULLSLOT"]       = PackageTest::FULLSLOT;
	match_field_map["fullslot"]       = PackageTest::FULLSLOT;
	match_field_map["INSTALLED_SLOT"] = PackageTest::INST_SLOT;
	match_field_map["INSTALLED-SLOT"] = PackageTest::INST_SLOT;
	match_field_map["INSTALLEDSLOT"]  = PackageTest::INST_SLOT;
	match_field_map["installed_slot"] = PackageTest::INST_SLOT;
	match_field_map["installed-slot"] = PackageTest::INST_SLOT;
	match_field_map["installedslot"]  = PackageTest::INST_SLOT;
	match_field_map["INSTALLED_FULLSLOT"] = PackageTest::INST_FULLSLOT;
	match_field_map["INSTALLED-FULLSLOT"] = PackageTest::INST_FULLSLOT;
	match_field_map["INSTALLEDFULLSLOT"]  = PackageTest::INST_FULLSLOT;
	match_field_map["installed_fullslot"] = PackageTest::INST_FULLSLOT;
	match_field_map["installed-fullslot"] = PackageTest::INST_FULLSLOT;
	match_field_map["installedfullslot"]  = PackageTest::INST_FULLSLOT;
	match_field_map["DEP"]            = PackageTest::DEPS;
	match_field_map["DEPS"]           = PackageTest::DEPS;
	match_field_map["DEPENDENCIES"]   = PackageTest::DEPS;
	match_field_map["dep"]            = PackageTest::DEPS;
	match_field_map["deps"]           = PackageTest::DEPS;
	match_field_map["dependencies"]   = PackageTest::DEPS;
	match_field_map["DEPEND"]         = PackageTest::DEPEND;
	match_field_map["depend"]         = PackageTest::DEPEND;
	match_field_map["RDEPEND"]        = PackageTest::RDEPEND;
	match_field_map["rdepend"]        = PackageTest::RDEPEND;
	match_field_map["PDEPEND"]        = PackageTest::PDEPEND;
	match_field_map["pdepend"]        = PackageTest::PDEPEND;
	match_field_map["HDEPEND"]        = PackageTest::HDEPEND;
	match_field_map["hdepend"]        = PackageTest::HDEPEND;
	match_field_map["ERROR"]          = PackageTest::NONE;
	match_field_map["error"]          = PackageTest::NONE;
}

PackageTest::MatchField PackageTest::name2field(const string& p, bool default_match_field) {
	eix_assert_static(static_match_field_map != NULLPTR);
	MatchFieldMap::const_iterator it(static_match_field_map->find(p));
	if(unlikely(it == static_match_field_map->end())) {
		if(default_match_field) {
			eix::say_error(_("unknown match field \"%s\" in DEFAULT_MATCH_FIELD")) % p;
			return NAME;
		}
		eix::say_error(_("unknown match field \"%s\" in DEFAULT_MATCH_ALGORITHM")) % p;
		return NONE;
	}
	MatchField result(it->second);
	if(unlikely((!default_match_field) && (result == NONE))) {
		eix::say_error(_("match field \"error\" is invalid in DEFAULT_MATCH_ALGORITHM"));
	}
	return result;
}

class MatcherField {
	private:
		typedef vector<Regex *> MatchType;
		MatchType match;
		typedef vector<PackageTest::MatchField> FieldsType;
		FieldsType fields;
		PackageTest::MatchField default_value;

	public:
		explicit MatcherField(const string& s) {
			WordVec pairs;
			split_string(&pairs, s, true);
			for(WordVec::iterator it(pairs.begin());
				likely(it != pairs.end()); ++it) {
				string& regex = *it;
				++it;
				if(unlikely(it == pairs.end())) {
					default_value = PackageTest::name2field(regex, true);
					return;
				}
				match.PUSH_BACK(new Regex(regex.c_str()));
				fields.PUSH_BACK(PackageTest::name2field(*it, true));
			}
			default_value = PackageTest::NAME;
		}

		~MatcherField() {
			for(MatchType::iterator it(match.begin()); likely(it != match.end()); ++it) {
				delete *it;
			}
		}

		PackageTest::MatchField parse(const char *p) {
			for(MatchType::size_type i(0); likely(i != match.size()); ++i) {
				if(match[i]->match(p)) {
					return fields[i];
				}
			}
			return default_value;
		}
};

PackageTest::MatchField PackageTest::get_matchfield(const char *p) {
	static MatcherField *m = NULLPTR;
	if(m == NULLPTR) {
		EixRc& rc(get_eixrc());
		m = new MatcherField(rc["DEFAULT_MATCH_FIELD"]);
	}
	return m->parse(p);
}

static void init_match_algorithm_map() {
	eix_assert_static(static_match_algorithm_map == NULLPTR);
	static_match_algorithm_map = new MatchAlgorithmMap;
	MatchAlgorithmMap& match_algorithm_map(*static_match_algorithm_map);
	match_algorithm_map["REGEX"]      = PackageTest::ALGO_REGEX;
	match_algorithm_map["REGEXP"]     = PackageTest::ALGO_REGEX;
	match_algorithm_map["regex"]      = PackageTest::ALGO_REGEX;
	match_algorithm_map["regexp"]     = PackageTest::ALGO_REGEX;
	match_algorithm_map["EXACT"]      = PackageTest::ALGO_EXACT;
	match_algorithm_map["exact"]      = PackageTest::ALGO_EXACT;
	match_algorithm_map["BEGIN"]      = PackageTest::ALGO_BEGIN;
	match_algorithm_map["begin"]      = PackageTest::ALGO_BEGIN;
	match_algorithm_map["END"]        = PackageTest::ALGO_END;
	match_algorithm_map["end"]        = PackageTest::ALGO_END;
	match_algorithm_map["SUBSTRING"]  = PackageTest::ALGO_SUBSTRING;
	match_algorithm_map["substring"]  = PackageTest::ALGO_SUBSTRING;
	match_algorithm_map["PATTERN"]    = PackageTest::ALGO_PATTERN;
	match_algorithm_map["pattern"]    = PackageTest::ALGO_PATTERN;
	match_algorithm_map["FUZZY"]      = PackageTest::ALGO_FUZZY;
	match_algorithm_map["fuzzy"]      = PackageTest::ALGO_FUZZY;
	match_algorithm_map["ERROR"]      = PackageTest::ALGO_ERROR;
	match_algorithm_map["error"]      = PackageTest::ALGO_ERROR;
}

PackageTest::MatchAlgorithm PackageTest::name2algorithm(const string& p) {
	eix_assert_static(static_match_algorithm_map != NULLPTR);
	MatchAlgorithmMap::const_iterator it(static_match_algorithm_map->find(p));
	if(unlikely(it == static_match_algorithm_map->end())) {
		eix::say_error(_("unknown match algorithm \"%s\" in DEFAULT_MATCH_ALGORITHM")) % p;
		return ALGO_REGEX;
	}
	return it->second;
}

void PackageTest::parse_field_specification(const string& spec, MatchField *or_field, MatchField *and_field, MatchField *not_field) {
	if(spec.empty()) {
		return;
	}
	string::size_type i(0);
	for(;;) {
		MatchField *current;
		switch(spec[i]) {
			case '|':
				current = or_field;
				break;
			case '&':
				current = and_field;
				break;
			case '!':
				current = not_field;
				break;
			default:
				eix::say_error(_("missing '|', '&', or '!' in \"%s\" in DEFAULT_MATCH_ALGORITHM")) % spec;
				continue;
		}
		++i;
		string::size_type next(spec.find_first_of("|&!", i));
		if(next == string::npos) {
			*current |= name2field(spec.substr(i), false);
			return;
		}
		*current |= name2field(spec.substr(i, next - i), false);
		i = next;
	}
}

class MatcherAlgorithm {
	private:
		typedef vector<Regex *> MatchType;
		MatchType match;
		typedef vector<PackageTest::MatchAlgorithm> AlgorithmType;
		AlgorithmType algorithms;
		typedef vector<PackageTest::MatchField> FieldType;
		FieldType or_list, and_list, not_list;
		PackageTest::MatchAlgorithm default_value;

	public:
		explicit MatcherAlgorithm(const string& s) {
			WordVec pairs;
			split_string(&pairs, s, true);
			for(WordVec::iterator it(pairs.begin());
				likely(it != pairs.end()); ++it) {
				string& expression = *it;
				++it;
				if(it == pairs.end()) {
					default_value = PackageTest::name2algorithm(expression);
					return;
				}
				PackageTest::MatchField
					or_field(PackageTest::NONE),
					and_field(PackageTest::NONE),
					not_field(PackageTest::NONE);
				const char *regex(expression.c_str());
				if(expression[0] == '(') {
					string::size_type closing(expression.find(')', 1));
					if(likely(closing != string::npos)) {
						regex = expression.c_str() + closing + 1;
						PackageTest::parse_field_specification(
							expression.substr(1, closing - 1),
							&or_field, &and_field, &not_field);
					} else {
						eix::say_error(_("closing ')' missing in \"%s\" in DEFAULT_MATCH_ALGORITHM")) % expression;
					}
				}
				or_list.PUSH_BACK(or_field);
				and_list.PUSH_BACK(and_field);
				not_list.PUSH_BACK(not_field);
				match.PUSH_BACK(new Regex(regex));
				algorithms.PUSH_BACK(PackageTest::name2algorithm(*it));
			}
			default_value = PackageTest::ALGO_REGEX;
		}

		~MatcherAlgorithm() {
			for(MatchType::iterator it(match.begin()); likely(it != match.end()); ++it) {
				delete *it;
			}
		}

		PackageTest::MatchAlgorithm parse(const char *p, PackageTest::MatchField field) {
			for(MatchType::size_type i(0); likely(i != match.size()); ++i) {
				PackageTest::MatchField curr(or_list[i]);
				if((curr != PackageTest::NONE) &&
					((curr & field) == PackageTest::NONE)) {
					continue;
				}
				curr = and_list[i];
				if((curr & field) != curr) {
					continue;
				}
				curr = not_list[i];
				if((curr != PackageTest::NONE) &&
					((field & ~curr) != PackageTest::NONE)) {
					continue;
				}
				if(match[i]->match(p)) {
					return algorithms[i];
				}
			}
			return default_value;
		}
};

PackageTest::MatchAlgorithm PackageTest::get_matchalgorithm(const char *p, PackageTest::MatchField field) {
	static MatcherAlgorithm *m = NULLPTR;
	if(m == NULLPTR) {
		EixRc& rc(get_eixrc());
		m = new MatcherAlgorithm(rc["DEFAULT_MATCH_ALGORITHM"]);
	}
	return m->parse(p, field);
}

void PackageTest::setAlgorithm(BaseAlgorithm *p) {
	delete algorithm;
	algorithm = p;
}

void PackageTest::setAlgorithm(MatchAlgorithm a) {
	switch(a) {
		case ALGO_REGEX:
			setAlgorithm(new RegexAlgorithm());
			break;
		case ALGO_EXACT:
			setAlgorithm(new ExactAlgorithm());
			break;
		case ALGO_BEGIN:
			setAlgorithm(new BeginAlgorithm());
			break;
		case ALGO_END:
			setAlgorithm(new EndAlgorithm());
			break;
		case ALGO_SUBSTRING:
			setAlgorithm(new SubstringAlgorithm());
			break;
		case ALGO_PATTERN:
			setAlgorithm(new PatternAlgorithm());
			break;
		// case ALGO_FUZZY:
		default:
			setAlgorithm(new FuzzyAlgorithm(get_eixrc().getInteger("LEVENSHTEIN_DISTANCE")));
			break;
	}
}

void PackageTest::setPattern(const char *p) {
	if(!know_pattern) {
		if(field == NONE) {
			field = get_matchfield(p);
			if(unlikely(field == NONE)) {
				eix::say_error(
					_("cannot autodetect match field for search string \"%s\"\n"
					"Specify some explicitly, e.g. --any, --name, --category, --slot, etc."))
					% p;
				std::exit(EXIT_FAILURE);
			}
		}
		if(algorithm == NULLPTR) {
			MatchAlgorithm algo(get_matchalgorithm(p, field));
			if(unlikely(algo == ALGO_ERROR)) {
				eix::say_error(
					_("cannot autodetect match algorithm for search string \"%s\"\n"
					"Specify some explicitly, e.g. --regex, --pattern, --exact, etc."))
					% p;
				std::exit(EXIT_FAILURE);
			}
			setAlgorithm(get_matchalgorithm(p, field));
		}
		know_pattern = true;
	}
	algorithm->setString(p);
}

void PackageTest::init_static() {
	NowarnMask::init_static();
	FuzzyAlgorithm::init_static();
	init_match_field_map();
	init_match_algorithm_map();
}
