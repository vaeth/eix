// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include "argsreader.h"

#include <eixTk/likely.h>
#include <eixTk/i18n.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

ArgumentReader::ArgumentReader(int argc, char **argv, struct Option opt_table[])
{
	bool seen_escape(false);
	name = argv[0];
	unsigned int paramarg_remain(0);
	for(int i(1); likely(i < argc) ; ++i) {
		if(seen_escape || paramarg_remain)
		{
			push_back(Parameter(argv[i]));
			if(paramarg_remain)
				--paramarg_remain;
			continue;
		}

		char *ptr(argv[i]);
		if(*ptr != '-')
		{
			push_back(Parameter(ptr));
			continue;
		}
		int opt;
		switch(*++ptr)
		{
			case '\0':
				push_back(Parameter("-"));
				continue;
			case '-':
				/* something that begins with -- */
				switch(*++ptr)
				{
					case '\0':
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
				for(char c(*ptr); likely(c != '\0'); c = *++ptr)
				{
					if(paramarg_remain)
					{
						push_back(Parameter(ptr));
						--paramarg_remain;
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
	while(likely((opt_table->longopt || opt_table->shortopt))) {
		if(unlikely((opt_table->shortopt) && (opt_table->shortopt == opt))) {
			return opt_table;
		}
		++opt_table;
	}
	return NULL;
}

unsigned int
ArgumentReader::numargs(const int opt, struct Option *opt_table)
{
	Option *c(lookup_option(opt, opt_table));
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
	while(likely(opt_table->longopt || opt_table->shortopt)) {
		if(unlikely((opt_table->longopt) && (strcmp(opt_table->longopt, long_opt) == 0))) {
			return opt_table->shortopt;
		}
		++opt_table;
	}
	fprintf(stderr, _("Unknown option --%s\n"), long_opt);
	exit(EXIT_FAILURE);
	return 0;// never reached, but might avoid compiler warning
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
	fprintf(stderr, _("Unknown option -%c\n"), short_opt);
	exit(EXIT_FAILURE);
	return 0;// never reached, but might avoid compiler warning
}

void
ArgumentReader::foldAndRemove(struct Option *opt_table)
{
	ArgumentReader::iterator it(begin());
	while(it != end())
	{
		if(unlikely(it->type == Parameter::ARGUMENT)) {
			++it;
			continue;
		}

		Option *c(lookup_option(it->m_option, opt_table));
		if(unlikely(c == NULL)) {
			++it;
			continue;
		}

		const char *remember("");
		const char *second("");
		bool keep(false);
		bool optional(false);
		switch(c->type)
		{
			case Option::BOOLEAN_F:
			case Option::BOOLEAN_T:
			case Option::BOOLEAN:
					if(c->type == Option::BOOLEAN_T)
						*(c->u.boolean) = true;
					else if(c->type == Option::BOOLEAN_F)
						*(c->u.boolean) = false;
					else
						*(c->u.boolean) = ! *(c->u.boolean);
					it = erase(it);
					break;
			case Option::INTEGER:
					++*(c->u.integer);
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
					if(it == end()) {
						if(!optional) {
							fprintf(stderr, _("Missing parameter to --%s\n"), c->longopt);
							exit(EXIT_FAILURE);
						}
					}
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
						c->u.strlist->push_back(remember);
						break;
					}
					if((c->type == Option::STRING) || (c->type == Option::STRING_OPTIONAL))
					{
						*(c->u.str) = remember;
						break;
					}
					if(it == end()) {
						if(!optional) {
							fprintf(stderr, _("Missing second parameter to --%s\n"), c->longopt);
							exit(EXIT_FAILURE);
						}
					}
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
						*((c->u.pr).first)  = remember;
						*((c->u.pr).second) = second;
						break;
					}
					//if((c->type == Option::PAIRLIST) || (c->type == Option::PAIRLIST_OPTIONAL))
						c->u.prlist->push_back(ArgPair(remember, second));
					break;
			default: // KEEP
					++it;
		}
	}
}
