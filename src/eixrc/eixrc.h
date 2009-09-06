// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef EIX__EIXRC_H__
#define EIX__EIXRC_H__ 1

#include <portage/keywords.h>
#include <search/redundancy.h>

#include <cstdio>
#include <iostream>

#define EIX_VARS_PREFIX "EIX_"
#define DIFF_VARS_PREFIX "DIFF_"

class EixRcOption {
	public:
		typedef enum { STRING, PREFIXSTRING, INTEGER, BOOLEAN, LOCAL } OptionType;
		OptionType type;
		std::string key, value, local_value, description;

		EixRcOption(OptionType t, std::string name, std::string val, std::string desc);
};

class EixRc {
	public:
		std::string varprefix;
		std::string m_eprefixconf;

		typedef std::vector<EixRcOption>::size_type default_index;
		typedef std::pair<RedAtom, RedAtom> RedPair;

		void read();

		void clear();

		void addDefault(EixRcOption option);

		bool getBool(const std::string &key)
		{ return istrue((*this)[key].c_str()); }

		short getBoolText(const std::string &key, const char *text)
		{
			const char *s = (*this)[key].c_str();
			if(!strcasecmp(s, text))
				return -1;
			if(istrue(s))
				return 1;
			return 0;
		}

		LocalMode getLocalMode(const std::string &key);

		void getRedundantFlags(const std::string &key,
			Keywords::Redundant type,
			RedPair &p);

		unsigned int getInteger(const std::string &key);

		void dumpDefaults(FILE *s, bool use_defaults);

		const char *cstr(const std::string &key) const;

		const char *prefix_cstr(const std::string &key) const;

		void print_var(const std::string &key);

		const std::string &operator[](const std::string &key)
		{
			std::map<std::string,std::string>::const_iterator it = main_map.find(key);
			if(it != main_map.end())
				return it->second;
			add_later_variable(key);
			return main_map[key];
		}
	private:
		std::map<std::string,std::string> main_map;
		std::map<std::string,std::string> filevarmap;

		static bool istrue(const char *s);
		enum DelayedType { DelayedNotFound, DelayedVariable, DelayedIfTrue, DelayedIfFalse, DelayedIfNonempty, DelayedIfEmpty, DelayedElse, DelayedFi, DelayedQuote };
		std::vector<EixRcOption> defaults;
		std::set<std::string> prefix_keys;
		static bool getRedundantFlagAtom(const char *s, Keywords::Redundant type, RedAtom &r);
		void modify_value(std::string &value, const std::string &key);

		/** This will fetch a variable which was not set in the
		    defaults (or its modification or its delayed references),
		    i.e. it must be fetched from the config or ENV setting.
		    Of course, it will be resolved for delayed substitutions,
		    and delayed references are also be added similarly. */
		void add_later_variable(const std::string &key);

		void resolve_delayed(std::string key, std::set<std::string> &has_delayed);
		std::string *resolve_delayed_recurse(std::string key, std::set<std::string> &visited, std::set<std::string> &has_delayed, const char **errtext, std::string *errvar);

		/** Create defaults and main_map with all variables
		   (including all values required by delayed references).
		   @arg has_delayed is initialized to corresponding keys */
		void read_undelayed(std::set<std::string> &has_delayed);
		/** Recursively join key and its delayed references to
		    main_map and default; set has_delayed if appropriate */
		void join_key(const std::string &key, std::set<std::string> &has_delayed, bool add_top_to_defaults, const std::set<std::string> *exclude_defaults);
		void join_key_rec(const std::string &key, const std::string &val, std::set<std::string> &has_delayed, const std::set<std::string> *exclude_defaults);
		void join_key_if_new(const std::string &key, std::set<std::string> &has_delayed, const std::set<std::string> *exclude_defaults)
		{
			if(main_map.find(key) == main_map.end())
				join_key(key, has_delayed, true, exclude_defaults);
		}

		static DelayedType find_next_delayed(const std::string &str, std::string::size_type *pos, std::string::size_type *length = NULL);
		static std::string as_comment(const std::string &s);
};

#endif /* EIX__EIXRC_H__ */
