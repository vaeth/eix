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

#include "argsreader.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

using namespace std;

#define __ASSERT(x, ...) do { \
	if(!(x)) { \
		fprintf(stderr, __VA_ARGS__); exit(1); \
	}  \
} while(0)

ArgumentReader::ArgumentReader(int argc, char **argv, struct Option opt_table[])
{
	bool seen_escape = false;
	name = argv[0];
	unsigned int paramarg_remain = 0;
	for(int i = 1; i<argc ; i++)
	{
		if(seen_escape || paramarg_remain)
		{
			push_back(Parameter(argv[i]));
			if(paramarg_remain)
				paramarg_remain--;
			continue;
		}

		char *ptr = argv[i];
		if(*ptr != '-')
		{
			push_back(Parameter(ptr));
			continue;
		}
		int opt;
		switch(*++ptr)
		{
			case 0:
				push_back(Parameter("-"));
				continue;
			case '-':
				/* something that begins with -- */
				switch(*++ptr)
				{
					case 0:
						/* a lonely -- */
						seen_escape = true;
						continue;
					default:
						/* some longopt */
						opt = lookup_longopt(ptr, opt_table);
						push_back(Parameter(opt));
						paramarg_remain = numargs(opt, opt_table);
				}
				break;
			default:
				/* some shortopts */
				for(char c = *ptr; c != '\0' ; c = *++ptr)
				{
					if(paramarg_remain)
					{
						push_back(Parameter(ptr));
						paramarg_remain--;
						break;
					}
					opt = lookup_shortopt(c, opt_table);
					push_back(Parameter(opt));
					paramarg_remain = numargs(opt, opt_table);
				}
		}
	}

	foldAndRemove(opt_table);
}

Option *
ArgumentReader::lookup_option(const int opt, struct Option *opt_table)
{
	while( (opt_table->longopt || opt_table->shortopt) )
	{
		if( (opt_table->shortopt) && (opt_table->shortopt == opt) )
			return opt_table;
		++opt_table;
	}
	return NULL;
}

unsigned int
ArgumentReader::numargs(const int opt, struct Option *opt_table)
{
	Option *c = lookup_option(opt, opt_table);
	if(c == NULL)
		return 0;
	switch(c->type)
	{
		case Option::STRING:
		case Option::STRING_OPTIONAL:
		case Option::STRINGLIST:
		case Option::STRINGLIST_OPTIONAL:
		case Option::KEEP_STRING:
		case Option::KEEP_STRING_OPTIONAL:
			return 1;
		case Option::PAIR:
		case Option::PAIR_OPTIONAL:
		case Option::PAIRLIST:
		case Option::PAIRLIST_OPTIONAL:
		case Option::KEEP_PAIR:
		case Option::KEEP_PAIR_OPTIONAL:
			return 2;
		default:
			break;
	}
	return 0;
}

/** Return shortopt for longopt stored in opt.
 * @param long_opt longopt that should be resolved.
 * @return shortopt for given longopt */
int
ArgumentReader::lookup_longopt(const char *long_opt, struct Option *opt_table)
{
	while( (opt_table->longopt || opt_table->shortopt) )
	{
		if( (opt_table->longopt) && strcmp(opt_table->longopt, long_opt) == 0 )
			return opt_table->shortopt;
		++opt_table;
	}
	fprintf(stderr, "Unknown option --%s\n", long_opt);
	exit(-1);
}

/** Check if short_opt is a known option.
 * @param long_opt longopt that should be resolved.
 * @return shortopt for given longopt */
int
ArgumentReader::lookup_shortopt(const char short_opt, struct Option *opt_table)
{
	while( (opt_table->longopt || opt_table->shortopt) )
	{
		if( (opt_table->shortopt) && (opt_table->shortopt == short_opt) )
			return short_opt;
		++opt_table;
	}
	fprintf(stderr, "Unknown option -%c\n", short_opt);
	exit(-1);
}

void
ArgumentReader::foldAndRemove(struct Option *opt_table)
{
	ArgumentReader::iterator it = begin();
	while(it != end())
	{
		if(it->type == Parameter::ARGUMENT)
		{
			++it;
			continue;
		}

		Option *c = lookup_option(it->m_option, opt_table);
		if(c == NULL)
		{
			++it;
			continue;
		}

		const char *remember = "";
		const char *second = "";
		bool keep = false;
		bool optional = false;
		switch(c->type)
		{
			case Option::BOOLEAN_F:
			case Option::BOOLEAN_T:
			case Option::BOOLEAN:
					if(c->type == Option::BOOLEAN_T)
						*c->boolean = true;
					else if(c->type == Option::BOOLEAN_F)
						*c->boolean = false;
					else
						*c->boolean = ! *c->boolean;
					it = erase(it);
					break;
			case Option::INTEGER:
					++*c->integer;
					it = erase(it);
					break;
			case Option::KEEP_STRING_OPTIONAL:
			case Option::KEEP_PAIR_OPTIONAL:
					optional = true;
			case Option::KEEP_STRING:
			case Option::KEEP_PAIR:
					keep = true;
			case Option::STRING:
			case Option::PAIR:
			case Option::STRINGLIST:
			case Option::PAIRLIST:
			case Option::STRING_OPTIONAL:
			case Option::PAIR_OPTIONAL:
			case Option::STRINGLIST_OPTIONAL:
			case Option::PAIRLIST_OPTIONAL:
					switch(c->type) {
						case Option::STRING_OPTIONAL:
						case Option::STRINGLIST_OPTIONAL:
						case Option::PAIR_OPTIONAL:
						case Option::PAIRLIST_OPTIONAL:
							optional = true;
						default:
							break;
					}
					if(keep)
						++it;
					else
						it = erase(it);
					if(it == end())
						__ASSERT(optional, "Missing parameter to --%s\n", c->longopt);
					else {
						remember = it->m_argument;
						if(keep)
							++it;
						else
							it = erase(it);
					}
					if((c->type == Option::KEEP_STRING) || (c->type == Option::KEEP_STRING_OPTIONAL))
						break;
					if((c->type == Option::STRINGLIST) || (c->type == Option::STRINGLIST_OPTIONAL))
					{
						c->strlist->push_back(remember);
						break;
					}
					if((c->type == Option::STRING) || (c->type == Option::STRING_OPTIONAL))
					{
						*(c->str) = remember;
						break;
					}
					if(it == end())
						__ASSERT(optional, "Missing second parameter to --%s\n", c->longopt);
					else {
						second = it->m_argument;
						if(keep)
							++it;
						else
							it = erase(it);
					}
					if(keep)
					//((c->type == Option::KEEP_PAIR) || (c->type == Option::KEEP_PAIR_OPTIONAL))
						break;
					if((c->type == Option::PAIR) || (c->type == Option::PAIR_OPTIONAL))
					{
						*((c->pr).first)  = remember;
						*((c->pr).second) = second;
						break;
					}
					//if((c->type == Option::PAIRLIST) || (c->type == Option::PAIRLIST_OPTIONAL))
						c->prlist->push_back(ArgPair(remember, second));
					break;
			default: // KEEP
					++it;
		}
	}
}
