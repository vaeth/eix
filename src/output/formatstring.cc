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
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
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

#include "formatstring.h"

#include <eixTk/ansicolor.h>
#include <eixTk/stringutils.h>

#include <iostream>
#include <cstdlib>
#include <map>

using namespace std;

string
get_escape(char *p)
{
	switch(*p) {
		case '\\': return string("\\");
		case 'n':  return string("\n");
		case 'r':  return string("\r");
		case 't':  return string("\t");
		case 'b':  return string("\b");
		case 'a':  return string("\a");
	}
	return string(p, 1);
}

void SpecialList::add(const char *pkg, const char *ver)
{
	pair<string, BasicVersion*> p;
	p.first = string(pkg);
	if(ver)
		p.second = new BasicVersion(ver);
	else
		p.second = NULL;
	insert(p);
}

/** Return true if pkg is special. If ver is non-NULL also *ver must match */
bool SpecialList::is_special(const Package &pkg, const BasicVersion *ver) const
{
	typedef multimap <string, BasicVersion*>::const_iterator CIT; 
	typedef pair<CIT, CIT> Range;
	Range pair=equal_range(pkg.category + "/" + pkg.name);
	if(pair.first == pair.second)// no match
		return false;
	if(!ver)	// do not care about versions
		return true;
	for(CIT it = pair.first ; it != pair.second; ++it )
	{
		BasicVersion *p = it->second;
		if(p)
		{
			if(*p == *ver)
				return true;
		}
	}
	return false;
}

void
PrintFormat::recPrint(void *entity, PrintProperty print_property, GetProperty get_property, Node *root)
{
	for(;
		root != NULL;
		root = root->next)
	{
		switch(root->type) {
			case Node::TEXT: /* text!! */
				cout << ((Text*)root)->text;
				break;
			case Node::VARIABLE:
				try {
					print_property(this, entity, ((Property*)root)->name);
				}
				catch(ExBasic e) {
					cerr << e << endl;
				}
				break;
			case Node::IF:
				{
					ConditionBlock *ief = (ConditionBlock*)root;
					bool ok = false;
					try {
						ok = get_property(this, entity, ief->variable.name) == ief->text.text;
					}
					catch(ExBasic e) {
						cerr << e << endl;
					}
					ok = ief->negation ? !ok : ok;
					if(ok && ief->if_true) {
						recPrint(entity, print_property, get_property, ief->if_true);
					}
					else if(!ok && ief->if_false) {
						recPrint(entity, print_property, get_property, ief->if_false);
					}
				}
				break;
		}
	}
	return; /* never reached */
}


FormatParser::ParserState
FormatParser::state_START()
{
	switch(*band_position++) {
		case '\0': return STOP;
		case '{':  return IF;
		case '<':  return PROPERTY;
		case '(':  return COLOR;
	}
	--band_position;
	return TEXT;
}

FormatParser::ParserState
FormatParser::state_TEXT()
{
	string textbuffer;
	while(*band_position && (strchr("<{(", *band_position ) == NULL)) {
		if(*band_position == '\\') {
			textbuffer.append(get_escape(++band_position));
		}
		else {
			textbuffer.append(band_position, 1);
		}
		++band_position;
	}
	keller.push(new Text(textbuffer));
	return START;
}

FormatParser::ParserState
FormatParser::state_COLOR()
{
	char *q = strchr(band_position, ')');
	if(q == NULL) {
		last_error = "'(' without closing ')'";
		return ERROR;
	}
	if(enable_colors) {
		try {
			keller.push(new Text(AnsiColor(string(band_position, q - band_position)).asString()));
		}
		catch(ExBasic e) {
			last_error = "Error while parsing color: " + e.getMessage();
			return ERROR;
		}
	}
	band_position = q + 1;
	return START;
}

FormatParser::ParserState
FormatParser::state_PROPERTY()
{
	char *q = strchr(band_position, '>');
	if(q == NULL) {
		last_error = "'<' without closing '>'";
		return ERROR;
	}
	keller.push(new Property(string(band_position, q - band_position)));
	band_position = q + 1;
	return START;
}

char *
seek_character(char *fmt)
{
	while(*fmt && isspace(*fmt)) {
		++fmt;
	}
	return fmt;
}

FormatParser::ParserState
FormatParser::state_IF()
{
	if(strncmp(band_position, "else}", 5) == 0) {
		band_position += 5;
		return ELSE;
	}
	if(strncmp(band_position, "}", 1) == 0) {
		band_position += 1;
		return FI;
	}

	ConditionBlock *n = new ConditionBlock;
	keller.push(n);

	/* Look for negation */
	band_position = seek_character(band_position);
	if(*band_position == '\0' || *band_position == '}') {
		last_error = "Ran into end-of-string or '}' while looking for possible negation-mark (!) in condition.";
		return ERROR;
	}
	if(*band_position == '!') {
		n->negation = true;
		++band_position;
	}
	else {
		n->negation = false;
	}

	band_position = seek_character(band_position);
	if(*band_position == '\0' || *band_position == '}') {
		last_error = "Ran into end-of-string or '}' while looking for property-name in condition.";
		return ERROR;
	}
	unsigned int i = 0;
	while(*band_position && strchr(" \t\n\r}", *band_position) == NULL) {
		++band_position;
		++i;
	}
	if(i == 0 || !*band_position) {
		last_error = "Ran into end-of-string while reading property-name.";
		return ERROR;
	}
	n->variable = Property(string(band_position - i, i));

	band_position = seek_character(band_position);
	if(*band_position == '}') {
		n->text = Text("");
		n->negation = !n->negation;
		++band_position;
		return START;
	}
	/* This MUST be a '=' */
	if(*band_position != '=') {
		last_error = "Unknown operator in if-construct.";
		return ERROR;
	}
	++band_position;

	band_position = seek_character(band_position);
	if(!*band_position) {
		last_error = "Run into end-of-string while looking for right-hand of condition.";
		return ERROR;
	}

	string textbuffer;
	static const char single_quote = '\'', double_quote = '"', plain = ' ';
	char parse_modus = plain;

	if(*band_position == double_quote) {
		parse_modus = double_quote;
		++band_position;
	}
	else if(*band_position == single_quote) {
		parse_modus = single_quote;
		++band_position;
	}

	while(*band_position) {
		if((parse_modus != plain && *band_position == parse_modus) || strchr(" \t\n\r}", *band_position) != NULL) {
			break;
		}
		if(*band_position == '\\' && parse_modus != single_quote) {
			textbuffer.append(get_escape(++band_position));
		}
		else {
			textbuffer.append(band_position, 1);
		}
		++band_position;
	}
	n->text = Text(textbuffer);
	++band_position;
	band_position = seek_character(band_position);
	++band_position;
	return START;
}

FormatParser::ParserState
FormatParser::state_ELSE()
{
	Node *p = NULL, *q = NULL;
	if(keller.size() == 0) {
		return START;
	}
	p = keller.top();
	keller.pop();
	while(p->type != Node::IF || ((ConditionBlock*)p)->final == true) {
		p->next = q;
		q = p;
		if(keller.size() == 0) {
			last_error = "Found ELSE without IF.";
			return ERROR;
		}
		p = keller.top();
		keller.pop();
	}
	if(q == NULL) {
		q = new Text("");
	}
	((ConditionBlock*)p)->if_true = q;
	keller.push(p);
	return START;
}

FormatParser::ParserState
FormatParser::state_FI()
{
	Node *p = NULL, *q = NULL;
	if(keller.size() == 0) {
		return START;
	}
	p = keller.top();
	keller.pop();
	while(p->type != Node::IF || ((ConditionBlock*)p)->final == true) {
		p->next = q;
		q = p;
		if(keller.size() == 0) {
			last_error = "Found FI without IF.";
			return ERROR;
		}
		p = keller.top();
		keller.pop();
	}
	if(((ConditionBlock*)p)->if_true == NULL) {
		((ConditionBlock*)p)->if_true = q;
	}
	else {
		((ConditionBlock*)p)->if_false = q;
	}
	((ConditionBlock*)p)->final = true;
	keller.push(p);
	return START;
}

Node *
FormatParser::start(const char *fmt, bool colors) throw(ExBasic)
{
	/* Initialize machine */
	enable_colors = colors;
	last_error.clear();
	state = START;
	band = fmt;
	band_position = (char *) fmt;
	/* Run machine */
	while(state != STOP && state != ERROR) {
		switch(state) {
			case START:    state = state_START(); break;
			case TEXT:     state = state_TEXT(); break;
			case COLOR:    state = state_COLOR(); break;
			case PROPERTY: state = state_PROPERTY(); break;
			case IF:       state = state_IF(); break;
			case ELSE:     state = state_ELSE(); break;
			case FI:       state = state_FI(); break;
			case ERROR:    throw(ExBasic("Bad state: ERROR."));
			case STOP:     throw(ExBasic("Bad state: STOP."));
			default:       throw(ExBasic("Bad state: undefined."));
		}
	}
	/* Check if the machine went into ERROR-state. */
	if(state == ERROR) {
		/* Clean stacks. */
		while(keller.size() > 0) {
			delete keller.top();
			keller.pop();
		}
		int line = 0, column = 0;
		getPosition(&line, &column);
		throw(ExBasic("Line %i, column %i: %s",
					line, column,
					last_error.size() > 0 ? last_error.c_str() : "Check your syntax!"));
	}
	/* Pop elements and form a single linked list. */
	Node *p = NULL, *q = NULL;
	while(keller.size() != 0) {
		p = keller.top();
		keller.pop();
		p->next = q;
		q = p;
	}
	/* Return root-node. */
	return p;
}
