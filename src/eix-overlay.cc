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
#include <list>
#include <string>

#include "database/header.h"
#include "database/io.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "main/main.h"

using std::list;
using std::string;

using std::cerr;
using std::cout;
using std::endl;

class OverlayOption;
typedef list<OverlayOption> OverlayOptionList;

typedef eix::TinyUnsigned PrintOverlayMode;
static const PrintOverlayMode
	PRINT_OVERLAY_NONE  = 0x00,
	PRINT_OVERLAY_LABEL = 0x01,
	PRINT_OVERLAY_PATH  = 0x02,
	PRINT_OVERLAY_LABEL_PATH = (PRINT_OVERLAY_LABEL | PRINT_OVERLAY_PATH);

static void print_help();
int overlay_loop(string *result, const OverlayOptionList &options) ATTRIBUTE_NONNULL_;
bool open_database(Database *db, DBHeader *header, const char *name) ATTRIBUTE_NONNULL_;
bool print_overlay_data(string *result, const DBHeader &header, const char *overlay, const string &sep, const char *name, PrintOverlayMode mode) ATTRIBUTE_NONNULL_;

static void
print_help()
{
	printf(_(
"Usage: %s [-s SEP] [-f FILE] [-l OVERLAY] [-p OVERLAY] [-o OVERLAY] ...\n"
"Print label or path or both of specified OVERLAY for eix database FILE,\n"
"appending SEP to each data. If SEP is unspecified, the null symbol is used.\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.\n"), program_name);
}

class OverlayOption {
	public:
		char opt;
		const char *arg;

		OverlayOption(char option, const char *argument) :
			opt(option), arg(argument)
		{ }
};

typedef list<OverlayOption> OverlayOptionList;

int
run_eix_overlay(int argc, char *argv[])
{
	OverlayOptionList options;

	if(argc > 0) {
		++argv;
		--argc;
	}
	for(; argc > 0; --argc, ++argv) {
		size_t len(strlen(argv[0]));
		if(unlikely((len < 2) || (argv[0][0] != '-'))) {
			print_help();
			return EXIT_FAILURE;
		}
		char opt(argv[0][1]);
		if(unlikely(strchr("sflpo", opt) == NULLPTR)) {
			print_help();
			return (likely(opt == 'h') ? EXIT_SUCCESS : EXIT_FAILURE);
		}
		const char *arg;
		if(len == 2) {
			if(unlikely(--argc == 0)) {
				print_help();
				return EXIT_FAILURE;
			}
			arg = *(++argv);
		} else {
			arg = &(argv[0][2]);
		}
		options.push_back(OverlayOption(opt, arg));
	}

	string result;
	int ret(overlay_loop(&result, options));
	if(likely(!result.empty())) {
		cout << result;
	}
	return ret;
}

int
overlay_loop(string *result, const OverlayOptionList &options)
{
	string separator(1, '\0');
	const char *name(NULLPTR);
	Database db;
	DBHeader header;
	int ret(EXIT_SUCCESS);
	for(OverlayOptionList::const_iterator it(options.begin());
		likely(it != options.end()); ++it) {
		PrintOverlayMode mode(PRINT_OVERLAY_NONE);
		switch(it->opt) {
			case 's':
				separator.assign(it->arg);
				break;
			case 'f':
				if(likely(open_database(&db, &header, it->arg))) {
					name = it->arg;
				} else {
					name = NULLPTR;
					ret = EXIT_FAILURE;
				}
				break;
			case 'o':
				mode = PRINT_OVERLAY_LABEL_PATH;
				break;
			case 'l':
				mode = PRINT_OVERLAY_LABEL;
				break;
			default:
			// case 'p':
				mode = PRINT_OVERLAY_PATH;
				break;
		}
		if(unlikely(mode == PRINT_OVERLAY_NONE)) {
			continue;
		}
		if(unlikely((name == NULLPTR) || !print_overlay_data(result, header, it->arg, separator, name, mode))) {
			result->append(separator);
			if(likely(mode == PRINT_OVERLAY_LABEL_PATH)) {
				result->append(separator);
			}
			ret = EXIT_FAILURE;
		}
	}
	return ret;
}

bool
open_database(Database *db, DBHeader *header, const char *name)
{
	if(likely((name[0] != '\0') && db->openread(name))) {
		if(likely(db->read_header(header, NULLPTR))) {
			return true;
		}
		cerr << eix::format(_(
			"%s was created with an incompatible eix-update:\n"
			"It uses database format %s (current is %s)."))
			% name % header->version % DBHeader::current
			<< endl;
	} else {
		cerr << eix::format(_(
			"Can't open the database file %r for reading (mode = 'rb')"));
	}
	return false;
}

bool
print_overlay_data(string *result, const DBHeader &header, const char *overlay, const string &print_append, const char *name, PrintOverlayMode mode)
{
	ExtendedVersion::Overlay num;
	if(unlikely(!header.find_overlay(&num, overlay, NULLPTR, 0, DBHeader::OVTEST_ALL))) {
		cerr << eix::format(_("Can't find overlay %s in %s")) % overlay % name << endl;
		return false;
	}
	const OverlayIdent& ov(header.getOverlay(num));
	if((mode & PRINT_OVERLAY_LABEL) != PRINT_OVERLAY_NONE) {
		result->append(ov.label);
		result->append(print_append);
	}
	if((mode & PRINT_OVERLAY_PATH) != PRINT_OVERLAY_NONE) {
		result->append(ov.path);
		result->append(print_append);
	}
	return true;
}
