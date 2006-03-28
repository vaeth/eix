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
	for(int i = 1; i<argc ; i++)
	{
		if(seen_escape)
		{
			push_back(Parameter(0, argv[i]));
			continue;
		}

		char *ptr = argv[i];
		if(*ptr == '-')
			switch(*++ptr)
			{
				case 0:
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
							push_back(Parameter(lookup_longopt(ptr, opt_table), NULL));
							continue;
					}
				default:
					/* some shortopts */
					int c = 0;
					while(ptr[c] != '\0')
						push_back(Parameter(lookup_shortopt(ptr[c++], opt_table), NULL));
					continue;
			}
		push_back(Parameter(0, ptr));
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

		Option *c = lookup_option(it->opt, opt_table);
		if(c == NULL || c->type == Option::NONE)
		{
			++it;
			continue;
		}

		switch(c->type)
		{
			case Option::BOOLEAN_F:
			case Option::BOOLEAN_T:
			case Option::BOOLEAN:
				if(c->boolean != NULL)
				{
					if(c->type == Option::BOOLEAN_T)
						*c->boolean = true;
					else if(c->type == Option::BOOLEAN_F)
						*c->boolean = false;
					else
						*c->boolean = ! *c->boolean;
					it = erase(it);
				}
				continue;
			case Option::INTEGER:
				if(c->integer != NULL)
				{
					++*c->integer;
					it = erase(it);
				}
				continue;
			case Option::STRING:
				if(c->str != NULL)
				{
					it = erase(it);
					__ASSERT(it != end(), "Missing parameter to --%s\n", c->longopt);
					__ASSERT(it->type == Parameter::ARGUMENT, "Missing parameter to --%s\n", c->longopt);
					*c->str = it->arg;
					it = erase(it);
				}
				continue;
			case Option::NONE:
			default:
				++it;
				continue;
		}
	}
}
