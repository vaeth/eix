// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Martin Väth <martin@mvath.de>

#include <config.h>

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixrc/eixrc.h"
#include "eixrc/global.h"
#include "main/main.h"
#include "various/drop_permissions.h"

static void print_help() {
	printf(_("Usage: %s [options] [--] command [arguments]\n"
"Executes command [arguments] with the permissions according to the eix\n"
"variables EIX_USER, EIX_GROUP, EIX_UID, and EIX_GID.\n"
"The following options are available:\n"
"-q suppress output of warnings/errors\n"
"-e error (do not execute command) if change of permission failed\n"
"\n"
"This program is covered by the GNU General Public License. See COPYING for\n"
"further information.\n"), program_name);
}

int run_eix_drop_permissions(int argc, char *argv[]) {
	EixRc& eixrc(get_eixrc(DROP_VARS_PREFIX));
	if(argc > 0) {
		++argv;
		--argc;
	}
	bool quiet(false);
	bool make_fatal(false);
	while(argc > 0) {
		if(strcmp("-q", argv[0]) == 0) {
			quiet = true;
			++argv;
			--argc;
			continue;
		} else if(strcmp("-e", argv[0]) == 0) {
			make_fatal = true;
			++argv;
			--argc;
			continue;
		} else if((strcmp("-eq", argv[0]) == 0) || (strcmp("-qe", argv[0]) == 0)) {
			make_fatal = quiet = true;
			++argv;
			--argc;
			continue;
		} else if(strcmp("--", argv[0]) == 0) {
			++argv;
			--argc;
		}
		break;
	}
	if(argc == 0) {
		print_help();
		return EXIT_FAILURE;
	}
	if(unlikely(!drop_permissions(&eixrc))) {
		if(make_fatal) {
			if(!quiet) {
				fprintf(stderr, _("failed to drop permissions\n"));
			}
			return EXIT_FAILURE;
		} else if(!quiet) {
			fprintf(stderr, _("warning: failed to drop permissions\n"));
		}
	}
	execv(argv[0], argv);
	if(!quiet) {
		fprintf(stderr, _("failed to execute %s\n"), argv[0]);
	}
	return EXIT_FAILURE;
}
