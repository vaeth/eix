// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>
#include <vector>

#include "database/header.h"
#include "database/io.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "main/main.h"

using std::string;
using std::vector;

using std::cerr;
using std::cout;
using std::endl;

class OverlayOption;
typedef vector<OverlayOption> OverlayOptionList;

typedef eix::TinyUnsigned PrintOverlayMode;
static CONSTEXPR const PrintOverlayMode
	PRINT_OVERLAY_NONE  = 0x00,
	PRINT_OVERLAY_LABEL = 0x01,
	PRINT_OVERLAY_PATH  = 0x02,
	PRINT_OVERLAY_FORMAT = 0x04,
	PRINT_OVERLAY_LABEL_PATH = (PRINT_OVERLAY_LABEL | PRINT_OVERLAY_PATH);

static void print_help();
ATTRIBUTE_NONNULL_ int overlay_loop(string *result, const OverlayOptionList& options);
ATTRIBUTE_NONNULL_ bool open_database(DBHeader *header, const char *name, bool verbose);
ATTRIBUTE_NONNULL_ bool print_overlay_data(string *result, const DBHeader *header, const char *overlay, const string& sep, const char *name, PrintOverlayMode mode, bool verbose);

static void print_help() {
	cout << eix::format(
	/* xgettext: no-space-ellipsis-check */
_("Usage: %s [-q] [-f FILE] [-s SEP] [-c|-C] [-l OV] [-p OV] [-o OV] ...\n"
"Check whether eix database FILE has current format, and print label, path,\n"
"or both of specified overlay OV in FILE, appending SEP to each data.\n"
"All options can be used repeatedly: FILE and SEP are active until modified.\n"
"FILE defaults to %s\n"
"Note that this default for FILE is independent of the EIX_CACHEFILE variable,\n"
"i.e. for usage in portable scripts you should always specify -f as one of the\n"
"first options, e.g. as in this script snippet:\n"
"\t. eix-functions.sh; ReadFunctions\n"
"\tReadVar EIX_CACHEFILE || die 'cache name not known'\n"
"\t%s -qf \"$EIX_CACHEFILE\" -c || die 'no current database'\n"
"SEP defaults to the null character (to be distinguished from the empty string).\n"
"For the case the OV is the empty string, the main overlay is printed (as if OV\n"
"is 0), but if there is no overlay in file, no error (but empty data) is output.\n"
"-c can be used to check only whether FILE has current format without producing\n"
"any output. -C outputs the actual format number.\n"
"After option -q no further output is made, usually; -C is exceptional here.\n"
"So specify -q first if you want no output at all.\n"
"The special option -h outputs this help text and quits.\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.\n")) % program_name % EIX_CACHEFILE % program_name;
}

class OverlayOption {
	public:
		char opt;
		const char *arg;

		explicit OverlayOption(char option) : opt(option) {
		}

		OverlayOption(char option, const char *argument) :
			opt(option), arg(argument) {
		}
};

int run_eix_header(int argc, char *argv[]) {
	bool verbose(true);
	OverlayOptionList options;

	if(argc > 0) {
		++argv;
		--argc;
	}
	if(argc == 0) {
		print_help();
		return EXIT_FAILURE;
	}
	for(; argc > 0; --argc, ++argv) {
		size_t len(strlen(argv[0]));
		if(unlikely((len < 2) || (argv[0][0] != '-'))) {
			print_help();
			return EXIT_FAILURE;
		}
		size_t curr(1);
		char opt(argv[0][curr++]);
		while(unlikely(strchr("CcqhH?", opt) != NULLPTR)) {
			switch(opt) {
				case '?':
				case 'H':
				case 'h':
					if(likely(verbose)) {
						print_help();
					}
					return EXIT_SUCCESS;
				case 'q':
					verbose = false;
				default:
					break;
			}
			options.push_back(OverlayOption(opt));
			if(curr == len) {
				opt = 0;
				break;
			}
			opt = argv[0][curr++];
		}
		if(opt == 0) {
			continue;
		}
		if(unlikely(strchr("sflpo", opt) == NULLPTR)) {
			if(likely(verbose)) {
				print_help();
			}
			return EXIT_FAILURE;
		}
		const char *arg;
		if(curr == len) {
			if(unlikely(--argc == 0)) {
				if(likely(verbose)) {
					print_help();
				}
				return EXIT_FAILURE;
			}
			arg = *(++argv);
		} else {
			arg = &(argv[0][curr]);
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

int overlay_loop(string *result, const OverlayOptionList& options) {
	bool verbose(true);
	string separator(1, '\0');
	const char *name(EIX_CACHEFILE);
	DBHeader *header = NULLPTR;
	int ret(EXIT_SUCCESS);
	bool changedname(true);
	for(OverlayOptionList::const_iterator it(options.begin());
		likely(it != options.end()); ++it) {
		PrintOverlayMode mode;
		switch(it->opt) {
			case 'q':
				verbose = false;
				continue;
			case 's':
				separator.assign(it->arg);
				continue;
			case 'f':
				changedname = true;
				name = it->arg;
				continue;
			case 'o':
				mode = PRINT_OVERLAY_LABEL_PATH;
				break;
			case 'l':
				mode = PRINT_OVERLAY_LABEL;
				break;
			case 'p':
				mode = PRINT_OVERLAY_PATH;
				break;
			case 'C':
				mode = PRINT_OVERLAY_FORMAT;
				break;
			default:
			// case 'c':
				mode = PRINT_OVERLAY_NONE;
				break;
		}
		if(changedname) {
			changedname = false;
			delete header;
			header = new DBHeader;
			if(unlikely(!open_database(header, name, verbose))) {
				name = NULLPTR;
				ret = EXIT_FAILURE;
			}
		}
		if(unlikely((mode & PRINT_OVERLAY_FORMAT) != PRINT_OVERLAY_NONE)) {
			if(likely(name != NULLPTR)) {
				result->append(eix::format("%s") % header->version);
			}
			result->append(separator);
		}
		if(unlikely((mode & PRINT_OVERLAY_LABEL_PATH) == PRINT_OVERLAY_NONE)) {
			continue;
		}
		if(unlikely((name == NULLPTR) || !print_overlay_data(result, header, it->arg, separator, name, mode, verbose))) {
			if(likely(verbose)) {
				result->append(separator);
				if(likely(mode == PRINT_OVERLAY_LABEL_PATH)) {
					result->append(separator);
				}
			}
			ret = EXIT_FAILURE;
		}
	}
	delete header;
	return ret;
}

bool open_database(DBHeader *header, const char *name, bool verbose) {
	Database db;
	if(likely((name[0] != '\0') && db.openread(name))) {
		if(likely(db.read_header(header, NULLPTR, 0))) {
			return true;
		}
		if(likely(verbose)) {
			cerr << eix::format(_(
				"%s was created with an incompatible eix-update:\n"
				"It uses database format %s (current is %s)."))
				% name % header->version % DBHeader::current
				<< endl;
		}
	} else {
		if(likely(verbose)) {
			cerr << eix::format(_(
				"cannot open database file %s for reading (mode = 'rb')"))
				% name << endl;
		}
	}
	return false;
}

bool print_overlay_data(string *result, const DBHeader *header, const char *overlay, const string& print_append, const char *name, PrintOverlayMode mode, bool verbose) {
	ExtendedVersion::Overlay num;
	bool emptysearch(overlay[0] == '\0');
	bool found(header->find_overlay(&num, emptysearch ? "0" : overlay, NULLPTR,
		0, emptysearch ? DBHeader::OVTEST_NUMBER : DBHeader::OVTEST_ALL));
	if(unlikely((!found) && !emptysearch)) {
		if(likely(verbose)) {
			cerr << eix::format(_("cannot find overlay %s in %s")) % overlay % name << endl;
		}
		return false;
	}
	if(unlikely(!verbose)) {
		return true;
	}
	const OverlayIdent *ov(found ? &header->getOverlay(num)
		: new OverlayIdent(""));
	if((mode & PRINT_OVERLAY_LABEL) != PRINT_OVERLAY_NONE) {
		if(likely(ov->know_label)) {
			result->append(ov->label);
		}
		result->append(print_append);
	}
	if((mode & PRINT_OVERLAY_PATH) != PRINT_OVERLAY_NONE) {
		result->append(ov->path);
		result->append(print_append);
	}
	if(unlikely(!found)) {
		delete ov;
	}
	return true;
}
