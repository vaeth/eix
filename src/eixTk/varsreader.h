// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef SRC_EIXTK_VARSREADER_H_
#define SRC_EIXTK_VARSREADER_H_ 1

#include <sys/types.h>

#include <map>
#include <string>

#include "eixTk/constexpr.h"
#include "eixTk/inttypes.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"

/** A wrapper to FSM that can read shell-style key=value declarations.
 * The constructor inits, starts the FSM. Then you can access them .. The deconstructor deinits it. */
class VarsReader {
	public:
		typedef std::map<std::string, std::string> my_map;
		typedef my_map::const_iterator const_iterator;
		typedef my_map::iterator iterator;
		typedef uint16_t Flags;
		static CONSTEXPR Flags
			NONE                 = 0x0000U,  /**< Flag: No flags set; normal behavior. */
			ONLY_KEYWORDS_SLOT   = 0x0001U,  /**< Flag: Only read "KEYWORDS" and "SLOT" once, then stop the parser. */
			KEYWORDS_READ        = 0x0002U,  /**< Flag: Have already read "KEYWORDS" once. */
			SLOT_READ            = 0x0004U,  /**< Flag: Have already read "SLOT" once. */
			SUBST_VARS           = 0x0008U,  /**< Flag: Allow references to variable in declarations of a variable. i.e.  USE="${USE} -kde" */
			INTO_MAP             = 0x0010U,  /**< Flag: Init but don't parse .. you must first supply a pointer to my_map with useMap (...) */
			APPEND_VALUES        = 0x0020U,  /**< Flag: Respect IncrementalKeys */
			ALLOW_SOURCE         = 0x0040U,  /**< Flag: Allow "source"/"." command. */
			ALLOW_SOURCE_VARNAME = 0x0080U|ALLOW_SOURCE,  /**< Flag: Allow "source"/"." but Prefix is only a varname which might be modified during sourcing. */
			PORTAGE_ESCAPES      = 0x0100U,  /**< Flag: Treat escapes like portage does. */
			PORTAGE_SECTIONS     = 0x0200U,  /**< Flag: Section format of e.g. repos.conf */
			RECURSE              = 0x0400U,  /**< Flag: Allow recursive reading */
			HAVE_READ            = KEYWORDS_READ|SLOT_READ,       /**< Combination of previous "*_READ" */
			ONLY_HAVE_READ       = ONLY_KEYWORDS_SLOT|HAVE_READ;  /**< Combination of HAVE_READ and ONLY_KEYWORDS_SLOT */


		/** Init and parse the FSM depending on supplied flags. */
		explicit VarsReader(Flags flags) {
			if((flags & PORTAGE_SECTIONS) != NONE) {
				flags |= PORTAGE_ESCAPES;
			}
			parse_flags = flags;
			if((parse_flags & INTO_MAP) == NONE) {
				vars = new my_map;
			}
		}

		/** Free memory. */
		~VarsReader() {
			if((parse_flags & INTO_MAP) == NONE)
				delete vars;
		}

		/** Read file.
		 * @return true if the file was successfully read. */
		bool read(const char *filename, std::string *errtext, bool noexist_ok, WordSet *sourced, bool nodir) ATTRIBUTE_NONNULL((2));
		bool read(const char *filename, std::string *errtext, bool noexist_ok, WordSet *sourced) ATTRIBUTE_NONNULL((2)) {
			return read(filename, errtext, noexist_ok, sourced, false);
		}
		bool read(const char *filename, std::string *errtext, bool noexist_ok) ATTRIBUTE_NONNULL_ {
			return read(filename, errtext, noexist_ok, NULLPTR);
		}

		/** Use a supplied map for variables. */
		void useMap(my_map *vars_map) ATTRIBUTE_NONNULL_ {
			vars = vars_map;
		}

		/** Prefix (path resp. varname) used for sourcing */
		void setPrefix(const std::string& prefix) {
			source_prefix = prefix;
		}

		/** Set array of keys which values should be prepended to the new value. */
		void accumulatingKeys(const char **keys) {
			incremental_keys = keys;
		}

		/** Operator that helps us to be used like a map. */
		std::string& operator[](const std::string& key) {
			return (*vars)[key];
		}

		const std::string *find(const std::string& key) const ATTRIBUTE_PURE {
			const_iterator i(vars->find(key));
			if(i == vars->end())
				return NULLPTR;
			return &(i->second);
		}

		VarsReader::const_iterator begin() const {
			return vars->begin();
		}

		VarsReader::const_iterator end() const {
			return vars->end();
		}

	private:
		/** Kepper of the holy state. */
		void (VarsReader::*STATE)();
		bool retstate;

		/** Eats most of the noise between declarations.
		 * Read until '\n' '#' '\\' '\'' '"'
		 * '\\' -> [RV] NOISE_ESCAPE | '\'' -> [RV] NOISE_SINGLE_QUOTE | '"' -> [RV] NOISE_DOUBLE_QUOTE | '#' -> [RV] JUMP_COMMENT
		 * '\n' -> [RV] (and check if we are at EOF, EOF's only occur after a newline) -> JUMP_WHITESPACE */
		void JUMP_NOISE();

		/** Assign key=value or source file and change state to JUMP_NOISE (or STOP
		 *  if an early stop is required due to ONLY_KEYWORDS_SLOT or ONLY_HAVE_READ)
		 * -> [RV] JUMP_NOISE */
		void ASSIGN_KEY_VALUE();

		/** Jumps comments.
		 * Read until the next '\n' comes in. Then move to JUMP_NOISE. */
		void JUMP_COMMENT();

		/** Jump whitespaces and tabs at begining of line.
		 * Read while ' ' || '\t'
		 * Comment line -> JUMP_WHITESPACE | [A-Z_] -> (reset key) FIND_ASSIGNMENT |
		 * source or . -> EVAL_VALUE with sourcecmd=true | -> JUMP_NOISE
		 * '[' -> EVAL_SECTION
		 * @see isValidKeyCharacter */
		void JUMP_WHITESPACE();

		/** Find key .. if there is one :).
		 * Keys are marked with a '=' at the end ('=' is not part of the key). This state always gets the
		 * first character of the keyword as current INPUT, thus we don't need to check if key_len is >= 1,
		 * is always >= 1.
		 * Read while not ('=' || '#') and [A-Z_]
		 * '=' -> EVAL_VALUE | '#' -> [RV] JUMP_COMMENT | -> JUMP_NOISE */
		void FIND_ASSIGNMENT();

		/** Looks if the following input is a valid value-part.
		 * ['"\^#\\n\\t ] is allowed.
		 * [\\n\\t ] -> JUMP_NOISE | '#' -> [RV] JUMP_COMMENT
		 * '\'' -> [RV] VALUE_SINGLE_QUOTE{_PORTAGE}
		 * '"' -> [RV] VALUE_DOUBLE_QUOTE{_PORTAGE}
		 * '\\' -> [RV] WHITESPACE_ESCAPE{_PORTAGE} | -> VALUE_WHITESPACE{_PORTAGE}
		 * We are resetting the value-buffer. */
		void EVAL_VALUE();

		/** Looks if the following input is a valid secondary value-part.
		 * ['"\^#\n\r\t ] is allowed.
		 * [\n\r\t# ] (or [\n\r\t;|&)# ]) -> ASSIGN_KEY_VALUE
		 * '\'' -> [RV] VALUE_SINGLE_QUOTE{_PORTAGE}
		 * '"' -> [RV] VALUE_DOUBLE_QUOTE{_PORTAGE}
		 * '\\' -> [RV] WHITESPACE_ESCAPE{_PORTAGE} | -> VALUE_WHITESPACE{_PORTAGE} */
		void EVAL_READ();

		/** Reads a value enclosed in single quotes (').
		 * Copy INPUT into value-buffer while INPUT is not in ['\\].
		 * If the value ends, ASSIGN_KEY_VALUE is called.
		 * '\\' -> [RV] SINGLE_QUOTE_ESCAPE | '\'' -> [RV] JUMP_NOISE */
		void VALUE_SINGLE_QUOTE();

		/** Reads a value enclosed in single quotes (') for PORTAGE_ESCAPES.
		 * Copy INPUT into value-buffer while INPUT is not in ['\\].
		 * If the value ends, ASSIGN_KEY_VALUE is called.
		 * '\\' -> [RV] SINGLE_QUOTE_ESCAPE_PORTAGE | '\'' -> [RV] JUMP_NOISE */
		void VALUE_SINGLE_QUOTE_PORTAGE();

		/** Read value enclosed in double-quotes (").
		 * Copy INPUT into value-buffer while INPUT is not in ["\\].
		 * If the value ends, ASSIGN_KEY_VALUE is called.
		 * '\\' -> [RV] DOUBLE_QUOTE_ESCAPE | '"' -> [RV] JUMP_NOISE */
		void VALUE_DOUBLE_QUOTE();

		/** Read value enclosed in double-quotes (") for PORTAGE_ESCAPES.
		 * Copy INPUT into value-buffer while INPUT is not in ["\\].
		 * If the value ends, ASSIGN_KEY_VALUE is called.
		 * '\\' -> [RV] DOUBLE_QUOTE_ESCAPE_PORTAGE] | '"' -> [RV] JUMP_NOISE */
		void VALUE_DOUBLE_QUOTE_PORTAGE();

		/** Read value not enclosed in any quotes.
		 * Thus there are no spaces and tabs allowed. Everything must be escaped.
		 * Move INPUT into buffer while it's not in [ \t\n\r\\'"#;|&)].
		 * If the value ends we call EVAL_READ.
		 * [# \t\r\n] -> EVAL_READ |
		 * '\\' -> WHITESCAPE_ESCAPE | \' -> VALUE_SINGLE_QUOTE |
		 * '"' -> VALUE_DOUBLE_QUOTE */
		void VALUE_WHITESPACE();

		/** Read value not enclosed in any quotes for PORTAGE_ESCAPES.
		 * Thus there are no spaces and tabs allowed. Everything must be escaped.
		 * Move INPUT into buffer while it's not in [ \t\n\r\\'"#].
		 * If the value ends we call EVAL_READ.
		 * [# \t\r\n] -> EVAL_READ |
		 * '\\' -> WHITESCAPE_ESCAPE_PORTAGE | \' -> VALUE_SINGLE_QUOTE_PORTAGE |
		 * '"' -> VALUE_DOUBLE_QUOTE_PORTAGE */
		void VALUE_WHITESPACE_PORTAGE();

		/** Looks if the following input is a valid section-part -> JUMP_NOISE */
		void EVAL_SECTION();

		/** Cares about \\ in single-quote values.
		 * \n is ignored, "\\'" transforms to "'" .. for everything
		 * else the \\ is just put into buffer.
		 * -> [RV] VALUE_SINGLE_QUOTE */
		void SINGLE_QUOTE_ESCAPE();

		/** Cares about \\ in single-quote values for PORTAGE_ESCAPES.
		 * \n is ignored, "\\'" transforms to "'" .. for everything
		 * else the \\ is just put into buffer.
		 * -> [RV] VALUE_SINGLE_QUOTE_PORTAGE */
		void SINGLE_QUOTE_ESCAPE_PORTAGE();

		/** Cares about \\ in double-quote values.
		 * \n is ignored, everything else is put into buffer without the \\.
		 * -> [RV] VALUE_DOUBLE_QUOTE */
		void DOUBLE_QUOTE_ESCAPE();

		/** Cares about \\ in double-quote values for PORTAGE_ESCAPES.
		 * \n is ignored, known Portage escapes are handled
		 * and everything else is put into buffer without the \\.
		 * -> [RV] VALUE_DOUBLE_QUOTE_PORTAGE */
		void DOUBLE_QUOTE_ESCAPE_PORTAGE();

		/** Cares about \\ in values without quotes.
		 * \n is ignored, everything else is put into buffer without the \\.
		 * -> [RV] VALUE_WHITESPACE */
		void WHITESPACE_ESCAPE();

		/** Cares about \\ in values without quotes for PORTAGE_ESCAPES.
		 * \n is ignored, known Portage escapes are handled
		 * and everything else is put into buffer without the \\.
		 * -> [RV] VALUE_WHITESPACE_PORTAGE */
		void WHITESPACE_ESCAPE_PORTAGE();

		/** Cares about \\ in the noise. Everything is ignored (there is no buffer .. don't care!).
		 * -> [RV] JUMP_NOISE */
		void NOISE_ESCAPE();

		/** This little fellow was created to care for puny little single-quote-values (') in the noise surrounding us.
		 * Read while INPUT is not in [\\'].
		 * '\\' -> [RV] NOISE_SINGLE_QUOTE_ESCAPE | '\'' -> [RV] JUMP_NOISE */
		void NOISE_SINGLE_QUOTE();

		/** Like his happy brother NOISE_SINGLE_QUOTE, this one is for double-quote-values (") in the noise surrounding us.
		 * Read while INPUT is not in [\\"].
		 * '\\' -> [RV] NOISE_DOUBLE_QUOTE_ESCAPE | '"' -> [RV] JUMP_NOISE */
		void NOISE_DOUBLE_QUOTE();

		/** This one does not care at all about the \\ in single-quote-values in the noise.
		 * And thats what it should!
		 * [RV] -> NOISE_SINGLE_QUOTE */
		void NOISE_SINGLE_QUOTE_ESCAPE();

		/** Realy lazy thing .. doesn't do anything about the \\ in double-quote-values in the noise.
		 * It just sits on the couch making faces at me! Stupid thing ..
		 * [RV] -> NOISE_DOUBLE_QUOTE */
		void NOISE_DOUBLE_QUOTE_ESCAPE();

		void var_append(char *beginning, size_t ref_key_length) ATTRIBUTE_NONNULL_;

		/** Try to resolve $... references to variables.
		 * If we fail we recover from it. However, INPUT_EOF might be true at stop. */
		void resolveReference();

		/** Try to resolve %(...)s references to variables.
		 * If we fail we recover from it. However, INPUT_EOF might be true at stop. */
		void resolveSectionReference();

		/** Read file using a new instance of VarsReader with the same
		    settings (except for APPEND_VALUES),
		    adding variables and changed HAVE_READ to current instance. */
		bool source(const std::string& filename, std::string *errtext);

		size_t key_len; /**< Length of the key. */
		char *key_begin;      /**< Pointer to first character of key. */

		char *x;        /**< Pointer to current position in filebuffer. */
		bool sourcecmd; /**< A flag whether we are currently parsing a source command */

		std::string value; /**< Buffy for value */

		std::string section; /**< current section */

		Flags parse_flags; /**< Flags for configuration of parser. */

	protected:
		WordSet *sourced_files;

		std::string m_errtext;

		char *filebuffer,     /**< The filebuffer everyone is taking about. */
		     *filebuffer_end; /**< Marks the end of the filebuffer. */
		const char **incremental_keys;  /**< c-array of pattern for keys which values should be prepended to the new value. */
		/** Mapping of key to value. */
		my_map *vars;

		/** Prefix resp. varname used for sourcing */
		std::string source_prefix;

		/** Init the FSM.
		 * Mallocs buffer for file and reads it into this buffer.
		 * @warning You need to call deinit() if you called this function. */
		void initFsm();

		/** True if c matches [A-Za-z0-9_] */
		static bool isValidKeyCharacter(char c) ATTRIBUTE_PURE {
			return (isalnum(c, localeC) || (c == '_') || (c == '-'));
		}

		/** True if c matches [A-Za-z_] */
		static bool isValidKeyCharacterStart(char c) ATTRIBUTE_PURE {
			return (isalpha(c, localeC) || (c == '_'));
		}

		const char *file_name; /**< Name of parsed file. */

		/** Manages the mess around it.
		 * Nothing to say .. it calls functions acording to the current state
		 * and returns if the state is STOP. */
		bool runFsm();

		bool isIncremental(const char *key) {
			return match_list(incremental_keys, key);
		}
};

#endif  // SRC_EIXTK_VARSREADER_H_
