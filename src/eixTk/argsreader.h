// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __ARGSREADER_H__
#define __ARGSREADER_H__

#include <list>

typedef std::pair<const char *,const char *> ArgPair;

/// Maps longopt->shortopt.
class Option
{
	public:
	enum Type {
		BOOLEAN_T,     ///< Boolean. Will be set to true if found.
		BOOLEAN_F,     ///< Boolean. Will be set to false if found.
		BOOLEAN,       ///< Boolean. Will be flipped if found.
		INTEGER,       ///< Int. Increase value if found.
		STRING,        ///< String. Barf if not found.
		STRING_OPTIONAL,///< String. Empty if not found.
		PAIR,          ///< Pair of strings. Barf if not found.
		PAIR_OPTIONAL, ///< Pair of strings. Empty if not found.
		STRINGLIST,    ///< Accumulative strings.
		STRINGLIST_OPTIONAL,///< Accumulative strings. Empty if not found.
		PAIRLIST,      ///< Accumulative pairs of strings.
		PAIRLIST_OPTIONAL,///< Accumulative pairs of strings.
		KEEP,          ///< Do not remove. No arg.
		KEEP_STRING,   ///< Do not remove. String arg.
		KEEP_STRING_OPTIONAL,///< Do not remove. String arg.
		KEEP_PAIR,     ///< Do not remove. Pair of strings arg.
		KEEP_PAIR_OPTIONAL ///< Do not remove. Pair of strings arg.
	} type;

	Option(const char *l, int s, enum Type t, int *i)
		: type(t), longopt(l), shortopt(s)
	{ u.integer = i; }

	Option(const char *l, int s, enum Type t, bool *b)
		: type(t), longopt(l), shortopt(s)
	{ u.boolean = b; }

	Option(const char *l, int s, enum Type t, const char **c)
		: type(t), longopt(l), shortopt(s)
	{ u.str = c; }

	Option(const char *l, int s, enum Type t, const char **c1, const char **c2)
		: type(t), longopt(l), shortopt(s)
	{ u.pr.first = c1; u.pr.second = c2; }

	Option(const char *l, int s, enum Type t, std::list<const char*> *c)
		: type(t), longopt(l), shortopt(s)
	{ u.strlist = c; }

	Option(const char *l, int s, enum Type t, std::list<ArgPair> *c)
		: type(t), longopt(l), shortopt(s)
	{ u.prlist = c; }

	Option(const char *l, int s, enum Type t = KEEP)
		: type(t), longopt(l), shortopt(s)
	{ }

	const char *longopt; ///< longopt of this pair.
	int  shortopt;       ///< shortopt of this pair.

	union { ///< Pointer to variable of argument.
		int   *integer;
		bool  *boolean;
		const char **str;
		struct { const char **first; const char **second; } pr;
		std::list<const char *> *strlist;
		std::list<ArgPair> *prlist;
	} u;
};

/// Represents a parameter.
class Parameter
{
	public:
		/// Type of argument. ARGUMENT, OPTION, or PAIR.
		const enum Type {
			ARGUMENT = 1,
			OPTION = 2,
			PAIR = 3
		} type;

		/// This is the string we got.
		const char *m_argument;
		/// If type is OPTION this holds the option-key.
		int m_option;

		Parameter(int option)
			: type(Parameter::OPTION), m_option(option)
		{ }

		Parameter(const char *argument)
			: type(Parameter::ARGUMENT), m_argument(argument)
		{ }

		int operator * ()
		{ return type == OPTION ? m_option : -1; }
};

/// Main class for argument parsing.
class ArgumentReader
	: public std::list<Parameter>
{
	public:
		char *name; ///< Name of called program.

		/// Reads arguments into std::list of TParameters.
		ArgumentReader(int argc, char **argv, struct Option opt_table[]);

	private:
		/// Return shortopt for longopt stored in opt.
		// @return shortopt
		static int lookup_longopt(const char *long_opt, struct Option *opt_table);

		/// Check if short_opt is a known option.
		// @return shortopt
		static int lookup_shortopt(const char short_opt, struct Option *opt_table);

		/// Return Option from internal table.
		static Option *lookup_option(const int opt, struct Option *opt_table);

		/// Return number of args for opt
		static unsigned int numargs(const int opt, struct Option *opt_table);

		/// Fold parameter-list so that a option with an arguments has its argument set
		// internal rather than lying around after it in the list.
		// Options which are booleans and integers will be removed and their
		// values increased, flipped, set true or whatever.
		void foldAndRemove(struct Option *opt_table);
};

#endif /* __ARGSREADER_H__ */
