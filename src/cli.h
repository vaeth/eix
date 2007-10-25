// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#ifndef __CLI_H__
#define __CLI_H__

#include <search/dbmatchcriteria.h>
#include <eixTk/argsreader.h>
#include <output/formatstring.h>
#include <database/header.h>

class SetStability;
Matchatom *parse_cli(EixRc &eixrc, VarDbPkg &varpkg_db, PortageSettings &portagesettings, const SetStability &stability, const DBHeader &header, MarkedList **marked_list, ArgumentReader::iterator arg, ArgumentReader::iterator end);

/*	If you want to add a new parameter to eix just insert a line into
 *	long_options. If you only want a longopt, add a new define.
 *
 *	-- ebeinroth
 */

enum cli_options {
	O_FMT = 256,
	O_FMT_VERBOSE,
	O_FMT_COMPACT,
	O_PRINT_VAR,
	O_DUMP,
	O_DUMP_DEFAULTS,
	O_CARE,
	O_IGNORE_ETC_PORTAGE,
	O_ONLY_NAMES,
	O_CURRENT,
	O_STABLE_DEFAULT,
	O_TESTING_DEFAULT,
	O_NONMASKED_DEFAULT,
	O_SYSTEM_DEFAULT,
	O_STABLE_LOCAL,
	O_TESTING_LOCAL,
	O_NONMASKED_LOCAL,
	O_SYSTEM_LOCAL,
	O_STABLE_NONLOCAL,
	O_TESTING_NONLOCAL,
	O_NONMASKED_NONLOCAL,
	O_SYSTEM_NONLOCAL,
	O_UPGRADE_LOCAL,
	O_UPGRADE_NONLOCAL,
	O_OVERLAY,
	O_ONLY_OVERLAY,
	O_INSTALLED_OVERLAY,
	O_INSTALLED_SOME,
	O_INSTALLED_WITH_USE,
	O_INSTALLED_WITHOUT_USE,
	O_FROM_OVERLAY,
	O_FETCH,
	O_MIRROR,
	O_EIX_CACHEFILE,
	O_DEBUG
};


#endif /* __CLI_H__ */
