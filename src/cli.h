/***************************************************************************
 *   eix is a small utility for searching ebuilds in the                   *
 *   Gentoo Linux portage system. It uses indexing to allow quick searches *
 *   in package descriptions with regular expressions.                     *
 *                                                                         *
 *   https://sourceforge.net/projects/eix                                  *
 *                                                                         *
 *   Copyright (c)                                                         *
 *     Wolfgang Frisch <xororand@users.sourceforge.net>                    *
 *     Emil Beinroth <emilbeinroth@gmx.net>                                *
 *     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __CLI_H__
#define __CLI_H__

#include <search/dbmatchcriteria.h>
#include <eixTk/argsreader.h>
#include <output/formatstring.h>
#include <database/header.h>

Matchatom *parse_cli(EixRc &eixrc, VarDbPkg &varpkg_db, PortageSettings &portagesettings, const DBHeader &header, MarkedList **marked_list, ArgumentReader::iterator arg, ArgumentReader::iterator end);

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
	O_CURRENT,
	O_OVERLAY,
	O_ONLY_OVERLAY,
	O_INSTALLED_OVERLAY,
	O_INSTALLED_SOME,
	O_INSTALLED_WITH_USE,
	O_INSTALLED_WITHOUT_USE,
	O_FROM_OVERLAY,
	O_EIX_CACHEFILE,
	O_DEBUG
};


#endif /* __CLI_H__ */
