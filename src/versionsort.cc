// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <cstdlib>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

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

using std::cerr;
using std::cout;
using std::endl;

static void failparse(const char *v) ATTRIBUTE_NONNULL_ ATTRIBUTE_NORETURN;
static const char *get_version(const char *v) ATTRIBUTE_NONNULL_;
static const char *get_version(const char **name, const char *v) ATTRIBUTE_NONNULL_;
static void parse_version(BasicVersion *b, const char *v) ATTRIBUTE_NONNULL_;

static void failparse(const char *v) {
	cerr << eix::format(_("cannot determine version of %s")) % v << endl;
	exit(EXIT_FAILURE);
}

static const char *get_version(const char *v) {
	static string *r = NULLPTR;
	delete r;
	BasicVersion b;
	if(b.parseVersion(v, NULLPTR, false) == BasicVersion::parsedOK) {
		r = new string(v);
		return r->c_str();
	}
	char *s(ExplodeAtom::split_version(v));
	if(unlikely(s == NULLPTR)) {
		failparse(v);
	}
	r = new string(s);
	free(s);
	return r->c_str();
}

static const char *get_version(const char **name, const char *v) {
	static string *r = NULLPTR;
	static string *n = NULLPTR;
	delete r;
	delete n;
	char **s(ExplodeAtom::split(v));
	if(unlikely(s == NULLPTR)) {
		failparse(v);
	}
	if(name != NULLPTR) {
		n = new string(first_alnum(s[0]));
		*name = n->c_str();
	}
	free(s[0]);
	r = new string(s[1]);
	free(s[1]);
	return r->c_str();
}

static void parse_version(BasicVersion *b, const char *v) {
	string errtext;
	BasicVersion::ParseResult r(b->parseVersion(v, &errtext, true));
	if(unlikely(r != BasicVersion::parsedOK)) {
		cerr << errtext << endl;
		if(r != BasicVersion::parsedGarbage) {
			exit(EXIT_FAILURE);
		}
	}
}

int run_versionsort(int argc, char *argv[]) {
	if(unlikely(argc <= 1))
		return EXIT_SUCCESS;
	if(likely(argc == 2)) {
		cout << get_version(argv[1]);
		return EXIT_SUCCESS;
	}
	if((argc >= 2) && (argv[1][0] == '-') && (argv[1][1] != '\0') && (argv[1][2] == '\0')) {
		eix::SignedBool mode(0);
		bool full(false);
		bool revision(false);
		switch(argv[1][1]) {
			case 'f':
				revision = true;
			case 'p':
				full = true;
			case 'n':
				mode = 1;
				break;
			case 'V':
				full = true;
			case 'r':
				revision = true;
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
				const char *s(argv[curr]);
				if(mode > 0) {
					const char *n;
					const char *v(get_version(&n, s));
					string r(n);
					if(full) {
						r.append("-");
						BasicVersion b;
						parse_version(&b, v);
						r.append(revision ? b.getFull() : b.getPlain());
					}
					cout << r;
				} else {
					BasicVersion b;
					parse_version(&b, get_version(s));
					cout << (full ? b.getFull() : (revision ? b.getRevision() : b.getPlain()));
				}
				if(argc != 3) {
					cout << "\n";
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
		BasicVersion b;
		parse_version(&b, get_version(argv[i]));
		versions.push_back(b);
	}
	sort(versions.begin(), versions.end());
	for(Versions::const_iterator it(versions.begin());
		likely(it != versions.end()); ++it) {
		cout << it->getFull() << "\n";
	}
	return EXIT_SUCCESS;
}

