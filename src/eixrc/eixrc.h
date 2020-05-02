// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXRC_EIXRC_H_
#define SRC_EIXRC_EIXRC_H_ 1

#include <config.h>  // IWYU pragma: keep

#include <cstdio>

#include <string>
#include <utility>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/inttypes.h"
#include "eixTk/stringtypes.h"
#include "portage/keywords.h"
#include "search/redundancy.h"

#define EIX_VARS_PREFIX "EIX_"
#define DIFF_VARS_PREFIX "DIFF_"
#define UPDATE_VARS_PREFIX "UPDATE_"
#define DROP_VARS_PREFIX "DROP_"

class EixRcOption {
	public:
		typedef enum { STRING, PREFIXSTRING, INTEGER, BOOLEAN, LOCAL } OptionType;
		OptionType type;
		std::string key, value, local_value, description;

		EixRcOption(OptionType t, const char *name, const char *val, const char *desc) :
			type(t), key(name), value(val), description(desc) {
		}

		EixRcOption(const OptionType t, const std::string name, const std::string val, const std::string desc) :
			type(t), key(name), value(val), description(desc) {
		}

		EixRcOption(const std::string name, const std::string val) :
			type(LOCAL), key(name), local_value(val) {
		}
};

class EixRc {
	public:
		std::string m_eprefixconf;

		ATTRIBUTE_NONNULL_ explicit EixRc(const char *prefix) : varprefix(prefix) {
		}

		typedef std::vector<EixRcOption>::size_type default_index;
		typedef std::pair<RedAtom, RedAtom> RedPair;

		void read();

		void clear();

		void addDefault(EixRcOption option);

		bool getBool(const std::string& key) {
			return istrue((*this)[key].c_str());
		}

		ATTRIBUTE_NONNULL_ eix::SignedBool getBoolText(const std::string& key, const char *text);

		ATTRIBUTE_NONNULL_ eix::TinySigned getTinyTextlist(const std::string& key, const char *const *text);

		LocalMode getLocalMode(const std::string& key);

		ATTRIBUTE_NONNULL_ void getRedundantFlags(const std::string& key,
			Keywords::Redundant type,
			RedPair *p);

		unsigned int getInteger(const std::string& key);

		ATTRIBUTE_NONNULL_ void dumpDefaults(FILE *s, bool use_defaults);

		ATTRIBUTE_PURE const char *cstr(const std::string& key) const;

		ATTRIBUTE_PURE const char *prefix_cstr(const std::string& key) const;

		void known_vars();
		bool print_var(const std::string& key);

		const std::string& operator[](const std::string& key);

		ATTRIBUTE_PURE static bool istrue(const char *s);

	private:
		std::string varprefix;
		WordUnorderedMap main_map;
		WordIterateMap filevarmap;
		std::vector<EixRcOption> defaults;
		WordUnorderedSet prefix_keys;

		enum DelayedType { DelayedNotFound, DelayedVariable, DelayedIfTrue, DelayedIfFalse, DelayedIfNonempty, DelayedIfEmpty, DelayedElse, DelayedFi, DelayedQuote };

		ATTRIBUTE_NONNULL((3)) static bool getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom *r);

		void modify_value(std::string *value, const std::string& key);

		/**
		This will fetch a variable which was not set in the
		defaults (or its modification or its delayed references),
		i.e. it must be fetched from the config or ENV setting.
		Of course, it will be resolved for delayed substitutions,
		and delayed references are also be added similarly.
		**/
		void add_later_variable(const std::string& key);

		ATTRIBUTE_NONNULL_ void resolve_delayed(const std::string& key, WordUnorderedSet *has_delayed);
		ATTRIBUTE_NONNULL_ std::string *resolve_delayed_recurse(const std::string& key, WordUnorderedSet *visited, WordUnorderedSet *has_delayed, const char **errtext, std::string *errvar);

		/**
		Create defaults and main_map with all variables
		(including all values required by delayed references).
		@arg has_delayed is initialized to corresponding keys
		**/
		ATTRIBUTE_NONNULL_ void read_undelayed(WordUnorderedSet *has_delayed);
		/**
		Recursively join key and its delayed references to
		main_map and default; set has_delayed if appropriate
		**/
		ATTRIBUTE_NONNULL((3)) void join_key(const std::string& key, WordUnorderedSet *has_delayed, bool add_top_to_defaults, const WordUnorderedSet *exclude_defaults);
		ATTRIBUTE_NONNULL((4)) void join_key_rec(const std::string& key, const std::string& val, WordUnorderedSet *has_delayed, const WordUnorderedSet *exclude_defaults);
		ATTRIBUTE_NONNULL((3)) void join_key_if_new(const std::string& key, WordUnorderedSet *has_delayed, const WordUnorderedSet *exclude_defaults);

		typedef uint8_t DelayvarFlags;
		static CONSTEXPR const DelayvarFlags
			DELAYVAR_NONE   = 0x00,
			DELAYVAR_STAR   = 0x01,
			DELAYVAR_ESCAPE = 0x02,
			DELAYVAR_APPEND = 0x04;

		ATTRIBUTE_NONNULL((2, 3)) static DelayedType find_next_delayed(const std::string& str, std::string::size_type *pos, std::string::size_type *length, std::string *varname, DelayvarFlags *varflags, std::string *append);
		static std::string as_comment(const std::string& s);
};

#endif  // SRC_EIXRC_EIXRC_H_
