// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include "eixTk/argsreader.h"
#include <config.h>  // IWYU pragma: keep

#include <cstdlib>
#include <cstring>

#include "eixTk/attribute.h"
#include "eixTk/diagnostics.h"
#include "eixTk/dialect.h"
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
			EMPLACE_BACK(Parameter, (argv[i]));
			if(paramarg_remain != 0) {
				--paramarg_remain;
			}
			continue;
		}

		const char *ptr(argv[i]);
		if(*ptr != '-') {
			EMPLACE_BACK(Parameter, (ptr));
			continue;
		}
		switch(*++ptr) {
			case '\0':
				EMPLACE_BACK(Parameter, ("-"));
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
						EMPLACE_BACK(Parameter, (ptr));
						continue;
					default:
						/* some longopt */
						{
							const char *param;
							int opt(lookup_longopt(&param, &paramarg_remain, ptr, opt_table));
							EMPLACE_BACK(Parameter, (opt));
							if (param != NULLPTR) {
								EMPLACE_BACK(Parameter, (param));
							}
						}
						break;
				}
				break;
			default:
				/* some shortopts */
				for(char c(*ptr); likely(c != '\0'); c = *++ptr) {
					if(paramarg_remain != 0) {
						EMPLACE_BACK(Parameter, (ptr));
						--paramarg_remain;
						break;
					}
					const Option *option(lookup_shortopt(c, opt_table));
					EMPLACE_BACK(Parameter, (option->shortopt));
					paramarg_remain = numargs(option->type);
				}
				break;
		}
	}

	foldAndRemove(opt_table);
}

eix::TinyUnsigned ArgumentReader::numargs(const Option::Type opt_type) {
	switch(opt_type) {
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

const Option *ArgumentReader::lookup_option(const int opt, const OptionList& opt_table) {
	for(OptionList::const_iterator it(opt_table.begin());
		likely(it != opt_table.end()); ++it) {
		if(unlikely(it->shortopt == opt)) {
			return &(*it);
		}
	}
	return NULLPTR;
}

/**
Return shortopt for longopt stored in opt.
@param arg is set to start of =-separated argument or NULLPTR.
@param paramargs_remain is set to the number of args remaining for this option after arg.
@param long_opt longopt that should be resolved.
@return shortopt for given longopt
**/
int ArgumentReader::lookup_longopt(const char **arg, eix::TinyUnsigned *paramargs_remain, const char *long_opt, const OptionList& opt_table) {
	for(OptionList::const_iterator it(opt_table.begin());
		likely(it != opt_table.end()); ++it) {
		const char *optname = it->longopt;
		if(unlikely(optname == NULLPTR)) {
			continue;
		}
		size_t len = strlen(optname);
		if(unlikely(std::strncmp(optname, long_opt, len) == 0)) {
			const char *next = long_opt + len;
			eix::TinyUnsigned num_args = numargs(it->type);
			switch(*next) {
				case '\0':
					*arg = NULLPTR;
					*paramargs_remain = num_args;
					return it->shortopt;
				case '=':
					*arg = ++next;
					if(num_args == 0) {
						eix::say_error(_("Option --%s must not have argument but has %s")) % optname % next;
						std::exit(EXIT_FAILURE);
					}
					*paramargs_remain = num_args - 1;
					return it->shortopt;
				default:
					break;
			}
		}
	}
	eix::say_error(_("unknown option --%s")) % long_opt;
	std::exit(EXIT_FAILURE);
	return 0;  // never reached, but might avoid compiler warning
}

/**
Check if short_opt is a known option.
@param short_opt short_opt that should be resolved.
@return Option
**/
const Option *ArgumentReader::lookup_shortopt(const char short_opt, const OptionList& opt_table) {
	const Option *option = lookup_option(short_opt, opt_table);
	if (unlikely(option == NULLPTR)) {
		eix::say_error(_("unknown option -%s")) % short_opt;
		std::exit(EXIT_FAILURE);
	}
	return option;
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
					ATTRIBUTE_FALLTHROUGH
			case Option::KEEP_STRING:
			case Option::KEEP_PAIR:
					keep = true;
					ATTRIBUTE_FALLTHROUGH
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
							std::exit(EXIT_FAILURE);
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
						c->u.strlist->PUSH_BACK(remember);
						break;
					}
					if((c->type == Option::STRING) || (c->type == Option::STRING_OPTIONAL)) {
						*(c->u.str) = remember;
						break;
					}
					if(it == end()) {
						if(!optional) {
							eix::say_error(_("missing second parameter to --%s")) % (c->longopt);
							std::exit(EXIT_FAILURE);
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
					c->u.prlist->EMPLACE_BACK(ArgPair, (remember, second));
					break;
			default:  // KEEP
					++it;
		}
	}
}
