// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_ARGSREADER_H_
#define SRC_EIXTK_ARGSREADER_H_ 1

#include <utility>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/eixint.h"

typedef std::pair<const char *, const char *> ArgPair;

/**
Map longopt->shortopt
**/
class Option {
	public:
	enum Type {
		BOOLEAN_T,             ///< Boolean. Will be set to true if found.
		BOOLEAN_F,             ///< Boolean. Will be set to false if found.
		BOOLEAN,               ///< Boolean. Will be flipped if found.
		INTEGER,               ///< Int. Increase value if found.
		STRING,                ///< String. Barf if not found.
		STRING_OPTIONAL,       ///< String. Empty if not found.
		PAIR,                  ///< Pair of strings. Barf if not found.
		PAIR_OPTIONAL,         ///< Pair of strings. Empty if not found.
		STRINGLIST,            ///< Accumulative strings.
		STRINGLIST_OPTIONAL,   ///< Accumulative strings. Empty if not found.
		PAIRLIST,              ///< Accumulative pairs of strings.
		PAIRLIST_OPTIONAL,     ///< Accumulative pairs of strings.
		KEEP,                  ///< Do not remove. No arg.
		KEEP_STRING,           ///< Do not remove. String arg.
		KEEP_STRING_OPTIONAL,  ///< Do not remove. String arg.
		KEEP_PAIR,             ///< Do not remove. Pair of strings arg.
		KEEP_PAIR_OPTIONAL     ///< Do not remove. Pair of strings arg.
	} type;

	ATTRIBUTE_NONNULL((2, 5)) Option(const char *l, int s, enum Type t, int *i)
		: type(t), longopt(l), shortopt(s) {
		u.integer = i;
	}

	ATTRIBUTE_NONNULL((2, 5)) Option(const char *l, int s, enum Type t, bool *b)
		: type(t), longopt(l), shortopt(s) {
		u.boolean = b;
	}

	ATTRIBUTE_NONNULL((2, 5)) Option(const char *l, int s, enum Type t, const char *c[])
		: type(t), longopt(l), shortopt(s) {
		u.str = c;
	}

	ATTRIBUTE_NONNULL((2, 5, 6)) Option(const char *l, int s, enum Type t, const char *c1[], const char *c2[])
		: type(t), longopt(l), shortopt(s) {
		u.pr.first = c1;
		u.pr.second = c2;
	}

	ATTRIBUTE_NONNULL((2, 5)) Option(const char *l, int s, enum Type t, std::vector<const char*> *c)
		: type(t), longopt(l), shortopt(s) {
		u.strlist = c;
	}

	ATTRIBUTE_NONNULL((2, 5)) Option(const char *l, int s, enum Type t, std::vector<ArgPair> *c)
		: type(t), longopt(l), shortopt(s) {
		u.prlist = c;
	}

	ATTRIBUTE_NONNULL((2)) Option(const char *l, int s, enum Type t = KEEP)
		: type(t), longopt(l), shortopt(s) {
	}

	const char *longopt;  ///< longopt of this pair.
	int  shortopt;        ///< shortopt of this pair.

	union {  ///< Pointer to variable of argument.
		int   *integer;
		bool  *boolean;
		const char **str;
		struct {
			const char **first;
			const char **second;
		} pr;
		std::vector<const char *> *strlist;
		std::vector<ArgPair> *prlist;
	} u;
};

class OptionList : public std::vector<Option> {
};

/**
Represents a parameter
**/
class Parameter {
	public:
		/**
		Type of argument. ARGUMENT, OPTION, or PAIR
		**/
		enum Type {
			ARGUMENT = 1,
			OPTION = 2,
			PAIR = 3
		} type;

		/**
		This is the string we got.
		**/
		const char *m_argument;

		/**
		If type is OPTION this holds the option-key
		**/
		int m_option;

		explicit Parameter(int option) : type(Parameter::OPTION), m_option(option) {
		}

		ATTRIBUTE_NONNULL((2)) explicit Parameter(const char *argument)
			: type(Parameter::ARGUMENT), m_argument(argument) {
		}

		int operator*() const {
			return (type == OPTION) ? m_option : -1;
		}
};

/**
Main class for argument parsing
**/
class ArgumentReader : public std::vector<Parameter> {
	public:
		/**
		Name of called program
		**/
		const char *name;

		/**
		Read arguments into list of Parameters
		**/
		ATTRIBUTE_NONNULL_ ArgumentReader(int argc, const char *const *argv, const OptionList& opt_table);

	private:
		/**
		@return shortopt for longopt stored in opt
		**/
		ATTRIBUTE_NONNULL_ static int lookup_longopt(const char *long_opt, const OptionList& opt_table);

		/**
		Check if short_opt is a known option.
		@return shortopt
		**/
		static int lookup_shortopt(const char short_opt, const OptionList& opt_table);

		/**
		@return option from internal table
		**/
		ATTRIBUTE_PURE static const Option *lookup_option(const int opt, const OptionList& opt_table);

		/**
		@return number of args for opt
		**/
		ATTRIBUTE_PURE static eix::TinyUnsigned numargs(const int opt, const OptionList& opt_table);

		/**
		Fold parameter-list so that a option with an arguments has its argument set
		internal rather than lying around after it in the list.
		Options which are booleans and integers will be removed and their
		values increased, flipped, set true or whatever.
		**/
		void foldAndRemove(const OptionList& opt_table);
};

#endif  // SRC_EIXTK_ARGSREADER_H_
