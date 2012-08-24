// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "eixTk/i18n.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "main/main.h"
#include "various/drop_permissions.h"

static void
print_help()
{
	printf(_("Usage: %s [--] command [options]\n"
"Executes command [options] with the permissions according to the eix variables\n"
"EIX_USER, EIX_GROUP, EIX_UID, and EIX_GID.\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.\n"), program_name);
}

int
run_eix_drop_permissions(int argc, char *argv[])
{
	EixRc &eixrc(get_eixrc(DROP_VARS_PREFIX));
	drop_permissions(&eixrc);
	bool parsing(true);
	for(++argv; (argc > 0) && parsing; --argc, ++argv) {
		if(strcmp("--", argv[0]) == 0) {
			parsing = false;
			continue;
		}
		// parse other options here ...
		break;
	}
	if(argc == 0) {
		print_help();
		return EXIT_FAILURE;
	}
	execv(argv[0], argv);
	fprintf(stderr, _("failed to execute %s\n"), argv[0]);
	return EXIT_FAILURE;  // exit(EXIT_FAILURE); // make check_includes happy
}
