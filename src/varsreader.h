// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __VARSREADER_H__
#define __VARSREADER_H__

#if !defined(__OpenBSD__)
#include <cstdint>
#endif

// mmap and stat stuff
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <string>
#include <map>

/** A wrapper to FSM that can read shell-style key=value declarations.
 * The constructor inits, starts the FSM. Then you can access them .. The deconstructor deinits it. */
class VarsReader {
	public:
		typedef uint8_t Flags;
		static const Flags
			NONE                 = 0x00, /**< Flag: No flags set; normal behavior. */
			ONLY_KEYWORDS_SLOT   = 0x01, /**< Flag: Only read "KEYWORDS" and "SLOT" once, than exit the parser. */
			KEYWORDS_READ        = 0x02, /**< Flag: Have already read "KEYWORDS" once. */
			SLOT_READ            = 0x04, /**< Flag: Have already read "SLOT" once. */
			SUBST_VARS           = 0x08, /**< Flag: Allow references to variable in declarations
			                                  of a variable. i.e.  USE="${USE} -kde" */
			INTO_MAP             = 0x10, /**< Flag: Init but don't parse .. you must first supply
			                                  a pointer to map<string,string> with useMap(...) */
			APPEND_VALUES        = 0x20, /**< Flag: appended new values rather then replace the old value. */
			ALLOW_SOURCE         = 0x40, /**< Flag: Allow "source"/"." command. */
			ALLOW_SOURCE_VARNAME = 0xc0, /**< Flag: Allow "source"/"." but
			                                  Prefix is only a varname which
			                                  might be modified during sourcing. */
			HAVE_READ            = KEYWORDS_READ|SLOT_READ,     /**< Combination of previous "*_READ" */
			ONLY_HAVE_READ       = ONLY_KEYWORDS_SLOT|HAVE_READ;/**< Combination of HAVE_READ and ONLY_KEYWORDS_SLOT */


		/** Init and parse the FSM depending on supplied flags. */
		VarsReader(Flags flags) {
			parse_flags = flags;
			if( ! (parse_flags & INTO_MAP) ) {
				vars = new std::map<std::string,std::string>;
			}
		}

		/** Free memory. */
		virtual ~VarsReader() {
			if( !(parse_flags & INTO_MAP) )
				delete  vars;
		}

		/** Read file.
		 * @return true if the file was successfully read. */
		bool read(const char *filename);

		/** Use a supplied map for variables. */
		void useMap(std::map<std::string,std::string> *vars_map) {
			vars = vars_map;
		}

		/** Prefix (path resp. varname) used for sourcing */
		void setPrefix(std::string prefix) {
			source_prefix = prefix;
		}

		/** Set array of keys which values should be prepended to the new value. */
		void accumulatingKeys(const char **keys) {
			incremental_keys = keys;
		}

		/** Operator that helps us to be used like a map. */
		std::string& operator[] (std::string key) {
			return (*vars)[key];
		}

	private:
		/** Kepper of the holy state. */
		enum States {
			state_STOP,
			state_JUMP_NOISE,
			state_JUMP_COMMENT,
			state_JUMP_WHITESPACE,
			state_FIND_ASSIGNMENT,
			state_EVAL_VALUE,
			state_VALUE_SINGLE_QUOTE,
			state_VALUE_DOUBLE_QUOTE,
			state_VALUE_WHITESPACE,
			state_DOUBLE_QUOTE_ESCAPE,
			state_WHITESPACE_ESCAPE,
			state_NOISE_ESCAPE,
			state_NOISE_SINGLE_QUOTE,
			state_NOISE_DOUBLE_QUOTE
		} STATE;

		/** Eats most of the noise between declarations.
		 * Read until '\n' '#' '\\' '\'' '"'
		 * '\\' -> [RV] NOISE_ESCAPE | '\'' -> [RV] NOISE_SINGLE_QUOTE | '"' -> [RV] NOISE_DOUBLE_QUOTE | '#' -> [RV] JUMP_COMMENT
		 * '\n' -> [RV] (and check if we are at EOF, EOF's only occur after a newline) -> JUMP_WHITESPACE */
		void JUMP_NOISE();

		/** Jumps comments.
		 * Read until the next '\n' comes in. Then move to JUMP_NOISE. */
		void JUMP_COMMENT();

		/** Jump whitespaces and tabs at begining of line.
		 * Read while ' ' || '\\t'
		 * '#' -> [RV] JUMP_COMMENT | [A-Z_] -> (reset key) FIND_ASSIGNMENT | -> JUMP_NOISE
		 * @see IS_STAB_KEY_CHARACTER */
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
		 * '\'' -> [RV] VALUE_SINGLE_QUOTE | '"' -> [RV] VALUE_DOUBLE_QUOTE | '\\' -> [RV] WHITESPACE_ESCAPE | -> VALUE_WHITESPACE
		 * If we have the begin of a valid value we are reseting the value-buffer. */
		void EVAL_VALUE();

		/** Reads a value enclosed in single quotes (').
		 * Copy INPUT into value-buffer while INPUT is not in ['\\]. If the value ends, ASSIGN_KEY_VALUE is
		 * called.
		 * '\\' -> [RV] SINGLE_QUOTE_ESCAPE | '\'' -> [RV] JUMP_NOISE */
		void VALUE_SINGLE_QUOTE();

		/** Read value enclosed in double-quotes (").
		 * Copy INPUT into value-buffer while INPUT is not in ["\\]. If the value ends, ASSIGN_KEY_VALUE is
		 * called.
		 * '\\' -> [RV] DOUBLE_QUOTE_ESCAPE | '"' -> [RV] JUMP_NOISE */
		void VALUE_DOUBLE_QUOTE();

		/** Read value not inclosed in any quotes.
		 * Thus there are no spaces and tabs allowed. Everything must be escaped.
		 * Move INPUT into buffer while it's not in [ \\t\\n\\]. If the value ends we call ASSIGN_KEY_VALUE.
		 * [\\n\\t ] -> (ASSIGN_KEY_VALUE) JUMP_NOISE | '\\' -> WHITESPACE_ESCAPE */
		void VALUE_WHITESPACE();

		/** Cares about \\ in single-quote values. \n is ignored, "\\'" transforms to "'" .. for everything
		 * else the \\ is just put into buffer.
		 * -> [RV] VALUE_SINGLE_QUOTE */
		void SINGLE_QUOTE_ESCAPE();

		/** Cares about \\ in double-quote values. \n is ignored, everything else is put into buffer without
		 * the \\.
		 * -> [RV] VALUE_DOUBLE_QUOTE */
		void DOUBLE_QUOTE_ESCAPE();

		/** Cares about \\ in values without quotes. \n is ignored, everything else is put into buffer without
		 * the \\.
		 * -> [RV] VALUE_WHITESPACE */
		void WHITESPACE_ESCAPE();

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

		/** Assign key=value or source file.
		    Return true if a stop is required due to ONLY_KEYWORDS_SLOT */
		bool assign_key_value();

		/** Resolve references to a variable in declaration of a variable. */
		void resolveReference();

		/** Read file using a new instance of VarsReader with the same
		    settings (except for APPEND_VALUES),
		    adding variables and changed HAVE_READ to current instance. */
		bool source(const std::string &filename);

		unsigned int key_len; /**< Lenght of the key. */
		char *key_begin;      /**< Pointer to first character of key. */

		char *x;         /**< Pointer to current position in filebuffer. */
		bool sourcecmd; /* A flag whether we are currently parsing a source command */

		std::string value;     /**< Buffy for value */

		Flags parse_flags; /**< Flags for configuration of parser. */

	protected:
		char *filebuffer, /**< The filebuffer everyone is taking about. */
			 *filebuffer_end; /**< Marks the end of the filebuffer. */
		const char **incremental_keys; /**< c-array of pattern for keys which values
										 should be prepended to the new value. */
		/** Mapping of key to value. */
		std::map<std::string,std::string> *vars;

		/** Prefix resp. varname used for sourcing */
		std::string source_prefix;

		/** Init the FSM.
		 * Mallocs buffer for file and reads it into this buffer.
		 * @warning You need to call deinit() if you called this function. */
		void initFsm();

		/** True if c matches [A-Z_0-9] */
		virtual bool isValidKeyCharacter(char c) {
			return ((c >= 'A' && c <= 'Z')
					|| c == '_' || (c >= '0' && c <= '9'));
		}

		/** True if c matches [A-Z_] */
		virtual bool isValidKeyCharacterStart(char c) {
			return ((c >= 'A' && c <= 'Z')
					|| c == '_' );
		}

		const char *file_name; /**< Name of parsed file. */

		/** Manages the mess around it.
		 * Nothing to say .. it calls functions acording to the current state
		 * and returns if the state is STOP. */
		void runFsm();

		bool isIncremental(const char *key);

};

#endif /* __VARSREADER_H__ */
