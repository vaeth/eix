// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>

#include "eixTk/argsreader.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "eixTk/utils.h"
#include "main/main.h"
#include "portage/basicversion.h"
#include "portage/mask.h"
#include "portage/mask_list.h"
#include "portage/package.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

static void print_help() {
	printf(_("Usage: %s [options] category/name-version[:slot][::repo] ...\n"
"Output all arguments matching a list of mask\n"
"Options:\n"
" -h, --help              show a short help screen\n"
" -q, --quiet             Do not output matching arguments, but exit with\n"
"                         success only if at least one argument matches.\n"
" -m, --mask MASK         add MASK to the list of masks\n"
" -f, --file FILE         add masks from FILE.\n"
" -F, --read-file FILE    read args as words from FILE.\n"
"Directories are read recursively.\n"
"An empty/omitted filename means standard input.\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.\n"), program_name);
}

/** Local options for argument reading. */
static struct LocalOptions {
	bool
		be_quiet,
		help;
} rc_options;

/** Arguments and options. */
class MaskedOptionList : public OptionList {
	public:
		MaskedOptionList();
};

MaskedOptionList::MaskedOptionList() {
	push_back(Option("help",      'h', Option::BOOLEAN, &rc_options.help));
	push_back(Option("quiet",     'q', Option::BOOLEAN, &rc_options.be_quiet));
	push_back(Option("file",      'f', Option::KEEP_STRING_OPTIONAL));
	push_back(Option("read-file", 'F', Option::KEEP_STRING_OPTIONAL));
	push_back(Option("mask",      'm', Option::KEEP_STRING));
}

static void read_stdin(vector<string> *lines, string *name) ATTRIBUTE_NONNULL((1));
static void add_file(vector<string> *lines, const string &name, string *new_name = NULLPTR) ATTRIBUTE_NONNULL((1));
static void add_file(PreList *pre_list, const string &name) ATTRIBUTE_NONNULL_;
static void add_words(vector<string> *lines, const string &name) ATTRIBUTE_NONNULL_;
static void read_args(MaskList<Mask> *mask_list, vector<string> *args, const ArgumentReader &ar) ATTRIBUTE_NONNULL_;
static const char *opt_arg(ArgumentReader::const_iterator *arg, const ArgumentReader &ar) ATTRIBUTE_NONNULL_;

static void read_stdin(vector<string> *lines, string *name) {
	static size_t stdin_count(0);
	while(likely(!std::cin.eof())) {
		string line;
		getline(std::cin, line);
		trim(&line);
		if((!line.empty()) && (line[0] != '#')) {
			lines->push_back(line);
		}
	}
	if(stdin_count++ == 0) {
		if(name != NULLPTR) {
			*name = "(stdin)";
		}
	} else if(name != NULLPTR) {
		*name = eix::format("(stdin%s)") % stdin_count;
	}
}

static void add_file(vector<string> *lines, const string &name, string *new_name) {
	if(name.empty() || (name == "-")) {
		read_stdin(lines, new_name);
	} else {
		pushback_lines(name.c_str(), lines, true);
		if(new_name != NULLPTR) {
			*new_name = name;
		}
	}
}

static void add_file(PreList *pre_list, const string &name) {
	vector<string> lines;
	string new_name;
	add_file(&lines, name, &new_name);
	pre_list->handle_file(lines, new_name, NULLPTR, false);
}

static void add_words(vector<string> *words, const string &name) {
	vector<string> lines;
	add_file(&lines, name);
	for(vector<string>::const_iterator it(lines.begin());
		likely(it != lines.end()); ++it) {
		split_string(words, *it);
	}
}

static const char *opt_arg(ArgumentReader::const_iterator *arg, const ArgumentReader &ar) {
	if(unlikely(++(*arg) == ar.end())) {
		--(*arg);
		return "";
	}
	return (*arg)->m_argument;
}

static void read_args(MaskList<Mask> *mask_list, vector<string> *args, const ArgumentReader &ar) {
	PreList::LineNumber linenr(0);
	PreList pre_list;
	PreListEntry::FilenameIndex argindex;
	bool need_argindex(true);
	for(ArgumentReader::const_iterator arg(ar.begin());
		likely(arg != ar.end()); ++arg) {
		switch(**arg) {
			case 'f':
				add_file(&pre_list, opt_arg(&arg, ar));
				break;
			case 'm':
				if(need_argindex) {
					need_argindex = false;
					argindex =  pre_list.push_name("(arg)", NULLPTR);
				}
				pre_list.handle_line(opt_arg(&arg, ar), argindex, ++linenr, false, false);
				break;
			case 'F':
				add_words(args, opt_arg(&arg, ar));
				break;
			default:
				args->push_back(arg->m_argument);
				break;
		}
	}
	pre_list.initialize(mask_list, Mask::maskMask);
}

int run_masked_packages(int argc, char *argv[]) {
	memset(&rc_options, 0, sizeof(rc_options));
	ArgumentReader argreader(argc, argv, MaskedOptionList());

	if(rc_options.help) {
		print_help();
		return EXIT_SUCCESS;
	}

	MaskList<Mask> mask_list;
	vector<string> args;
	read_args(&mask_list, &args, argreader);

	for(vector<string>::const_iterator it(args.begin());
		likely(it != args.end()); ++it) {
		Mask m(Mask::maskPseudomask);
		string errtext;
		if(unlikely(m.parseMask(it->c_str(), &errtext) == BasicVersion::parsedError)) {
			cerr << eix::format(_("warning: ignoring %r: %s")) % *it % errtext << endl;
			continue;
		}
		Package p;
		m.to_package(&p);
		if(!mask_list.applyMasks(&p)) {
			continue;
		}
		for(Package::const_iterator v(p.begin()); v != p.end(); ++v) {
			if(v->maskflags.isHardMasked()) {
				if(rc_options.be_quiet) {
					return EXIT_SUCCESS;
				}
				cout << *it << "\n";
				break;
			}
		}
	}
	return (rc_options.be_quiet ? EXIT_FAILURE : EXIT_SUCCESS);
}
