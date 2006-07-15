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

#include "../config.h"

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_NEW",
			"[{installedversions}(yellow,1)U{else}(green,1)N{}()]"
			" (green,1)>>() "
			"{system}(yellow){else}(){}<category>()/(default,1)<name>() "
			"\\({best}<best>{else}none{}()){overlaykey} (cyan,1)<overlaykey>(){}: <description>",
			"Define the format used for new packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_DELETE",
			"    (red,1)\\<\\<() "
			"{system}(yellow){else}(){}<category>()/(default,1)<name>() "
			"\\({best}<best>{else}none{}()){overlaykey} (cyan,1)<overlaykey>(){}: <description>",
			"Define the format used for packages that were deleted.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_CHANGED",
			"[{installedversions}{upgrade}(yellow,1)U{else}(red,1)D{}{else}(green,1)N{}()]"
			" (yellow,0)==() "
			"{system}(yellow){else}(){}<category>()/(default,1)<name>() "
			"\\({best}<best>{else}none{}()){overlaykey} (cyan,1)<overlaykey>(){}: <description>",
			"Define the format used for packages that were deleted.")
		);

/* Setting default values for eixrc */
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT",
			"(green)* {system}(yellow){else}(){}<category>()/(default,1)<name> (cyan,1)<overlaykey>()\n"
			"     (green)Available versions:()  <availableversions>\n"
			"     (green)Installed:()           {installedversions}<installedversions>{else}none{}\n"
			"     (green)Homepage:()            <homepage>\n"
			"     (green)Description:()         <description>\n", "Define the format for the normal output of searches.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_COMPACT",
			"[{installedversions}(yellow,1)I{else}(green,1)N{}()] {system}(yellow){else}(){}<category>()/(default,1)<name>() \\((green)<installedversions>()\\): <description>",
			"Define the compact output shown when -c is used.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_VERBOSE",
			"(green,0)* {system}(yellow){else}(){}<category>()/(default,1)<name> (cyan,1)<overlaykey>()\n"
			"     (green)Available versions:()  <availableversions>\n"
			"     (green)Installed:()           {installedversions}<installedversions>{else}none{}\n"
			"     (green)Homepage:()            <homepage>\n"
			"     (green)Description:()         <description>\n"
			"     (green)Provides:()            {provide}<provide>{else}none{}\n"
			"     (green)License:()             <licenses>\n", "Defines the verbose output for eix (-v).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_MASKED",
			"red", "Define color for masked versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_UNSTABLE",
			"yellow", "Define color for unstable versions")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_STABLE",
			"green", "Define color for stable versions")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_OVERLAYKEY",
			"cyan,1", "Color for the overlaykey in version listings.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "MARK_INSTALLED",
			"inverse", "How installed packages are marked in version listings.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "FORCE_USECOLORS",
			"false", "This turns --force-color on for every query.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "STYLE_VERSION_LINES",
			"false", "Allways show version as lines (--version-lines).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::INTEGER, "LEVENSHTEIN_DISTANCE",
			LEVENSHTEIN_DISTANCE_STR,
			"Defines the default maximal levensthein for which a string is considered a match.")
		);

/* Setting default values for eixrc */
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "PORTDIR_CACHE_METHOD",
			PORTDIR_CACHE_METHOD ,"Portage cache-backend that should be used for PORTDIR (flat/cdb/none/backport).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "OVERLAY_CACHE_METHOD",
			"none","Portage cache-backend that should be used for the overlays (flat/cdb/none/backport).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "EXCLUDE_OVERLAY",
			"","List of overlays that should be excluded from the index.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "LOCAL_PORTAGE_CONFIG",
			"true","If false, eix won't read /etc/portage and ACCEPT_KEYWORDS.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DIFF_LOCAL_PORTAGE_CONFIG",
			"true","If false, diff-eix won't read /etc/portage and ACCEPT_KEYWORDS.")
		);

/* fancy new feature: change default matchfield depending on the searchstring. */
#define MATCH_IF(field, value)                                                                       \
	eixrc.addDefault(                                                                               \
			EixRcOption(EixRcOption::STRING, "MATCH_" #field "_IF",                                  \
				value, "Use " #field "as default matchfield if the search string match the given extended regex.") \
			)

MATCH_IF(NAME,          ".*");
MATCH_IF(DESCRIPTION,   ".*");
MATCH_IF(LICENSE,       ".*");
MATCH_IF(CATEGORY,      ".*");
MATCH_IF(CATEGORY_NAME, "/");
MATCH_IF(HOMEPAGE,      ".*");
MATCH_IF(PROVIDE,       "^virtual/");

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "MATCH_ORDER",
			"PROVIDE CATEGORY_NAME NAME","Try the regex from MATCH_(.*)_IF in this order. Use whitespaces as delimiter.")
		);
