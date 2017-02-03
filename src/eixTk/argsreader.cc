// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/argsreader.h"
#include <config.h>

#include <cstdlib>
#include <cstring>

#include "eixTk/diagnostics.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"

ArgumentReader::ArgumentReader(int argc, const char *const *argv, const OptionList& opt_table) {
GCC_DIAG_OFF(sign-conversion)
	reserve(argc);  // argc is 1 too large, but there might be bundeled options
GCC_DIAG_ON(sign-conversion)
	bool seen_escape(false);
	name = argv[0];
	eix::TinyUnsigned paramarg_remain(0);
	for(int i(1); likely(i < argc) ; ++i) {
		if(seen_escape || paramarg_remain) {
			push_back(Parameter(argv[i]));
			if(paramarg_remain != 0) {
				--paramarg_remain;
			}
			continue;
		}

		const char *ptr(argv[i]);
		if(*ptr != '-') {
			push_back(Parameter(ptr));
			continue;
		}
		int opt;
		switch(*++ptr) {
			case '\0':
				push_back(Parameter("-"));
				continue;
			case '-':
				/* something that begins with -- */
				switch(*++ptr) {
					case '\0':
						/* a lonely -- */
						seen_escape = true;
						continue;
					case '-':
						/* something ---escaped */
						push_back(Parameter(ptr));
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
				for(char c(*ptr); likely(c != '\0'); c = *++ptr) {
					if(paramarg_remain) {
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

const Option *ArgumentReader::lookup_option(const int opt, const OptionList& opt_table) {
	for(OptionList::const_iterator it(opt_table.begin());
		likely(it != opt_table.end()); ++it) {
		if(unlikely(it->shortopt == opt)) {
			return &(*it);
		}
	}
	return NULLPTR;
}

eix::TinyUnsigned ArgumentReader::numargs(const int opt, const OptionList& opt_table) {
	const Option *c(lookup_option(opt, opt_table));
	if(c == NULLPTR) {
		return 0;
	}
	switch(c->type) {
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

/**
Return shortopt for longopt stored in opt.
@param long_opt longopt that should be resolved.
@return shortopt for given longopt
**/
int ArgumentReader::lookup_longopt(const char *long_opt, const OptionList& opt_table) {
	for(OptionList::const_iterator it(opt_table.begin());
		likely(it != opt_table.end()); ++it) {
		if(unlikely((it->longopt != NULLPTR) && (strcmp(it->longopt, long_opt) == 0))) {
			return it->shortopt;
		}
	}
	eix::say_error(_("unknown option --%s")) % long_opt;
	exit(EXIT_FAILURE);
	return 0;  // never reached, but might avoid compiler warning
}

/**
Check if short_opt is a known option.
@param long_opt longopt that should be resolved.
@return shortopt for given longopt
**/
int ArgumentReader::lookup_shortopt(const char short_opt, const OptionList& opt_table) {
	for(OptionList::const_iterator it(opt_table.begin());
		likely(it != opt_table.end()); ++it) {
		if(unlikely(it->shortopt == short_opt))
			return short_opt;
	}
	eix::say_error(_("unknown option -%s")) % short_opt;
	exit(EXIT_FAILURE);
	return 0;  // never reached, but might avoid compiler warning
}

void ArgumentReader::foldAndRemove(const OptionList& opt_table) {
	ArgumentReader::iterator it(begin());
	while(it != end()) {
		if(unlikely(it->type == Parameter::ARGUMENT)) {
			++it;
			continue;
		}

		const Option *c(lookup_option(it->m_option, opt_table));
		if(unlikely(c == NULLPTR)) {
			++it;
			continue;
		}

		const char *remember("");
		const char *second("");
		bool keep(false);
		bool optional(false);
		switch(c->type) {
			case Option::BOOLEAN_F:
			case Option::BOOLEAN_T:
			case Option::BOOLEAN:
					if(c->type == Option::BOOLEAN_T) {
						*(c->u.boolean) = true;
					} else if(c->type == Option::BOOLEAN_F) {
						*(c->u.boolean) = false;
					} else {
						*(c->u.boolean) = (!(*(c->u.boolean)));
					}
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
					if(keep) {
						++it;
					} else {
						it = erase(it);
					}
					if(it == end()) {
						if(!optional) {
							eix::say_error(_("missing parameter to --%s")) % (c->longopt);
							exit(EXIT_FAILURE);
						}
					} else {
						remember = it->m_argument;
						if(keep) {
							++it;
						} else {
							it = erase(it);
						}
					}
					if((c->type == Option::KEEP_STRING) || (c->type == Option::KEEP_STRING_OPTIONAL)) {
						break;
					}
					if((c->type == Option::STRINGLIST) || (c->type == Option::STRINGLIST_OPTIONAL)) {
						c->u.strlist->push_back(remember);
						break;
					}
					if((c->type == Option::STRING) || (c->type == Option::STRING_OPTIONAL)) {
						*(c->u.str) = remember;
						break;
					}
					if(it == end()) {
						if(!optional) {
							eix::say_error(_("missing second parameter to --%s")) % (c->longopt);
							exit(EXIT_FAILURE);
						}
					} else {
						second = it->m_argument;
						if(keep) {
							++it;
						} else {
							it = erase(it);
						}
					}
					if(keep) {
					// ((c->type == Option::KEEP_PAIR) || (c->type == Option::KEEP_PAIR_OPTIONAL))
						break;
					}
					if((c->type == Option::PAIR) || (c->type == Option::PAIR_OPTIONAL)) {
						*((c->u.pr).first)  = remember;
						*((c->u.pr).second) = second;
						break;
					}
					// if((c->type == Option::PAIRLIST) || (c->type == Option::PAIRLIST_OPTIONAL))
					c->u.prlist->push_back(ArgPair(remember, second));
					break;
			default:  // KEEP
					++it;
		}
	}
}
