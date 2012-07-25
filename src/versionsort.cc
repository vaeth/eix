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

static void failparse(const char *v) ATTRIBUTE_NORETURN;
static const char *get_version(const char *v);
static const char *get_version(const char *&name, const char *v);
static void parse_version(BasicVersion *b, const char *v);

static void
failparse(const char *v)
{
	cerr << eix::format(_("cannot determine version of %s")) % v << endl;
	exit(EXIT_FAILURE);
}

static const char *
get_version(const char *v)
{
	static string r;
	BasicVersion b;
	if(b.parseVersion(v, NULLPTR, false) == BasicVersion::parsedOK) {
		r = v;
		return r.c_str();
	}
	char *s(ExplodeAtom::split_version(v));
	if(unlikely(s == NULLPTR)) {
		failparse(v);
	}
	r = s;
	free(s);
	return r.c_str();
}

static const char *
get_version(const char *&name, const char *v)
{
	static string r;
	static string n;
	char **s(ExplodeAtom::split(v));
	if(unlikely(s == NULLPTR)) {
		failparse(v);
	}
	if(name != NULLPTR) {
		n = s[0];
		name = n.c_str();
	}
	free(s[0]);
	r = s[1];
	free(s[1]);
	return r.c_str();
}

static void
parse_version(BasicVersion *b, const char *v) {
	string errtext;
	BasicVersion::ParseResult r(b->parseVersion(v, &errtext, true));
	if(unlikely(r != BasicVersion::parsedOK)) {
		cerr << errtext << endl;
		if(r != BasicVersion::parsedGarbage) {
			exit(EXIT_FAILURE);
		}
	}
}

int
run_versionsort(int argc, char *argv[])
{
	if(unlikely(argc <= 1))
		return EXIT_SUCCESS;
	if(likely(argc == 2)) {
		cout << get_version(argv[1]);
		return EXIT_SUCCESS;
	}
	if((argc == 3) && (argv[1][0] == '-') && (argv[1][1]) && !argv[1][2]) {
		char c(argv[1][1]);
		const char *s(argv[2]);
		switch(c) {
			case 'n':
			case 'p':
				{
					const char *n;
					const char *v(get_version(n, s));
					string r(n);
					if(c != 'n') {
						r.append("-");
						BasicVersion b;
						parse_version(&b, v);
						r.append(b.getPlain());
					}
					cout << r;
				}
				return EXIT_SUCCESS;
			case 'v':
			case 'r':
				{
					BasicVersion b;
					parse_version(&b, get_version(s));
					cout << ((c == 'v') ? b.getPlain() : b.getRevision());
				}
				return EXIT_SUCCESS;
			default:
				break;
		}
	}
	vector<BasicVersion> versions;
	for(int i(1); likely(i < argc); ++i) {
		BasicVersion b;
		parse_version(&b, get_version(argv[i]));
		versions.push_back(b);
	}
	sort(versions.begin(), versions.end());
	for(vector<BasicVersion>::const_iterator it(versions.begin());
		likely(it != versions.end()); ++it) {
		cout << it->getFull() << "\n";
	}
	return EXIT_SUCCESS;
}

