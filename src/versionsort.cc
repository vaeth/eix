// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <cstdlib>

#include <algorithm>
#include <string>
#include <vector>

#include "eixTk/attribute.h"
#include "eixTk/dialect.h"
#include "eixTk/eixint.h"
#include "eixTk/formated.h"
#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "main/main.h"
#include "portage/basicversion.h"

using std::string;
using std::vector;

ATTRIBUTE_NORETURN ATTRIBUTE_NONNULL_ static void failparse(const string& v);
ATTRIBUTE_NONNULL_ static void get_version(string *version, const char *str);
ATTRIBUTE_NONNULL_ static void get_name_version(string *name, string *version, const char *str);
ATTRIBUTE_NONNULL_ static void parse_version(BasicVersion *b, const string& v);

static void failparse(const string& v) {
	eix::say_error(_("cannot determine version of \"%s\"")) % v;
	std::exit(EXIT_FAILURE);
}

static void get_version(string *version, const char *str) {
	BasicVersion b;
	if(b.parseVersion(str, NULLPTR, false) == BasicVersion::parsedOK) {
		version->assign(str);
		return;
	}
	if(unlikely(!ExplodeAtom::split_version(version, str))) {
		failparse(str);
	}
}

static void get_name_version(string *name, string *version, const char *str) {
	if(unlikely(!ExplodeAtom::split(name, version, str))) {
		failparse(str);
	}
}

static void parse_version(BasicVersion *b, const string& v) {
	string errtext;
	BasicVersion::ParseResult r(b->parseVersion(v, &errtext, true));
	if(unlikely(r != BasicVersion::parsedOK)) {
		eix::say_error() % errtext;
		if(r != BasicVersion::parsedGarbage) {
			std::exit(EXIT_FAILURE);
		}
	}
}

int run_versionsort(int argc, char *argv[]) {
	if(unlikely(argc <= 1))
		return EXIT_SUCCESS;
	if(likely(argc == 2)) {
		string ver;
		get_version(&ver, argv[1]);
		eix::print() % ver;
		return EXIT_SUCCESS;
	}
	if((argc >= 2) && (argv[1][0] == '-') && (argv[1][1] != '\0') && (argv[1][2] == '\0')) {
		eix::SignedBool mode(0);
		bool full(false);
		bool revision(false);
		switch(argv[1][1]) {
			case 'f':
				revision = true;
				ATTRIBUTE_FALLTHROUGH
			case 'p':
				full = true;
				ATTRIBUTE_FALLTHROUGH
			case 'n':
				mode = 1;
				break;
			case 'V':
				full = true;
				ATTRIBUTE_FALLTHROUGH
			case 'r':
				revision = true;
				ATTRIBUTE_FALLTHROUGH
			case 'v':
				mode = -1;
				break;
			default:
				break;
		}
		if(mode) {
			if(unlikely(argc == 2)) {
				return EXIT_SUCCESS;
			}
			for(int curr(2); ;) {
				const char *str(argv[curr]);
				if(mode > 0) {
					string result, curr_version;
					get_name_version(&result, &curr_version, str);
					if(full) {
						result.append(1, '-');
						BasicVersion b;
						parse_version(&b, curr_version);
						result.append(revision ? b.getFull() : b.getPlain());
					}
					eix::print() % result;
				} else {
					string curr_version;
					get_version(&curr_version, str);
					BasicVersion b;
					parse_version(&b, curr_version);
					eix::print() % (full ? b.getFull() :
						(revision ? b.getRevision() :
							b.getPlain()));
				}
				if(argc != 3) {
					eix::say_empty();
				}
				if(unlikely(++curr == argc)) {
					return EXIT_SUCCESS;
				}
			}
		}
	}
	typedef vector<BasicVersion> Versions;
	Versions versions;
	for(int i(1); likely(i < argc); ++i) {
		string curr_version;
		get_version(&curr_version, argv[i]);
		BasicVersion b;
		parse_version(&b, curr_version);
		versions.PUSH_BACK(MOVE(b));
	}
	sort(versions.begin(), versions.end());
	for(Versions::const_iterator it(versions.begin());
		likely(it != versions.end()); ++it) {
		eix::say() % it->getFull();
	}
	return EXIT_SUCCESS;
}

