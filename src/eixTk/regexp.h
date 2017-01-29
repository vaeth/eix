// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#ifndef SRC_EIXTK_REGEXP_H_
#define SRC_EIXTK_REGEXP_H_ 1

#include <regex.h>

// include <cstdlib> make check_includes happy

#include <string>
#include <vector>

#include "eixTk/dialect.h"

/**
Handle regular expressions.
It is normally used within global scope so that a regular expression doesn't
have to be compiled with every instance of a class using it.
**/

class Regex {
	private:
		/**
		Forbid copy and assignment operations
		**/
		Regex(const Regex& s) ASSIGN_DELETE;
		Regex& operator=(const Regex& s) ASSIGN_DELETE;

	public:
		/**
		Initalize class
		**/
		Regex() : m_compiled(false) {
		}

		/**
		Initalize and compile regular expression
		**/
		Regex(const char *regex, int eflags) : m_compiled(false) {
			compile(regex, eflags);
		}

		/**
		Initalize and compile regular expression
		**/
		explicit Regex(const char *regex) : m_compiled(false) {
			compile(regex, REG_EXTENDED);
		}

		/**
		Free the regular expression
		**/
		void free();

		~Regex() {
			free();
		}

		/**
		Compile a regular expression
		**/
		void compile(const char *regex, int eflags);
		void compile(const char *regex) {
			compile(regex, REG_EXTENDED);
		}

		/**
		@arg s string to match.
		@return true if the regular expression matches
		**/
		ATTRIBUTE_NONNULL_ bool match(const char *s) const;

		/**
		@arg s string to match
		@arg b beginning of match
		@arg e end of match
		@return true if the regular expression matches
		**/
		ATTRIBUTE_NONNULL((2)) bool match(const char *s, std::string::size_type *b, std::string::size_type *e) const;

		bool compiled() const {
			return m_compiled;
		}

	protected:
		/**
		Get the internal regular expression structure
		**/
		const regex_t *get() const {
			return &m_re;
		}

		/**
		The actual regular expression (GNU C Library)
		**/
		regex_t m_re;

		/**
		Is the regex already compiled and nonempty?
		**/
		bool m_compiled;
};

class RegexList {
	public:
		explicit RegexList(const std::string& stringlist);

		~RegexList();

		bool match(const char *str);

	protected:
		std::vector<Regex *> reglist;
};

#endif  // SRC_EIXTK_REGEXP_H_
