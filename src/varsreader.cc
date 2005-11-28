/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "varsreader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/** Current input for FSM */
#define INPUT (*(x))
/** Move to next input and check for end of buffer. */
#define NEXT_INPUT do { if(++x == filebuffer_end) CHSTATE(STOP); } while(0)
#define PREV_INPUT (--(x))
/** Switch to different state */
#define CHSTATE(z) do { \
	STATE = (state_ ## z); return; } while(0)
/** Check value-buffer for overrun and push c into the current value-buffer . */
#define VALUE_APPEND(c) do { value.append(&(c), 1); } while(0)
/** Reset value pointer */
#define VALUE_CLEAR value.clear()

#include <fnmatch.h>

bool VarsReader::isIncremental(const char *key)
{
	if(incremental_keys == NULL)
		return false;
	while(*incremental_keys != NULL) {
		if(fnmatch(*incremental_keys++, key, 0) == 0)
			return true;
	}
	return false;
}

/** Assign key=value and reset key- and value-buffer. */
#define ASSIGN_KEY_VALUE  do { \
	if( (parse_flags & ONLY_KEYWORDS) ) { \
		if(strncmp("KEYWORDS=", key_begin, 9) == 0) { \
			(*vars)[string(key_begin, key_len)] = value; \
			CHSTATE(STOP); \
		} \
		break; \
	} \
	else { \
		(*vars)[string(key_begin, key_len)] = value; \
	} \
} while(0)

const short 
VarsReader::NONE          = 0x00, /**< Flag: No flags set; normal behavior. */	
	VarsReader::ONLY_KEYWORDS = 0x01, /**< Flag: Only read "KEYWORDS" once, than exit the parser. */
	VarsReader::SUBST_VARS    = 0x02, /**< Flag: Allow references to variable in declarations
							of a variable. i.e.  USE="${USE} -kde" */
	VarsReader::INTO_MAP      = 0x04, /**< Flag: Init but don't parse .. you must first supply
							a pointer to map<string,string> with useMap(...) */
	VarsReader::APPEND_VALUES = 0x08; /**< Flag: appended new values rather then replace the old value. */

/*************************************************************************/
/********************** FSM states begin here ****************************/

/** Eats most of the noise between declarations.
 * Read until '\n' '#' '\\' '\'' '"'
 * '\\' -> [RV] NOISE_ESCAPE | '\'' -> [RV] NOISE_SINGLE_QUOTE | '"' -> [RV] NOISE_DOUBLE_QUOTE | '#' -> [RV] JUMP_COMMENT
 * '\n' -> [RV] (and check if we are at EOF, EOF's only occur after a newline) -> JUMP_WHITESPACE */
void VarsReader::JUMP_NOISE()
{
	while(INPUT != '#' && INPUT != '\n' && INPUT != '\'' && INPUT != '"' && INPUT != '\\') NEXT_INPUT;
	switch(INPUT) {
		case '#':   NEXT_INPUT; CHSTATE(JUMP_COMMENT);
		case '\\':  NEXT_INPUT; CHSTATE(NOISE_ESCAPE);
		case '"':   NEXT_INPUT; CHSTATE(NOISE_DOUBLE_QUOTE);
		case '\'':  NEXT_INPUT; CHSTATE(NOISE_SINGLE_QUOTE);
	}
	NEXT_INPUT;
	CHSTATE(JUMP_WHITESPACE);
}

/** Jumps comments.
 * Read until the next '\n' comes in. Then move to JUMP_NOISE. */
void VarsReader::JUMP_COMMENT()
{
	while(INPUT != '\n') NEXT_INPUT;
	CHSTATE(JUMP_NOISE);
}

/** Jump whitespaces and tabs at begining of line.
 * Read while ' ' || '\t'
 * '#' -> [RV] JUMP_COMMENT | [A-Z_] -> (reset key) FIND_ASSIGNMENT | -> JUMP_NOISE
 * @see isValidKeyCharacter */
void VarsReader::JUMP_WHITESPACE()
{
	while(INPUT == '\t' || INPUT == ' ') NEXT_INPUT;
	switch(INPUT) {
		case '#':  NEXT_INPUT; CHSTATE(JUMP_COMMENT);
		default:   if(isValidKeyCharacter(INPUT)) {
					   key_begin = x;
					   CHSTATE(FIND_ASSIGNMENT);
				   }
				   CHSTATE(JUMP_NOISE);
	}
}

/** Find key .. if there is one :).
 * Keys are marked with a '=' at the end ('=' is not part of the key). This state always gets the
 * first character of the keyword as current INPUT, thus we don't need to check if key_len is >= 1,
 * is always >= 1.
 * Read while not ('=' || '#') and [A-Z_]
 * '=' -> EVAL_VALUE | '#' -> [RV] JUMP_COMMENT | -> JUMP_NOISE */
void VarsReader::FIND_ASSIGNMENT()
{
	key_len = 0;
	while(isValidKeyCharacter(INPUT)) { NEXT_INPUT; ++key_len;}
	switch(INPUT) {
		case '=':  CHSTATE(EVAL_VALUE);
		case '#':  NEXT_INPUT; CHSTATE(JUMP_COMMENT);
		default:   CHSTATE(JUMP_NOISE);
	}
}

/** Looks if the following input is a valid value-part.
 * ['"\^#\n\t ] is allowed.
 * [\n\t ] -> JUMP_NOISE | '#' -> [RV] JUMP_COMMENT
 * '\'' -> [RV] VALUE_SINGLE_QUOTE | '"' -> [RV] VALUE_DOUBLE_QUOTE | '\\' -> [RV] WHITESPACE_ESCAPE | -> VALUE_WHITESPACE
 * If we have the begin of a valid value we are reseting the value-buffer. */
void VarsReader::EVAL_VALUE()
{
	NEXT_INPUT;
	VALUE_CLEAR; 
	switch(INPUT) {
		case '"':   NEXT_INPUT; CHSTATE(VALUE_DOUBLE_QUOTE);
		case '\'':  NEXT_INPUT; CHSTATE(VALUE_SINGLE_QUOTE);
		case '\t':
		case '\n':
		case ' ':   CHSTATE(JUMP_NOISE);
		case '#':   NEXT_INPUT; CHSTATE(JUMP_COMMENT);
		case '\\':  NEXT_INPUT; CHSTATE(WHITESPACE_ESCAPE);
		default:    CHSTATE(VALUE_WHITESPACE);
	}
}

/** Reads a value enclosed in single quotes (').
 * Copy INPUT into value-buffer while INPUT is not in ['\\]. If the value ends, ASSIGN_KEY_VALUE is
 * called.
 * '\\' -> [RV] SINGLE_QUOTE_ESCAPE | '\'' -> [RV] JUMP_NOISE */
void VarsReader::VALUE_SINGLE_QUOTE()
{
	while(INPUT != '\'') {
		VALUE_APPEND(INPUT);
		NEXT_INPUT;
	}
	ASSIGN_KEY_VALUE; NEXT_INPUT; CHSTATE(JUMP_NOISE);
}

/** Read value enclosed in double-quotes (").
 * Copy INPUT into value-buffer while INPUT is not in ["\\]. If the value ends, ASSIGN_KEY_VALUE is
 * called.
 * '\\' -> [RV] DOUBLE_QUOTE_ESCAPE | '"' -> [RV] JUMP_NOISE */
void VarsReader::VALUE_DOUBLE_QUOTE()
{
	while(INPUT != '"' && INPUT != '\\') {
		if(INPUT == '$' && (parse_flags & SUBST_VARS))
		{
			NEXT_INPUT;
			resolveReference();
			continue;
		}
		VALUE_APPEND(INPUT);
		NEXT_INPUT;
	}
	switch(INPUT) {
		case '"':  ASSIGN_KEY_VALUE; NEXT_INPUT; CHSTATE(JUMP_NOISE);
		default:   NEXT_INPUT; CHSTATE(DOUBLE_QUOTE_ESCAPE);
	}
}

/** Read value not inclosed in any quotes.
 * Thus there are no spaces and tabs allowed. Everything must be escaped.
 * Move INPUT into buffer while it's not in [ \t\n\\]. If the value ends we call ASSIGN_KEY_VALUE.
 * [\n\t ] -> (ASSIGN_KEY_VALUE) JUMP_NOISE | '\\' -> WHITESPACE_ESCAPE */
void VarsReader::VALUE_WHITESPACE()
{
	while(INPUT != '\n' && INPUT != '\\' && INPUT != '\t' && INPUT != ' ') {
		if(INPUT == '$' && (parse_flags & SUBST_VARS))
		{
			NEXT_INPUT;
			resolveReference();
			continue;
		}
		VALUE_APPEND(INPUT);
		NEXT_INPUT;
	}
	switch(INPUT) {
		case '\t':
		case '\n':
		case ' ':  ASSIGN_KEY_VALUE; CHSTATE(JUMP_NOISE);
		default:   NEXT_INPUT; CHSTATE(WHITESPACE_ESCAPE);
	}
}

/** Cares about \\ in double-quote values. \n is ignored, everything else is put into buffer without
 * the \\.
 * -> [RV] VALUE_DOUBLE_QUOTE */
void VarsReader::DOUBLE_QUOTE_ESCAPE()
{
	switch(INPUT) {
		case '\n':  break;
		default:    VALUE_APPEND(INPUT);
	}
	NEXT_INPUT;
	CHSTATE(VALUE_DOUBLE_QUOTE);
}

/** Cares about \\ in values without quotes. \n is ignored, everything else is put into buffer without
 * the \\.
 * -> [RV] VALUE_WHITESPACE */
void VarsReader::WHITESPACE_ESCAPE()
{
	switch(INPUT) {
		case '\n':  break;
		default:    VALUE_APPEND(INPUT);
	}
	NEXT_INPUT;
	CHSTATE(VALUE_WHITESPACE);
}

/** Cares about \\ in the noise. Everything is ignored (there is no buffer .. don't care!).
 * -> [RV] JUMP_NOISE */
void VarsReader::NOISE_ESCAPE()
{
	NEXT_INPUT;
	CHSTATE(JUMP_NOISE);
}

/** This little fellow was created to care for puny little single-quote-values (') in the noise surrounding us.
 * Read while INPUT is not in [\\'].
 * '\\' -> [RV] NOISE_SINGLE_QUOTE_ESCAPE | '\'' -> [RV] JUMP_NOISE */
void VarsReader::NOISE_SINGLE_QUOTE()
{
	while(INPUT != '\'' && INPUT != '\\') {
		NEXT_INPUT;
	}
	switch(INPUT) {
		case '\'':  NEXT_INPUT; CHSTATE(JUMP_NOISE);
		default:    NEXT_INPUT; NEXT_INPUT;
	}
}

/** Like his happy brother NOISE_SINGLE_QUOTE, this one is for double-quote-values (") in the noise surrounding us.
 * Read while INPUT is not in [\\"].
 * '\\' -> [RV] [RV] | '"' -> [RV] JUMP_NOISE */
void VarsReader::NOISE_DOUBLE_QUOTE()
{
	while(INPUT != '"' && INPUT != '\\') {
		NEXT_INPUT;
	}
	switch(INPUT) {
		case '"':   NEXT_INPUT; CHSTATE(JUMP_NOISE);
		default:    NEXT_INPUT; NEXT_INPUT; 
	}
}

/** Try to resolve references to variables.
 * If we fail we recover from it and */
void VarsReader::resolveReference()
{
	char *begin = x;
	if(INPUT == '{')
		NEXT_INPUT;
	unsigned int ref_key_lenght = 0;

	while(isValidKeyCharacter(INPUT)) {
		NEXT_INPUT;
		++ref_key_lenght;
	}

	if(*begin == '{') {
		ASSERT(INPUT == '}', "%s: Ran into '%c' while looking for '}' after '%.*s'.",
				file_name, INPUT, ref_key_lenght, begin);

		value.append((*vars)[string(begin + sizeof(char), ref_key_lenght)]);
		NEXT_INPUT;
	}
	else {
		value.append((*vars)[string(begin, ref_key_lenght)]);
	}
	return;
}

/*************************** FSM ends here *******************************/
/*************************************************************************/

/** Manages the mess around it.
 * Nothing to say .. it calls functions acording to the current state and returns if the state is
 * STOP. */
void VarsReader::runFsm()
{
	for(;;) {
		switch(STATE) {
			case state_JUMP_WHITESPACE: JUMP_WHITESPACE(); break;
			case state_JUMP_NOISE: JUMP_NOISE(); break;
			case state_JUMP_COMMENT: JUMP_COMMENT(); break;
			case state_FIND_ASSIGNMENT: FIND_ASSIGNMENT(); break;
			case state_EVAL_VALUE: EVAL_VALUE(); break;
			case state_VALUE_DOUBLE_QUOTE: VALUE_DOUBLE_QUOTE(); break;
			case state_NOISE_DOUBLE_QUOTE: NOISE_DOUBLE_QUOTE(); break;
			case state_NOISE_ESCAPE: NOISE_ESCAPE(); break;
			case state_NOISE_SINGLE_QUOTE: NOISE_SINGLE_QUOTE(); break;
			case state_STOP: return;
			case state_VALUE_WHITESPACE: VALUE_WHITESPACE(); break;
			case state_VALUE_SINGLE_QUOTE: VALUE_SINGLE_QUOTE(); break;
			case state_DOUBLE_QUOTE_ESCAPE: DOUBLE_QUOTE_ESCAPE(); break;
			case state_WHITESPACE_ESCAPE: WHITESPACE_ESCAPE(); break;
		}
	}
	return;
}

/** Init the FSM to parse a file.
 * New's buffer for file and reads it into this buffer.
 * @param filename This you want to read.
 * @warn You need to call deinit() if you called this function. */
void VarsReader::initFsm()
{
	STATE = state_JUMP_WHITESPACE;
	x = filebuffer;
	key_len = 0;
}

bool VarsReader::read(const char *filename)
{
	struct stat st;
	int fd = 0;
	fd = open(filename, O_RDONLY);
	if(fd == -1)
		return false;
	if(fstat(fd, &st)) {
		close(fd);
		return false;
	}
	if(st.st_size == 0) {
		close(fd);
		return true;
	}
	filebuffer = (char *) mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	ASSERT(filebuffer != MAP_FAILED,
			"Can't map file %s.", filename);
	filebuffer_end = filebuffer + st.st_size;
	close(fd);

	initFsm();
	if(parse_flags & APPEND_VALUES)
	{
		// parse file into separate map and append values which keys are in
		// incremental_keys .. other keys are just replaced.
		map<string,string> *old_values = vars;
		map<string,string> new_values;
		vars = &new_values;
		runFsm();
		for(map<string,string>::iterator i = new_values.begin();
			i != new_values.end();
			++i)
		{
			if((*old_values)[i->first].size() > 0 
			   && isIncremental(i->first.c_str()))
			{
				(*old_values)[i->first].append(" " + i->second);
			}
			else
			{
				(*old_values)[i->first] = i->second;
			}
		}
		vars = old_values;
	}
	else
	{
		runFsm();
	}
	munmap(filebuffer, st.st_size);
	return true;
}
