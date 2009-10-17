// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include <main/main.h>

#include <eixTk/formated.h>
#include <portage/basicversion.h>

#include <algorithm>
#include <iostream>

using namespace std;

const char *parse_version(const char *v);

const char *
parse_version(const char *v)
{
	static string r;
	char *s = ExplodeAtom::split_version(v);
	if(s) {
		r = s;
		free(s);
		return r.c_str();
	}
	if(isdigit(*v, localeC))
		return v;
	cerr << eix::format(_("cannot determine version of %s")) % v << endl;
	exit(1);
}

int
run_versionsort(int argc, char *argv[])
{
	if(argc <= 1)
		return 0;
	if(argc == 2) {
		cout << parse_version(argv[1]);
		return 0;
	}
	vector<BasicVersion> versions;
	for(int i = 1; i < argc; ++i)
		versions.push_back(BasicVersion(parse_version(argv[i])));
	sort(versions.begin(), versions.end());
	for(vector<BasicVersion>::const_iterator it = versions.begin();
		it != versions.end(); ++it) {
		cout << it->getFull() << "\n";
	}
	return 0;
}

