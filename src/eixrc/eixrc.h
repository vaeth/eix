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

#include <cstdio>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/constexpr.h"
#include "eixTk/eixint.h"
#include "eixTk/inttypes.h"
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

		EixRcOption(OptionType t, std::string name, std::string val, std::string desc);
};

class EixRc {
	public:
		std::string m_eprefixconf;

		explicit EixRc(const char *prefix) ATTRIBUTE_NONNULL_ : varprefix(prefix) {
		}

		typedef std::vector<EixRcOption>::size_type default_index;
		typedef std::pair<RedAtom, RedAtom> RedPair;

		void read();

		void clear();

		void addDefault(EixRcOption option);

		bool getBool(const std::string& key) {
			return istrue((*this)[key].c_str());
		}

		eix::SignedBool getBoolText(const std::string& key, const char *text) ATTRIBUTE_NONNULL_;

		eix::TinySigned getTinyTextlist(const std::string& key, const char **text) ATTRIBUTE_NONNULL_;

		LocalMode getLocalMode(const std::string& key);

		void getRedundantFlags(const std::string& key,
			Keywords::Redundant type,
			RedPair *p) ATTRIBUTE_NONNULL_;

		unsigned int getInteger(const std::string& key);

		void dumpDefaults(FILE *s, bool use_defaults) ATTRIBUTE_NONNULL_;

		const char *cstr(const std::string& key) const ATTRIBUTE_PURE;

		const char *prefix_cstr(const std::string& key) const ATTRIBUTE_PURE;

		void known_vars();
		bool print_var(const std::string& key);

		const std::string& operator[](const std::string& key);

	private:
		typedef std::map<std::string, std::string> my_map;
		std::string varprefix;
		my_map main_map;
		my_map filevarmap;
		std::vector<EixRcOption> defaults;
		std::set<std::string> prefix_keys;

		enum DelayedType { DelayedNotFound, DelayedVariable, DelayedIfTrue, DelayedIfFalse, DelayedIfNonempty, DelayedIfEmpty, DelayedElse, DelayedFi, DelayedQuote };

		static bool istrue(const char *s) ATTRIBUTE_PURE;
		static bool getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom *r) ATTRIBUTE_NONNULL((3));

		void modify_value(std::string *value, const std::string& key);

		/** This will fetch a variable which was not set in the
		    defaults (or its modification or its delayed references),
		    i.e. it must be fetched from the config or ENV setting.
		    Of course, it will be resolved for delayed substitutions,
		    and delayed references are also be added similarly. */
		void add_later_variable(const std::string& key);

		void resolve_delayed(const std::string& key, std::set<std::string> *has_delayed) ATTRIBUTE_NONNULL_;
		std::string *resolve_delayed_recurse(const std::string& key, std::set<std::string> *visited, std::set<std::string> *has_delayed, const char **errtext, std::string *errvar) ATTRIBUTE_NONNULL_;

		/** Create defaults and main_map with all variables
		   (including all values required by delayed references).
		   @arg has_delayed is initialized to corresponding keys */
		void read_undelayed(std::set<std::string> *has_delayed) ATTRIBUTE_NONNULL_;
		/** Recursively join key and its delayed references to
		    main_map and default; set has_delayed if appropriate */
		void join_key(const std::string& key, std::set<std::string> *has_delayed, bool add_top_to_defaults, const std::set<std::string> *exclude_defaults) ATTRIBUTE_NONNULL((3));
		void join_key_rec(const std::string& key, const std::string& val, std::set<std::string> *has_delayed, const std::set<std::string> *exclude_defaults) ATTRIBUTE_NONNULL((4));
		void join_key_if_new(const std::string& key, std::set<std::string> *has_delayed, const std::set<std::string> *exclude_defaults) ATTRIBUTE_NONNULL((3));

		typedef uint8_t DelayvarFlags;
		static CONSTEXPR DelayvarFlags
			DELAYVAR_NONE   = 0x00,
			DELAYVAR_STAR   = 0x01,
			DELAYVAR_ESCAPE = 0x02,
			DELAYVAR_APPEND = 0x04;

		static DelayedType find_next_delayed(const std::string& str, std::string::size_type *pos, std::string::size_type *length, std::string *varname, DelayvarFlags *varflags, std::string *append) ATTRIBUTE_NONNULL((2, 3));
		static std::string as_comment(const std::string& s);
};

#endif  // SRC_EIXRC_EIXRC_H_
