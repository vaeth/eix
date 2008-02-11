// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "varsreader.h"

#include <eixTk/exceptions.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>

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

using namespace std;

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
	VarsReader::HAVE_READ,
	VarsReader::ONLY_HAVE_READ;


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

/** Assign key=value or source file.
    Return true if a stop is required due to ONLY_KEYWORDS_SLOT */
bool VarsReader::assign_key_value()
{
	if(sourcecmd)
	{
		sourcecmd=false;
		source(value);
		return ((parse_flags & ONLY_HAVE_READ) == ONLY_HAVE_READ);
	}
	if( (parse_flags & ONLY_KEYWORDS_SLOT) )
	{
		if(strncmp("KEYWORDS=", key_begin, 9) == 0)
		{
			(*vars)[string(key_begin, key_len)] = value;
			parse_flags |= KEYWORDS_READ;
			return (parse_flags & SLOT_READ);
		}
		else if(strncmp("SLOT=", key_begin, 5) == 0)
		{
			(*vars)[string(key_begin, key_len)] = value;
			parse_flags |= SLOT_READ;
			return (parse_flags & KEYWORDS_READ);
		}
		return false;
	}
	(*vars)[string(key_begin, key_len)] = value;
	return false;
}

#define ASSIGN_KEY_VALUE  do { \
	if (assign_key_value()) { \
		CHSTATE(STOP); \
	} \
} while(0)

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
		default:    break;
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
 * '#' -> [RV] JUMP_COMMENT | [A-Z_] -> (reset key) FIND_ASSIGNMENT |
 # source or . -> EVAL_VALUE with sourcecmd=true | -> JUMP_NOISE
 * @see isValidKeyCharacter */
void VarsReader::JUMP_WHITESPACE()
{
	sourcecmd=false;
	while(INPUT == '\t' || INPUT == ' ') NEXT_INPUT;
	switch(INPUT) {
		case '#':  NEXT_INPUT;
			   while(INPUT != '\n') NEXT_INPUT;
			   CHSTATE(JUMP_WHITESPACE);
			   break;
		case 's':  {
				int i=0;
				const char *begin = x;
				while(isalpha(INPUT)) { NEXT_INPUT; ++i; }
				if((i!=6) || strncmp("source", begin, 6) != 0)
				{
					CHSTATE(JUMP_NOISE);
					break;
				}
			   }
			   --x;
		case '.':  NEXT_INPUT;
			   if((parse_flags & ALLOW_SOURCE) &&
			     (INPUT == '\t' || INPUT == ' '))
			   {
				sourcecmd=true;
				CHSTATE(EVAL_VALUE);
			   }
			   else
				CHSTATE(JUMP_NOISE);
			   break;
		default:   if(isValidKeyCharacterStart(INPUT)) {
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
 * Read while not ('=' || '#') and [A-Z_0-9]
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
	unsigned int ref_key_length = 0;

	while(isValidKeyCharacter(INPUT)) {
		NEXT_INPUT;
		++ref_key_length;
	}

	if(*begin == '{') {
		if(INPUT == '}') {
			value.append((*vars)[string(begin + sizeof(char), ref_key_length)]);
		}
		/** For some reseon, this fprintf crashes:
		else
			fprintf(stderr, "%s: Ran into '%c' while looking for '}' after '%.*s'.",
				file_name, INPUT, ref_key_length, begin);
		*/
		NEXT_INPUT;
	}
	else {
		value.append((*vars)[string(begin, ref_key_length)]);
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
			default: break;
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
	sourcecmd = false;
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
	filebuffer = static_cast<char *>(mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0));
	if (filebuffer == MAP_FAILED) {
		throw ExBasic("Can't map file %r") % filename;
	}
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

/** Read file using a new instance of VarsReader with the same
    settings (except for APPEND_VALUES),
    adding variables and changed HAVE_READ to current instance. */
bool VarsReader::source(const string &filename)
{
	static int depth=0;
	++depth;
	if (depth == 100) {
		throw ExBasic("Nesting level too deep when reading %r") % filename;
	}
	VarsReader includefile((parse_flags & (~APPEND_VALUES)) | INTO_MAP);
	includefile.accumulatingKeys(incremental_keys);
	includefile.useMap(vars);
	includefile.setPrefix(source_prefix);
	string currprefix;
	if((parse_flags & ALLOW_SOURCE_VARNAME) == ALLOW_SOURCE_VARNAME) {
		if(vars) {
			// Be careful to not declare the variable...
			map<string,string>::iterator it = vars->find(source_prefix);
			if(it != vars->end())
				currprefix = it->second;
		}
	}
	int rvalue=includefile.read((currprefix + filename).c_str());
	parse_flags |= (includefile.parse_flags & HAVE_READ);
	depth--;
	return rvalue;
}

