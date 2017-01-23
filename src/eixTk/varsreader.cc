// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
#include "eixTk/filenames.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringtypes.h"
#include "eixTk/stringutils.h"
#include "eixTk/utils.h"
#include "eixTk/varsreader.h"

/**
Current input for FSM
**/
#define INPUT (*(x))

/**
Switch to different state
**/
#define CHSTATE(z) do { \
		STATE = &VarsReader::z; \
		return; \
	} while(0)

#define STOP do { \
		STATE = NULLPTR; \
		return; \
	} while(0)

#define ERROR do { \
		retstate = false; \
		STATE = NULLPTR; \
		return; \
	} while(0)

/**
Move to next input and check for end of buffer
**/
#define NEXT_INPUT do { \
		if(unlikely(++(x) == filebuffer_end)) \
			STOP; \
	} while(0)

#define SKIP_SPACE do { \
		while((INPUT == '\t') || (INPUT == ' ')) NEXT_INPUT; \
	} while(0)

#define NEXT_INPUT_EVAL do { \
	if(unlikely(++(x) == filebuffer_end)) \
		CHSTATE(EVAL_READ); \
	} while(0)

#define INPUT_EOF (unlikely((x) == filebuffer_end))

#define NEXT_INPUT_OR_EOF do { \
		if(!INPUT_EOF) \
			++(x); \
	} while(0)

#define APPEND_RET do { \
		x = backup; \
		VALUE_APPEND(INPUT); \
		NEXT_INPUT_OR_EOF; \
		return; \
	} while(0);

#define NEXT_INPUT_APPEND_RET do { \
	if(unlikely(++(x) == filebuffer_end)) \
		APPEND_RET; \
	} while(0)

/**
Check value-buffer for overrun and push c into the current value-buffer
**/
#define VALUE_APPEND(c) do { \
		value.append(&(c), 1); \
	} while(0)

/**
Reset value pointer
**/
#define VALUE_CLEAR value.clear()

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

const VarsReader::Flags
	VarsReader::NONE,
	VarsReader::ONLY_KEYWORDS_SLOT,
	VarsReader::KEYWORDS_READ,
	VarsReader::SLOT_READ,
	VarsReader::SUBST_VARS,
	VarsReader::INTO_MAP,
	VarsReader::APPEND_VALUES,
	VarsReader::ALLOW_SOURCE,
	VarsReader::ALLOW_SOURCE_VARNAME,
	VarsReader::PORTAGE_ESCAPES,
	VarsReader::PORTAGE_SECTIONS,
	VarsReader::RECURSE,
	VarsReader::HAVE_READ,
	VarsReader::ONLY_HAVE_READ;

// ************************************************************************
// ********************* FSM states begin here ****************************

/**
Eat most of the noise between declarations.
Read until '\n' '#' '\\' '\'' '"'
'\\' -> [RV] NOISE_ESCAPE | '\'' -> [RV] NOISE_SINGLE_QUOTE | '"' -> [RV] NOISE_DOUBLE_QUOTE | '#' -> [RV] JUMP_COMMENT
'\n' -> [RV] (and check if we are at EOF, EOF's only occur after a newline) -> JUMP_WHITESPACE
**/
void VarsReader::JUMP_NOISE() {
	while(likely(INPUT != '#' && INPUT != '\n' && INPUT != '\'' && INPUT != '"' && INPUT != '\\')) NEXT_INPUT;
	switch(INPUT) {
		case '#':   NEXT_INPUT;
		            CHSTATE(JUMP_COMMENT);
		case '\\':  NEXT_INPUT;
		            CHSTATE(NOISE_ESCAPE);
		case '"':   NEXT_INPUT;
		            CHSTATE(NOISE_DOUBLE_QUOTE);
		case '\'':  NEXT_INPUT;
		            CHSTATE(NOISE_SINGLE_QUOTE);
		default:    break;
	}
	NEXT_INPUT;
	CHSTATE(JUMP_WHITESPACE);
}

/**
Assign key=value or source file and change state to JUMP_NOISE (or STOP
if an early stop is required due to ONLY_KEYWORDS_SLOT or ONLY_HAVE_READ)
-> [RV] JUMP_NOISE
**/
void VarsReader::ASSIGN_KEY_VALUE() {
	if(unlikely(sourcecmd)) {
		sourcecmd = false;
		string errtext;
		if(unlikely(!source(value, &errtext))) {
			m_errtext = eix::format(_("%s: failed to source %s (%s)")) % file_name % value % errtext;
			ERROR;
		}
		if((parse_flags & ONLY_HAVE_READ) == ONLY_HAVE_READ) {
			STOP;
		}
	} else if(unlikely( (parse_flags & ONLY_KEYWORDS_SLOT) )) {
		if(unlikely(strncmp("KEYWORDS=", key_begin, 9) == 0)) {
			(*vars)[string(key_begin, key_len)] = value;
			parse_flags |= KEYWORDS_READ;
			if(parse_flags & SLOT_READ) {
				STOP;
			}
		} else if(unlikely(strncmp("SLOT=", key_begin, 5) == 0)) {
			(*vars)[string(key_begin, key_len)] = value;
			parse_flags |= SLOT_READ;
			if(parse_flags & KEYWORDS_READ) {
				STOP;
			}
		}
	} else {
		(*vars)[string(key_begin, key_len) + section] = value;
	}
	if(INPUT_EOF) {
		STOP;
	}
	CHSTATE(JUMP_NOISE);
}

/**
Jump comments.
Read until the next '\n' comes in. Then move to JUMP_NOISE.
**/
void VarsReader::JUMP_COMMENT() {
	while(likely(INPUT != '\n')) {
		NEXT_INPUT;
	}
	CHSTATE(JUMP_NOISE);
}

/**
Jump whitespaces and tabs at begining of line.
Read while ' ' || '\t'
Comment line -> JUMP_WHITESPACE | [A-Z_] -> (reset key) FIND_ASSIGNMENT |
source or . -> EVAL_VALUE with sourcecmd=true | -> JUMP_NOISE
'[' -> EVAL_SECTION
@see isValidKeyCharacter
**/
void VarsReader::JUMP_WHITESPACE() {
	sourcecmd = false;
	SKIP_SPACE;
	switch(INPUT) {
		case ';':
			if((parse_flags & PORTAGE_SECTIONS) == NONE) {
				break;
			}
		case '#':
			NEXT_INPUT;
			while(likely(INPUT != '\n')) {
				NEXT_INPUT;
			}
			CHSTATE(JUMP_WHITESPACE);
			break;
		case 's': {
				size_t i(0);
				const char *beginning(x);
				while(likely(my_isalpha(INPUT))) {
					NEXT_INPUT;
					++i;
				}
				if(unlikely((i != 6) || strncmp("source", beginning, 6) != 0)) {
					CHSTATE(JUMP_NOISE);
					break;
				}
			}
			--x;
		case '.':
			NEXT_INPUT;
			if((parse_flags & ALLOW_SOURCE) &&
				(INPUT == '\t' || INPUT == ' ')) {
				sourcecmd = true;
				CHSTATE(EVAL_VALUE);
			} else {
				CHSTATE(JUMP_NOISE);
			}
			break;
		case '[':
			if((parse_flags & PORTAGE_SECTIONS) != NONE) {
				NEXT_INPUT;
				CHSTATE(EVAL_SECTION);
			}
			break;
		default:
			break;
	}
	if(isValidKeyCharacterStart(INPUT)) {
		key_begin = x;
		CHSTATE(FIND_ASSIGNMENT);
	}
	CHSTATE(JUMP_NOISE);
}

/**
Find key .. if there is one :).
Keys are marked with a '=' at the end ('=' is not part of the key). This state always gets the
first character of the keyword as current INPUT, thus we don't need to check if key_len is >= 1,
is always >= 1.
Read while not ('=' || '#') and [A-Z_0-9]
'=' -> EVAL_VALUE | '#' -> [RV] JUMP_COMMENT | -> JUMP_NOISE
**/
void VarsReader::FIND_ASSIGNMENT() {
	key_len = 0;
	if(likely((parse_flags & PORTAGE_SECTIONS) == NONE)) {
		while(likely(isValidKeyCharacter(INPUT))) {
			++key_len;
			NEXT_INPUT;
		}
		SKIP_SPACE;
	} else {
		while(likely(isValidKeyCharacter(INPUT) || (INPUT == ' ') || (INPUT == '\t'))) {
			if(unlikely((INPUT == ' ') || (INPUT == '\t'))) {
				const char *beginning(x);
				SKIP_SPACE;
				if(isValidKeyCharacter(INPUT)) {
GCC_DIAG_OFF(sign-conversion)
					key_len += (x - beginning);
GCC_DIAG_ON(sign-conversion)
				}
			} else {
				++key_len;
				NEXT_INPUT;
			}
		}
	}
	switch(INPUT) {
		case ':':
			if((parse_flags & PORTAGE_SECTIONS) == NONE) {
				break;
			}
		case '=':
			CHSTATE(EVAL_VALUE);
		case ';':
			if((parse_flags & PORTAGE_SECTIONS) == NONE) {
				break;
			}
		case '#':
			NEXT_INPUT;
			CHSTATE(JUMP_COMMENT);
		default:
			break;
	}
	CHSTATE(JUMP_NOISE);
}

/**
Look if the following input is a valid value-part.
['"\^#\n\t ] is allowed.
[\n\t ] -> JUMP_NOISE | '#' -> [RV] JUMP_COMMENT
'\'' -> [RV] VALUE_SINGLE_QUOTE{_PORTAGE}
'"' -> [RV] VALUE_DOUBLE_QUOTE{_PORTAGE}
'\\' -> [RV] WHITESPACE_ESCAPE{_PORTAGE} | -> VALUE_WHITESPACE{_PORTAGE}
We are resetting the value-buffer.
**/
void VarsReader::EVAL_VALUE() {
	NEXT_INPUT;
	VALUE_CLEAR;
	if(parse_flags & PORTAGE_ESCAPES) {
		SKIP_SPACE;
		switch(INPUT) {
			case '"':   NEXT_INPUT;
			            CHSTATE(VALUE_DOUBLE_QUOTE_PORTAGE);
			case '\'':  NEXT_INPUT;
			            CHSTATE(VALUE_SINGLE_QUOTE_PORTAGE);
			case '\r':
			case '\n':  CHSTATE(ASSIGN_KEY_VALUE);
			case '#':   NEXT_INPUT;
			            CHSTATE(JUMP_COMMENT);
			case '\\':  NEXT_INPUT;
			            CHSTATE(WHITESPACE_ESCAPE_PORTAGE);
			default:    CHSTATE(VALUE_WHITESPACE_PORTAGE);
		}
	}
	switch(INPUT) {
		case '"':   NEXT_INPUT;
		            CHSTATE(VALUE_DOUBLE_QUOTE);
		case '\'':  NEXT_INPUT;
		            CHSTATE(VALUE_SINGLE_QUOTE);
		case '\r':
		case '\n':  CHSTATE(ASSIGN_KEY_VALUE);
		case '\t':
		case ' ':   CHSTATE(JUMP_NOISE);
		case '#':   NEXT_INPUT;
		            CHSTATE(JUMP_COMMENT);
		case '\\':  NEXT_INPUT;
		            CHSTATE(WHITESPACE_ESCAPE);
		default:    CHSTATE(VALUE_WHITESPACE);
	}
}

/**
Look if the following input is a valid secondary value-part.
['"\^#\n\r\t ] is allowed.
[\n\r\t# ] (or [\n\r\t;|&)# ]) -> ASSIGN_KEY_VALUE
'\'' -> [RV] VALUE_SINGLE_QUOTE{_PORTAGE}
'"' -> [RV] VALUE_DOUBLE_QUOTE{_PORTAGE}
'\\' -> [RV] WHITESPACE_ESCAPE{_PORTAGE} | -> VALUE_WHITESPACE{_PORTAGE}
**/
void VarsReader::EVAL_READ() {
	if(INPUT_EOF) {
		CHSTATE(ASSIGN_KEY_VALUE);
	}
	if(parse_flags & PORTAGE_ESCAPES) {
		switch(INPUT) {
			case '"':   NEXT_INPUT_EVAL;
			            CHSTATE(VALUE_DOUBLE_QUOTE_PORTAGE);
			case '\'':  NEXT_INPUT_EVAL;
			            CHSTATE(VALUE_SINGLE_QUOTE_PORTAGE);
			case '\t':
			case '\r':
			case '\n':
			case ' ':
			case '#':   CHSTATE(ASSIGN_KEY_VALUE);
			case '\\':  NEXT_INPUT_EVAL;
			            CHSTATE(WHITESPACE_ESCAPE_PORTAGE);
			default:    CHSTATE(VALUE_WHITESPACE_PORTAGE);
		}
	}
	switch(INPUT) {
		case '"':   NEXT_INPUT_EVAL;
		            CHSTATE(VALUE_DOUBLE_QUOTE);
		case '\'':  NEXT_INPUT_EVAL;
		            CHSTATE(VALUE_SINGLE_QUOTE);
		case '\t':
		case '\r':
		case '\n':
		case ' ':
		case '#':
		case ';':
		case '|':
		case '&':
		case ')':   CHSTATE(ASSIGN_KEY_VALUE);
		case '\\':  NEXT_INPUT_EVAL;
		            CHSTATE(WHITESPACE_ESCAPE);
		default:    CHSTATE(VALUE_WHITESPACE);
	}
}

/**
Backslash escapes supported by portage.
**/
static CONSTEXPR const char ESC_A = 007;
static CONSTEXPR const char ESC_B = 010;
static CONSTEXPR const char ESC_E = 033;
static CONSTEXPR const char ESC_N = 012;
static CONSTEXPR const char ESC_R = 015;
static CONSTEXPR const char ESC_T = 011;
static CONSTEXPR const char ESC_V = 013;
/**
and helper ones.
**/
static CONSTEXPR const char ESC_BS = '\\';
static CONSTEXPR const char ESC_SP = ' ';

/**
Read a value enclosed in single quotes (').
Copy INPUT into value-buffer while INPUT is not in ['\\].
If the value ends, EVAL_READ is called.
'\\' -> [RV] SINGLE_QUOTE_ESCAPE | '\'' -> [RV] EVAL_READ
**/
void VarsReader::VALUE_SINGLE_QUOTE() {
	while(likely((INPUT != '\'') && (INPUT != '\\'))) {
		VALUE_APPEND(INPUT);
		NEXT_INPUT_EVAL;
	}
	switch(INPUT) {
		case '\'':  NEXT_INPUT_EVAL;
		            CHSTATE(EVAL_READ);
		default:    NEXT_INPUT_EVAL;
		            CHSTATE(SINGLE_QUOTE_ESCAPE);
	}
}

/**
Read a value enclosed in single quotes (') for PORTAGE_ESCAPES.
Copy INPUT into value-buffer while INPUT is not in ['\\].
If the value ends, EVAL_READ is called.
'\\' -> [RV] SINGLE_QUOTE_ESCAPE_PORTAGE | '\'' -> [RV] EVAL_READ
**/
void VarsReader::VALUE_SINGLE_QUOTE_PORTAGE() {
	while(likely((INPUT != '\'') && (INPUT != '\\'))) {
		if(unlikely((INPUT == '$') && ((parse_flags & SUBST_VARS) != NONE))) {
			resolveReference();
			if(INPUT_EOF) {
				CHSTATE(EVAL_READ);
			}
			continue;
		}
		if(unlikely((INPUT == '%') && ((parse_flags & (SUBST_VARS|PORTAGE_SECTIONS)) == (SUBST_VARS|PORTAGE_SECTIONS)))) {
			resolveSectionReference();
			if(INPUT_EOF) {
				CHSTATE(EVAL_READ);
			}
			continue;
		}
		if(unlikely(INPUT == '\n')) {
			VALUE_APPEND(ESC_SP);
		} else {
			VALUE_APPEND(INPUT);
		}
		NEXT_INPUT_EVAL;
	}
	switch(INPUT) {
		case '\'':
			NEXT_INPUT_EVAL;
			CHSTATE(EVAL_READ);
		default:
			NEXT_INPUT_EVAL;
			CHSTATE(SINGLE_QUOTE_ESCAPE_PORTAGE);
	}
}

/**
Read value enclosed in double-quotes (").
Copy INPUT into value-buffer while INPUT is not in ["\\].
If the value ends, EVAL_READ is called.
'\\' -> [RV] DOUBLE_QUOTE_ESCAPE | '"' -> [RV] EVAL_READ
**/
void VarsReader::VALUE_DOUBLE_QUOTE() {
	while(likely(INPUT != '"' && INPUT != '\\')) {
		if(unlikely(INPUT == '$' && ((parse_flags & SUBST_VARS) != NONE))) {
			resolveReference();
			if(INPUT_EOF) {
				CHSTATE(EVAL_READ);
			}
			continue;
		}
		VALUE_APPEND(INPUT);
		NEXT_INPUT_EVAL;
	}
	switch(INPUT) {
		case '"':  NEXT_INPUT_EVAL;
		           CHSTATE(EVAL_READ);
		default:   NEXT_INPUT_EVAL;
		           CHSTATE(DOUBLE_QUOTE_ESCAPE);
	}
}

/**
Read value enclosed in double-quotes (") for PORTAGE_ESCAPES.
Copy INPUT into value-buffer while INPUT is not in ["\\].
If the value ends, EVAL_READ is called.
'\\' -> [RV] DOUBLE_QUOTE_ESCAPE_PORTAGE | '"' -> [RV] EVAL_READ
**/
void VarsReader::VALUE_DOUBLE_QUOTE_PORTAGE() {
	while(likely(INPUT != '"' && INPUT != '\\')) {
		if(unlikely((INPUT == '$') && ((parse_flags & SUBST_VARS) != NONE))) {
			resolveReference();
			if(INPUT_EOF) {
				CHSTATE(EVAL_READ);
			}
			continue;
		}
		if(unlikely((INPUT == '%') && ((parse_flags & (SUBST_VARS|PORTAGE_SECTIONS)) == (SUBST_VARS|PORTAGE_SECTIONS)))) {
			resolveSectionReference();
			if(INPUT_EOF) {
				CHSTATE(EVAL_READ);
			}
			continue;
		}
		if(unlikely(INPUT == '\n')) {
			VALUE_APPEND(ESC_SP);
		} else {
			VALUE_APPEND(INPUT);
		}
		NEXT_INPUT_EVAL;
	}
	switch(INPUT) {
		case '"':
			NEXT_INPUT_EVAL;
			CHSTATE(EVAL_READ);
		default:
			NEXT_INPUT_EVAL;
			CHSTATE(DOUBLE_QUOTE_ESCAPE_PORTAGE);
	}
}

/**
Read value not enclosed in any quotes.
Thus there are no spaces and tabs allowed. Everything must be escaped.
Move INPUT into buffer while it's not in [ \t\n\r\\'"#;|&)].
If the value ends we call EVAL_READ.
[# \t\r\n] -> EVAL_READ |
'\\' -> WHITESCAPE_ESCAPE | \' -> VALUE_SINGLE_QUOTE |
'"' -> VALUE_DOUBLE_QUOTE
**/
void VarsReader::VALUE_WHITESPACE() {
	for(;;) {
		switch(INPUT) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '#':
			case ';':
			case '|':
			case '&':
			case ')':
				CHSTATE(EVAL_READ);
			case '\\':
				NEXT_INPUT_EVAL;
				CHSTATE(WHITESPACE_ESCAPE);
			case '\'':
				NEXT_INPUT_EVAL;
				CHSTATE(VALUE_SINGLE_QUOTE);
			case '"':
				NEXT_INPUT_EVAL;
				CHSTATE(VALUE_DOUBLE_QUOTE);
			case '$':
				if((parse_flags & SUBST_VARS) != NONE) {
					resolveReference();
					if(INPUT_EOF) {
						CHSTATE(EVAL_READ);
					}
					continue;
				}
			default:
				VALUE_APPEND(INPUT);
				NEXT_INPUT_EVAL;
		}
	}
}

/**
Read value not enclosed in any quotes for PORTAGE_ESCAPES.
Thus there are no spaces and tabs allowed. Everything must be escaped.
Move INPUT into buffer while it's not in [ \t\n\r\\'"#].
If the value ends we call EVAL_READ.
[# \t\r\n] -> EVAL_READ |
'\\' -> WHITESCAPE_ESCAPE_PORTAGE | \' -> VALUE_SINGLE_QUOTE_PORTAGE |
'"' -> VALUE_DOUBLE_QUOTE_PORTAGE
**/
void VarsReader::VALUE_WHITESPACE_PORTAGE() {
	for(;;) {
		switch(INPUT) {
			case '#':
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				CHSTATE(EVAL_READ);
			case '\\':
				NEXT_INPUT_EVAL;
				CHSTATE(WHITESPACE_ESCAPE_PORTAGE);
			case '\'':
				NEXT_INPUT_EVAL;
				CHSTATE(VALUE_SINGLE_QUOTE_PORTAGE);
			case '"':
				NEXT_INPUT_EVAL;
				CHSTATE(VALUE_DOUBLE_QUOTE_PORTAGE);
			case '$':
				if((parse_flags & SUBST_VARS) != NONE) {
					resolveReference();
					if(INPUT_EOF) {
						CHSTATE(EVAL_READ);
					}
					continue;
				}
				break;
			case '%':
				if((parse_flags & (SUBST_VARS|PORTAGE_SECTIONS)) == (SUBST_VARS|PORTAGE_SECTIONS)) {
					resolveSectionReference();
					if(INPUT_EOF) {
						CHSTATE(EVAL_READ);
					}
					continue;
				}
				break;
			default:
				break;
		}
		VALUE_APPEND(INPUT);
		NEXT_INPUT_EVAL;
	}
}

/**
Look if the following input is a valid section-part -> JUMP_NOISE
**/
void VarsReader::EVAL_SECTION() {
	VALUE_CLEAR;
	SKIP_SPACE;
	for(;;) {
		switch(INPUT) {
			case ']':
				if(value != "DEFAULT") {
					section.assign(1, ':');
					section.append(value);
				}
			case '\r':
			case '\n':
				CHSTATE(JUMP_NOISE);
			case '\\':
				NEXT_INPUT;
				switch(INPUT) {
					case '\\':
						NEXT_INPUT;
						if(INPUT == '\\') {
							NEXT_INPUT;
						}
						switch(INPUT) {
							case 'a':   VALUE_APPEND(ESC_A); break;
							case 'b':   VALUE_APPEND(ESC_B); break;
							case 'e':   VALUE_APPEND(ESC_E); break;
							case 'f':
							case 'n':   VALUE_APPEND(ESC_N); break;
							case 'r':   VALUE_APPEND(ESC_R); break;
							case 't':   VALUE_APPEND(ESC_T); break;
							case 'v':   VALUE_APPEND(ESC_V); break;
							case '\r':
							case '\n':  CHSTATE(JUMP_NOISE); break;
							default:    VALUE_APPEND(INPUT);
						}
						break;
					default:    VALUE_APPEND(INPUT);
				}
				NEXT_INPUT;
				break;
			default:
				VALUE_APPEND(INPUT);
				NEXT_INPUT;
		}
	}
}

/**
Care about \\ in single-quote values.
\n is ignored, and everything else is put into buffer without the \\.
-> [RV] VALUE_SINGLE_QUOTE
**/
void VarsReader::SINGLE_QUOTE_ESCAPE() {
	switch(INPUT) {
		case '\n':  break;
		default:    VALUE_APPEND(INPUT);
	}
	NEXT_INPUT_EVAL;
	CHSTATE(VALUE_SINGLE_QUOTE);
}

/**
Care about \\ in single-quote values for PORTAGE_ESCAPES.
\n is ignored, known Portage escapes are handled
and everything else is put into buffer without the \\.
-> [RV] VALUE_SINGLE_QUOTE_PORTAGE
**/
void VarsReader::SINGLE_QUOTE_ESCAPE_PORTAGE() {
	bool wasnl(false);
	switch(INPUT) {
		case 'a':   VALUE_APPEND(ESC_A); break;
		case 'b':   VALUE_APPEND(ESC_B); break;
		case 'e':   VALUE_APPEND(ESC_E); break;
		case 'f':
		case 'n':   VALUE_APPEND(ESC_N); break;
		case 'r':   VALUE_APPEND(ESC_R); break;
		case 't':   VALUE_APPEND(ESC_T); break;
		case 'v':   VALUE_APPEND(ESC_V); break;
		case '\n':  wasnl = true; break;
		default:    VALUE_APPEND(INPUT); break;
	}
	NEXT_INPUT_EVAL;
	/* any amount of backslashes forbids portage to expand var */
	if(unlikely((INPUT == '$') && !wasnl)) {
		VALUE_APPEND(INPUT);
		NEXT_INPUT_EVAL;
	}
	CHSTATE(VALUE_SINGLE_QUOTE_PORTAGE);
}

/**
Care about \\ in double-quote values.
\n is ignored, everything else is put into buffer without the \\.
-> [RV] VALUE_DOUBLE_QUOTE
**/
void VarsReader::DOUBLE_QUOTE_ESCAPE() {
	switch(INPUT) {
		case '\n':  break;
		default:    VALUE_APPEND(INPUT);
	}
	NEXT_INPUT_EVAL;
	CHSTATE(VALUE_DOUBLE_QUOTE);
}

/**
Cares about \\ in double-quote values for PORTAGE_ESCAPES.
\n is ignored, known Portage escapes are handled
and everything else is put into buffer without the \\.
-> [RV] VALUE_DOUBLE_QUOTE_PORTAGE
**/
void VarsReader::DOUBLE_QUOTE_ESCAPE_PORTAGE() {
	bool wasnl(false);
	switch(INPUT) {
		case 'a':   VALUE_APPEND(ESC_A); break;
		case 'b':   VALUE_APPEND(ESC_B); break;
		case 'e':   VALUE_APPEND(ESC_E); break;
		case 'f':
		case 'n':   VALUE_APPEND(ESC_N); break;
		case 'r':   VALUE_APPEND(ESC_R); break;
		case 't':   VALUE_APPEND(ESC_T); break;
		case 'v':   VALUE_APPEND(ESC_V); break;
		case '\n':  wasnl = true; break;
		case '\\':
			NEXT_INPUT_EVAL;
			switch(INPUT) {
				case 'a':   VALUE_APPEND(ESC_A); break;
				case 'b':   VALUE_APPEND(ESC_B); break;
				case 'e':   VALUE_APPEND(ESC_E); break;
				case 'f':
				case 'n':   VALUE_APPEND(ESC_N); break;
				case 'r':   VALUE_APPEND(ESC_R); break;
				case 't':   VALUE_APPEND(ESC_T); break;
				case 'v':   VALUE_APPEND(ESC_V); break;
				case '\n':  wasnl = true; break;
				case '\\':
					VALUE_APPEND(INPUT);
					NEXT_INPUT_EVAL;
					switch(INPUT) {
						case '\\':   break;
						case '\n':   VALUE_APPEND(ESC_SP); wasnl = true; break;
						default:     VALUE_APPEND(INPUT); break;
					}
					break;
				default:    VALUE_APPEND(INPUT); break;
			}
			break;
		default:    VALUE_APPEND(INPUT); break;
	}
	NEXT_INPUT_EVAL;
	/* any amount of backslashes forbids portage to expand var */
	if(unlikely((INPUT == '$') && !wasnl)) {
		VALUE_APPEND(INPUT);
		NEXT_INPUT_EVAL;
	}
	CHSTATE(VALUE_DOUBLE_QUOTE_PORTAGE);
}

/**
Care about \\ in values without quotes.
\n is ignored, everything else is put into buffer without the \\.
-> [RV] VALUE_WHITESPACE
**/
void VarsReader::WHITESPACE_ESCAPE() {
	switch(INPUT) {
		case '\n':  break;
		default:    VALUE_APPEND(INPUT);
	}
	NEXT_INPUT_EVAL;
	CHSTATE(VALUE_WHITESPACE);
}

/**
Cares about \\ in values without quotes for PORTAGE_ESCAPES.
\n is ignored, known Portage escapes are handled
and everything else is put into buffer without the \\.
-> [RV] VALUE_WHITESPACE_PORTAGE
**/
void VarsReader::WHITESPACE_ESCAPE_PORTAGE() {
	switch(INPUT) {
		case '\n':
			break;
		case '\\':
			NEXT_INPUT_EVAL;
			if(INPUT == '\\') {
				NEXT_INPUT_EVAL;
			} else if((INPUT == '\r') || (INPUT == '\n')) {
				VALUE_APPEND(ESC_BS);
			}
			switch(INPUT) {
				case 'a':   VALUE_APPEND(ESC_A); break;
				case 'b':   VALUE_APPEND(ESC_B); break;
				case 'e':   VALUE_APPEND(ESC_E); break;
				case 'f':
				case 'n':   VALUE_APPEND(ESC_N); break;
				case 'r':   VALUE_APPEND(ESC_R); break;
				case 't':   VALUE_APPEND(ESC_T); break;
				case 'v':   VALUE_APPEND(ESC_V); break;
				case '\r':
				case '\n':  break;
				default:    VALUE_APPEND(INPUT);
			}
			break;
		default:
			VALUE_APPEND(INPUT);
	}
	NEXT_INPUT_EVAL;
	/* any amount of backslashes forbids portage to expand var */
	if(unlikely(INPUT == '$')) {
		VALUE_APPEND(INPUT);
		NEXT_INPUT_EVAL;
	}
	CHSTATE(VALUE_WHITESPACE_PORTAGE);
}

/**
Care about \\ in the noise. Everything is ignored (there is no buffer .. don't care!).
-> [RV] JUMP_NOISE
**/
void VarsReader::NOISE_ESCAPE() {
	NEXT_INPUT;
	CHSTATE(JUMP_NOISE);
}

/**
This little fellow was created to care for puny little single-quote-values (') in the noise surrounding us.
Read while INPUT is not in [\\'].
'\\' -> [RV] NOISE_SINGLE_QUOTE_ESCAPE | '\'' -> [RV] JUMP_NOISE
**/
void VarsReader::NOISE_SINGLE_QUOTE() {
	while(INPUT != '\'' && INPUT != '\\') {
		NEXT_INPUT;
	}
	switch(INPUT) {
		case '\'':  NEXT_INPUT;
		            CHSTATE(JUMP_NOISE);
		default:    NEXT_INPUT;
		            NEXT_INPUT;
	}
}

/**
Like his happy brother NOISE_SINGLE_QUOTE, this one is for double-quote-values (") in the noise surrounding us.
Read while INPUT is not in [\\"].
'\\' -> [RV] [RV] | '"' -> [RV] JUMP_NOISE
**/
void VarsReader::NOISE_DOUBLE_QUOTE() {
	while(INPUT != '"' && INPUT != '\\') {
		NEXT_INPUT;
	}
	switch(INPUT) {
		case '"':   NEXT_INPUT;
		            CHSTATE(JUMP_NOISE);
		default:    NEXT_INPUT;
		            NEXT_INPUT;
	}
}

void VarsReader::var_append(const char *beginning, size_t ref_key_length) {
	if(unlikely(ref_key_length == 0)) {
		return;
	}
	string varname(beginning, ref_key_length);
	// map<string, string>::const_iterator it; make check_includes happy
	const_iterator it;
	if(likely(section.empty()) ||
		((it = vars->find(varname + section)) == vars->end())) {
		it = vars->find(varname);
	}
	if(it != vars->end()) {
		value.append(it->second);
	}
}

/**
Try to resolve $... references to variables.
If we fail we recover from it. However, INPUT_EOF might be true at stop.
**/
void VarsReader::resolveReference() {
	const char *backup(x);
	NEXT_INPUT_APPEND_RET;
	bool brace(false);
	const char *beginning(x);
	if(INPUT == '{') {
		brace = true;
		NEXT_INPUT_APPEND_RET;
		beginning = x;
	}
	size_t ref_key_length(0);

	if(likely((parse_flags & PORTAGE_SECTIONS) == NONE)) {
		while(likely(isValidKeyCharacter(INPUT))) {
			++ref_key_length;
			NEXT_INPUT_OR_EOF;
			if(INPUT_EOF) {
				break;
			}
		}
	} else {
		if(unlikely(!brace)) {
			APPEND_RET;
		}
		while(likely(isValidKeyCharacter(INPUT) || (INPUT == ' ') || (INPUT == '\t') || (INPUT == ':'))) {
			++ref_key_length;
			NEXT_INPUT_APPEND_RET;
		}
	}

	if(brace) {
		if(unlikely(INPUT_EOF) || (INPUT != '}')) {
			APPEND_RET;
		}
		var_append(beginning, ref_key_length);
		NEXT_INPUT_OR_EOF;
	} else {
		var_append(beginning, ref_key_length);
	}
}

/**
Try to resolve %(...)s references to variables.
If we fail we recover from it. However, INPUT_EOF might be true at stop.
**/
void VarsReader::resolveSectionReference() {
	const char *backup(x);
	NEXT_INPUT_APPEND_RET;
	if(INPUT != '(') {
		APPEND_RET;
	}
	NEXT_INPUT_APPEND_RET;

	const char *beginning(x);
	size_t ref_key_length(0);
	while(likely(isValidKeyCharacter(INPUT) || (INPUT == ' ') || (INPUT == '\t') || (INPUT == ':'))) {
		++ref_key_length;
		NEXT_INPUT_APPEND_RET;
	}
	if(unlikely(INPUT != ')')) {
		NEXT_INPUT_APPEND_RET;
	}
	NEXT_INPUT_APPEND_RET;
	if(unlikely(INPUT != 's')) {
		APPEND_RET;
	}
	var_append(beginning, ref_key_length);
	NEXT_INPUT_OR_EOF;
}

// ************************** FSM ends here *******************************
// ************************************************************************

/**
Manage the mess around it.
Nothing to say .. it calls functions acording to the current state
and return if the state is STOP.
**/
bool VarsReader::runFsm() {
	while(STATE != NULLPTR) {
		(this->*STATE)();
	}
	return retstate;
}

/**
Init the FSM to parse a file.
New's buffer for file and reads it into this buffer.
@param filename This you want to read.
**/
void VarsReader::initFsm() {
	STATE = &VarsReader::JUMP_WHITESPACE;
	retstate = true;
	sourcecmd = false;
	x = filebuffer;
	key_len = 0;
	section.clear();
}

bool VarsReader::readmem(const char *buffer, const char *buffer_end, string *errtext) {
	filebuffer = buffer;
	if(buffer_end != NULLPTR) {
		filebuffer_end = buffer_end;
	} else {
		filebuffer_end = buffer + strlen(buffer);
	}
	if(likely(parse())) {
		return true;
	}
	if(errtext != NULLPTR) {
		*errtext = m_errtext;
	}
	return false;
}

bool VarsReader::read(const char *filename, string *errtext, bool noexist_ok, set<string> *sourced, bool nodir) {
	if((!nodir) && ((parse_flags & RECURSE) != NONE)) {
		string dir(filename);
		dir.append(1, '/');
		WordVec files;
		if(pushback_files(dir, &files, pushback_lines_exclude, 3)) {
			for(WordVec::const_iterator file(files.begin());
				likely(file != files.end()); ++file) {
				if(!read(file->c_str(), errtext, false, sourced, false)) {
					return false;
				}
			}
			return true;
		}
	}
	int fd(open(filename, O_RDONLY));
	if(fd == -1) {
		if(noexist_ok) {
			return true;
		}
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("cannot read file %s")) % filename;
		}
		return false;
	}
	struct stat st;
	if(fstat(fd, &st)) {
		close(fd);
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("cannot stat file %s")) % filename;
		}
		return false;
	}
	if(st.st_size == 0) {
		close(fd);
		return true;
	}
GCC_DIAG_OFF(sign-conversion)
	void *buffer = mmap(NULLPTR, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	filebuffer = static_cast<const char *>(buffer);
GCC_DIAG_ON(sign-conversion)
	close(fd);
GCC_DIAG_OFF(old-style-cast)
	if (buffer == MAP_FAILED) {
GCC_DIAG_ON(old-style-cast)
		if(errtext != NULLPTR) {
			*errtext = eix::format(_("cannot map file %s")) % filename;
		}
		return false;
	}
	file_name = filename;
	filebuffer_end = filebuffer + st.st_size;

	string truename(normalize_path(filename));
	bool topcall(sourced == NULLPTR);
	if(likely(topcall)) {
		sourced_files = new WordSet;
	} else {
		sourced_files = sourced;
		if(sourced->find(truename) != sourced->end()) {
			if(errtext != NULLPTR) {
				*errtext = eix::format(_("recursively sourced file %s")) % truename;
			}
			return false;
		}
	}

	sourced_files->insert(truename);
	bool ret = parse();
GCC_DIAG_OFF(sign-conversion)
	munmap(buffer, st.st_size);
GCC_DIAG_ON(sign-conversion)
	if(likely(topcall)) {
		delete sourced_files;
		sourced_files = NULLPTR;
	} else {
		sourced_files->erase(truename);
	}

	if(likely(ret)) {
		return true;
	}
	if(errtext != NULLPTR) {
		*errtext = m_errtext;
	}
	return false;
}

bool VarsReader::parse() {
	initFsm();
	if((parse_flags & APPEND_VALUES) == NONE) {
		return runFsm();
	}
	typedef vector<pair<string, string> > IncType;
	IncType incremental;
	// Save and clear incremental keys
	for(iterator i(vars->begin());
		i != vars->end(); ++i) {
		if((!i->second.empty()) &&
			isIncremental(i->first.c_str())) {
			incremental.push_back(*i);
			i->second.clear();
		}
	}
	bool ret = runFsm();
	// Prepend previous content for incremental keys
	for(IncType::const_iterator it(incremental.begin());
		it != incremental.end(); ++it) {
		iterator f(vars->find(it->first));
		if(f->second.empty()) {
			f->second = it->second;
		} else {
			f->second = (it->second) + ' ' + (f->second);
		}
	}
	return ret;
}

/**
Read file using a new instance of VarsReader with the same
settings (except for APPEND_VALUES),
adding variables and changed HAVE_READ to current instance.
**/
bool VarsReader::source(const string& filename, string *errtext) {
	VarsReader includefile((parse_flags & (~APPEND_VALUES)) | INTO_MAP);
	includefile.accumulatingKeys(incremental_keys);
	includefile.useMap(vars);
	includefile.setPrefix(source_prefix);
	string currprefix(source_prefix);
	if((parse_flags & ALLOW_SOURCE_VARNAME) == ALLOW_SOURCE_VARNAME) {
		if(vars != NULLPTR) {
			// Be careful to not declare the variable...
			iterator it(vars->find(source_prefix));
			if(it != vars->end())
				currprefix = it->second;
		}
	}
	bool rvalue(includefile.read((currprefix + filename).c_str(), errtext, false, sourced_files));
	parse_flags |= (includefile.parse_flags & HAVE_READ);
	return rvalue;
}

