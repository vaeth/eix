// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include <eixTk/exceptions.h>
#include <eixTk/formated.h>
#include <eixTk/i18n.h>
#include <eixTk/likely.h>
#include <eixTk/null.h>
#include <eixTk/stringutils.h>
#include <main/main.h>
#include <portage/basicversion.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <cstdlib>

using namespace std;

void failparse(const char *v) ATTRIBUTE_NORETURN;
const char *parse_version(const char *v);
const char *parse_version(const char *v, const char **name);

void
failparse(const char *v)
{
	cerr << eix::format(_("cannot determine version of %s")) % v << endl;
	exit(EXIT_FAILURE);
}

const char *
parse_version(const char *v)
{
	static string r;
	try {
		BasicVersion b;
		b.parseVersion(v, true);
		return v;
	}
	catch(const ExBasic &e) { }
	char *s(ExplodeAtom::split_version(v));
	if(unlikely(s == NULLPTR)) {
		failparse(v);
	}
	r = s;
	free(s);
	return r.c_str();
}

const char *
parse_version(const char *v, const char **name)
{
	static string r;
	static string n;
	char **s(ExplodeAtom::split(v));
	if(unlikely(s == NULLPTR)) {
		failparse(v);
	}
	if(name != NULLPTR) {
		n = s[0];
		*name = n.c_str();
	}
	free(s[0]);
	r = s[1];
	free(s[1]);
	return r.c_str();
}

int
run_versionsort(int argc, char *argv[])
{
	if(unlikely(argc <= 1))
		return EXIT_SUCCESS;
	if(likely(argc == 2)) {
		cout << parse_version(argv[1]);
		return EXIT_SUCCESS;
	}
	if((argc == 3) && (argv[1][0] == '-') && (argv[1][1]) && !argv[1][2]) {
		char c(argv[1][1]);
		const char *s(argv[2]);
		const char *v;
		switch(c) {
			case 'n':
			case 'p':
				{
					const char *n;
					v = parse_version(s, &n);
					string r(n);
					if(c != 'n') {
						r.append("-");
						r.append(BasicVersion(v).getPlain());
					}
					cout << r;
				}
				return EXIT_SUCCESS;
			case 'v':
			case 'r':
				v = parse_version(s);
				{
					BasicVersion b(v);
					cout << ((c == 'v') ? b.getPlain() : b.getRevision());
				}
				return EXIT_SUCCESS;
			default:
				break;
		}
	}
	vector<BasicVersion> versions;
	for(int i(1); likely(i < argc); ++i)
		versions.push_back(BasicVersion(parse_version(argv[i])));
	sort(versions.begin(), versions.end());
	for(vector<BasicVersion>::const_iterator it(versions.begin());
		likely(it != versions.end()); ++it) {
		cout << it->getFull() << "\n";
	}
	return EXIT_SUCCESS;
}

