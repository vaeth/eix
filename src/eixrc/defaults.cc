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

#include "../config.h"

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "EIXRC",
			"", "The file which is used instead of /etc/eixrc and ~/.eixrc.\n"
			"This variable can of course only be set in the environment.\n")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EIXRC_SOURCE",
			"", "This path is prepended to source commands in /etc/eixrc.\n"
			"If set in /etc/eixrc it temporarily overrides the environment.\n"
			"When this variable takes effect no delayed substitution is performed.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX",
			EPREFIX_DEFAULT, "If this variable is set in the environment and PORTAGE_CONFIGROOT is unset,\n"
			"then this variable is prefixed to the path where /etc/eixrc is searched."
			"Moreover, this variable is used for delayed substitution for path prefixes.\n"
			"It influences most paths except for $HOME/.eixrc, the cache file\n"
			"passed in the command line, PORTAGE_PROFILE, PORTDIR,\n"
			"and overlays.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "ROOT",
			ROOT_DEFAULT, "This variable is only used for delayed substitution.\n"
			"It influences most paths except for $HOME/.eixrc, the cache file\n"
			"passed in the command line, PORTAGE_PROFILE, PORTDIR,\n"
			"and overlays. In contrast to EPREFIX, further exceptions are:\n"
			"PORTAGE_CONFIGROOT, portage/scripts-internal stuff and the eix cachefile.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_TREE",
			"", "This variable is only used for delayed substitution.\n"
			"It is the path prepended to PORTAGE_PROFILE, PORTDIR, and overlays.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_ROOT",
			"%{??EPREFIX}%{EPREFIX}%{else}%{ROOT}%{}",
			"This variable is only used for delayed substitution.\n"
			"It applies for those paths for which EPREFIX and ROOT should both apply.\n"
			"So you can decide here what to do if both are nonempty. For instance,\n"
			"the choice %{EPREFIX}%{ROOT} will apply both; the default applies EPREFIX\n"
			"but not ROOT for these paths in such a case (i.e. if both are nonempty).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "PORTAGE_CONFIGROOT",
			"%{EPREFIX}", "This path is prepended to the /etc paths.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_PORTAGE_EXEC",
			"%{EPREFIX}", "This prefix is used in connection with external portage tools.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_SOURCE",
			"%{EPREFIX_PORTAGE_EXEC}", "This path is prepended to source commands in /etc/make.{conf,globals}.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_INSTALLED",
			"%{EPREFIX_ROOT}", "Prefix to the path where eix expects information about installed packages.\n")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_PORTAGE_CACHE",
			"%{EPREFIX}", "This prefix is prepended to the portage cache.\n")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_ACCESS_OVERLAYS",
			"", "This prefix is prepended to overlays when their files are accessed.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_PORTDIR",
			"%{EPREFIX_TREE}", "This path is prepended to PORTDIR.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_OVERLAYS",
			"%{EPREFIX_TREE}", "This path is prepended to PORTIDIR_OVERLAY values.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_PORTAGE_PROFILE",
			"%{EPREFIX_TREE}", "This path is prepended to PORTAGE_PROFILE.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::PREFIXSTRING, "EPREFIX_VIRTUAL",
			"%{EPREFIX_TREE}", "This is prepended to overlays in eix database to test whether they exist.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "EIX_CACHEFILE",
			"%{EPREFIX}" EIX_CACHEFILE, "This file is the default eix cache.\n")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "QUICKMODE",
			"false", "Whether --quick is on by default.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "CAREMODE",
			"false", "Whether --care is on.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "QUIETMODE",
			"false", "Whether --quiet is on by default.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DIFF_ONLY_INSTALLED",
			"false", "If true, diff-eix will only consider version changes for installed packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DIFF_NO_SLOTS",
			"false", "If true, diff-eix will not consider slots for version changes.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DEFAULT_FORMAT",
			"normal", "Defines whether --compact or --verbose is on by default.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "PRINT_ALWAYS",
			"false",
			"This variable is only used for delayed substitution.\n"
			"It defines whether all information lines are printed (even if empty).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DIFF_PRINT_INSTALLED",
			"true",
			"This variable is only used for delayed substitution.\n"
			"It defines whether diff-eix will output installed versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_TITLE",
			"green",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for the title texts.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_NAME",
			"default,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the name of packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_CATEGORY",
			"",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the category of packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_CATEGORY_SYSTEM",
			"yellow",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the category of system packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_UPGRADE_TEXT",
			"cyan,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing upgrade recommendation texts.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_DOWNGRADE_TEXT",
			"purple,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing downgrade recommendation texts.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_UPGRADE",
			"%{COLOR_UPGRADE_TEXT};inverse",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing upgrade recommendation tags.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_DOWNGRADE",
			"%{COLOR_DOWNGRADE_TEXT};inverse",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing downgrade recommendation tags.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_COLOR_BETTER",
			"yellow,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for \"better version\" tags (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_COLOR_WORSE",
			"red,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for \"worse version\" tags (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_COLOR_NEW_TAG",
			"%{DIFF_COLOR_NEW}",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for \"new package\" tags (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_COLOR_NEW",
			"%{COLOR_TITLE},1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for \"new package\" separators (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_COLOR_DELETE",
			"red,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for \"deleted package\" separators (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_COLOR_CHANGED",
			"yellow",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for \"changed package\" separators (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_INST_TAG",
			"%{COLOR_TITLE},1;inverse",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for tagging installed packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_UNINST_TAG",
			"%{COLOR_TITLE}",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for tagging uninstalled packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_DATE",
			"purple",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the date.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_SET_USE",
			"red,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the set useflags.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_UNSET_USE",
			"black,1",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the unset useflags.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_INST_VERSION",
			"blue,1;%{MARK_INSTALLED}",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing the version of installed packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_INST_TITLE",
			"blue",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for the title texts for installed versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_MARKED_VERSION",
			"%{COLOR_MARKED_NAME}",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing a marked version of a packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_MARKED_NAME",
			"red,1;%{MARK_VERSIONS}",
			"This variable is only used for delayed substitution.\n"
			"It defines the color used for printing a marked package name.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "CHAR_UPGRADE",
			"U",
			"This variable is only used for delayed substitution.\n"
			"It defines the character printed for upgrade recommendations.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "CHAR_DOWNGRADE",
			"D",
			"This variable is only used for delayed substitution.\n"
			"It defines the character printed for downgrade recommendations.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "CHAR_INSTALLED",
			"I",
			"This variable is only used for delayed substitution.\n"
			"It defines the character printed for installed packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "CHAR_UNINSTALLED",
			"%{DIFF_CHAR_NEW}",
			"This variable is only used for delayed substitution.\n"
			"It defines the character printed for uninstalled packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_CHAR_NEW",
			"N",
			"This variable is only used for delayed substitution.\n"
			"It defines the character printed for new packages (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_CHAR_BETTER",
			">",
			"This variable is only used for delayed substitution.\n"
			"It defines the character used for \"better version\" tags (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_CHAR_WORSE",
			"\\<",
			"This variable is only used for delayed substitution.\n"
			"It defines the character used for \"worse version\" tags (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "STRING_PLAIN_INSTALLED",
			"%{STRING_PLAIN_UNINSTALLED}",
			"This variable is only used for delayed substitution.\n"
			"It defines the string used for \"plain\" tagging installed packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "STRING_PLAIN_UNINSTALLED",
			"*",
			"This variable is only used for delayed substitution.\n"
			"It defines the string used for \"plain\" tagging unintalled packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_STRING_NEW",
			">>",
			"This variable is only used for delayed substitution.\n"
			"It defines the string used for \"new package\" separators (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_STRING_DELETE",
			"\\<\\<",
			"This variable is only used for delayed substitution.\n"
			"It defines the string used for \"deleted package\" separators (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_STRING_CHANGED",
			"==",
			"This variable is only used for delayed substitution.\n"
			"It defines the string used for \"changed package\" separators (diff-eix).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "INSTALLEDVERSIONS",
			"<installedversions"
			":(%{COLOR_INST_VERSION}):():()::"
			":(%{COLOR_DATE})\\(:\\)()"
			":\\(:\\)"
			":(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()>",
			"This variable is only used for delayed substitution.\n"
			"It defines the format used for printing installed versions normally.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "INSTALLEDVERSIONS_COMPACT",
			"<installedversionsshortdate"
			":(%{COLOR_INST_VERSION}):():():():"
			":@(%{COLOR_DATE}):()"
			":\\(:\\)"
			":(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()>",
			"This variable is only used for delayed substitution.\n"
			"It defines the format used for printing installed versions compactly.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "INSTALLEDVERSIONS_VERBOSE",
			"<installedversions"
			":(%{COLOR_INST_TITLE})Version\\\\:() (%{COLOR_INST_VERSION}):():():():"
			":\\n                          (%{COLOR_INST_TITLE})Date\\\\:()    "
			"(%{COLOR_DATE}):()"
			":\\n                          (%{COLOR_INST_TITLE})USE\\\\:()     "
			"::(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()"
			":\\n                          >",
			"This variable is only used for delayed substitution.\n"
			"It defines the format used for printing installed versions verbosely.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_INSTALLEDVERSIONS",
			"%{!PRINT_ALWAYS}{installedversionsshort}%{}"
			"     (%{COLOR_TITLE})Installed versions:()"
			"  "
			"%{?PRINT_ALWAYS}{installedversionsshort}%{}"
			"%{INSTALLEDVERSIONS}"
			"%{?PRINT_ALWAYS}"
				"{else}None{}\\n"
			"%{else}"
				"\\n{}"
			"%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the \"normal\" format for a line with installed versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_INSTALLEDVERSIONS_VERBOSE",
			"%{!PRINT_ALWAYS}{installedversionsshort}%{}"
			"     (%{COLOR_TITLE})Installed versions:()"
			"  "
			"%{?PRINT_ALWAYS}{installedversionsshort}%{}"
			"%{INSTALLEDVERSIONS_VERBOSE}"
			"%{?PRINT_ALWAYS}"
				"{else}None{}\\n"
			"%{else}"
				"\\n{}"
			"%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the verbose format for a line with installed versions.")
		);


eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE_INSTALLEDVERSIONS",
			"{installedversionsshort}%{INSTALLEDVERSIONS_COMPACT}; {}"
			"",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for diff-eix for installed versions.")
		);


eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_NAME",
			"{system}(%{COLOR_CATEGORY_SYSTEM}){else}(%{COLOR_CATEGORY}){}<category>()/{marked}(%{COLOR_MARKED_NAME}){else}(%{COLOR_NAME}){}<name>()",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the printing the package name.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_HEADER",
			"{installedversionsshort}["
			"{upgrade}(%{COLOR_UPGRADE})%{CHAR_UPGRADE}{}"
			"{downgrade}(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}{}"
			"{!recommend}(%{COLOR_INST_TAG})%{CHAR_INSTALLED}{}()"
			"]{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the normal header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_HEADER_VERBOSE",
			"{installedversionsshort}(%{COLOR_INST_TAG})%{STRING_PLAIN_INSTALLED}"
			"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}"
			"()",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the verbose header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_HEADER_COMPACT",
			"[{installedversionsshort}"
				"{recommend}"
					"{upgrade}(%{COLOR_UPGRADE})%{CHAR_UPGRADE}{}"
					"{downgrade}(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}{}"
				"{else}"
					"{!recommend}(%{COLOR_INST_TAG})%{CHAR_INSTALLED}{}()"
				"{}"
			"{else}"
				"(%{COLOR_UNINST_TAG})%{CHAR_UNINSTALLED}"
			"{}()]",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the compact header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_HEADER_NEW",
			"["
			"{upgrade}(%{COLOR_UPGRADE})%{CHAR_UPGRADE}{}"
			"{downgrade}(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}{}"
			"{!recommend}(%{DIFF_COLOR_NEW_TAG})%{DIFF_CHAR_NEW}{}()"
			"()]"
			"{upgrade}"
				"{!downgrade} {}"
			"{else}"
				" "
			"{}"
			" (%{DIFF_COLOR_NEW})%{DIFF_STRING_NEW}()",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the diff-new header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_HEADER_DELETE",
			"{installedversionsshort}"
				"[(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()]"
			"{else}"
				"   "
			"{}"
			"  (%{DIFF_COLOR_DELETE})%{DIFF_STRING_DELETE}()",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the diff-delete header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_HEADER_CHANGED",
			"["
			"{upgrade}(%{COLOR_UPGRADE})%{CHAR_UPGRADE}{}"
			"{downgrade}(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}{}"
			"{recommend}(){}"
			"{!upgrade}{better}(%{DIFF_COLOR_BETTER})%{DIFF_CHAR_BETTER}{}{}"
			"{!downgrade}{worse}(%{DIFF_COLOR_WORSE})%{DIFF_CHAR_WORSE}{}{}"
			"{!recommend}(){}"
			"]"
			"{!upgrade}{!better} {}{}"
			"{!downgrade}{!worse} {}{}"
			" (%{DIFF_COLOR_CHANGED})%{DIFF_STRING_CHANGED}()",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the diff-changed header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_BESTSLOTS_COMPACT",
			"{bestshort}<bestslots>{else}<availableversions>{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the compact format for the best versions/slots.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_BESTSLOTS",
			"%{FORMAT_BESTSLOTS_COMPACT}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the diff-eix format for the best versions/slots.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_CHANGED_VERSIONS",
			"%{?DIFF_PRINT_INSTALLED}"
				"%{DIFF_FORMATLINE_INSTALLEDVERSIONS}"
			"%{}"
			"{oldbestshort}"
				"<oldbestslots>"
			"{else}"
				"<oldavailableversions>"
			"{}"
			"() -> %{DIFF_FORMAT_BESTSLOTS}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the diff-eix format for changed versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_OVERLAYKEY",
			"{overlaykey} <overlaykey>{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the printing the optional overlay key.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_NAME",
			"%{FORMAT_HEADER} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the normal header line.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_NAME_VERBOSE",
			"%{FORMAT_HEADER_VERBOSE} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the verbose header line.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_NAME_COMPACT",
			"%{FORMAT_HEADER_COMPACT} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the compact header line.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE_NAME_NEW",
			"%{DIFF_FORMAT_HEADER_NEW} %{FORMAT_NAME} ",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the diff-new header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE_NAME_DELETE",
			"%{DIFF_FORMAT_HEADER_DELETE} %{FORMAT_NAME} ",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the diff-delete header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE_NAME_CHANGED",
			"%{DIFF_FORMAT_HEADER_CHANGED} %{FORMAT_NAME} ",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for the diff-changed header symbols.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_AVAILABLEVERSIONS",
			"     (%{COLOR_TITLE})Available versions:()  <availableversions>\\n",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with installed versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE_BESTSLOTS",
			"\\(%{DIFF_FORMAT_BESTSLOTS}())",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the diff-eix line for the best versions/slots.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE_CHANGED_VERSIONS",
			"\\(%{DIFF_FORMAT_CHANGED_VERSIONS})",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the diff-eix line for changed versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_MARKEDVERSIONS",
			"%{!PRINT_ALWAYS}{marked}%{}"
			"     (%{COLOR_TITLE})Marked:()"
			"%{?PRINT_ALWAYS}{marked}%{}"
			"              "
			"(%{COLOR_MARKED_VERSION})<markedversions>()"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with marked versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_HOMEPAGE",
			"%{!PRINT_ALWAYS}{homepage}%{}"
			"     (%{COLOR_TITLE})Homepage:()"
			"%{?PRINT_ALWAYS}{homepage}%{}"
			"            "
			"<homepage>"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with the package homepage.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_DESCRIPTION",
			"%{!PRINT_ALWAYS}{description}%{}"
			"     (%{COLOR_TITLE})Description:()"
			"%{?PRINT_ALWAYS}{description}%{}"
			"         "
			"<description>"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with the package description.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_BESTSLOTS",
			"%{!PRINT_ALWAYS}{bestshort}%{}"
			"     (%{COLOR_TITLE})Best versions/slot:()"
			"%{?PRINT_ALWAYS}{bestshort}%{}"
			"  "
			"<bestslots>"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with the best versions/slots.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_RECOMMEND",
			"%{!PRINT_ALWAYS}{recommend}%{}"
			"     (%{COLOR_TITLE})Recommendation:()"
			"%{?PRINT_ALWAYS}{recommend}%{}"
			"      "
			"{upgrade}(%{COLOR_UPGRADE_TEXT})Upgrade()"
				"{downgrade}"
					" and "
				"{}"
			"{}"
			"{downgrade}(%{COLOR_DOWNGRADE_TEXT})Downgrade(){}"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with the up-/downgrade recommendations.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_PROVIDE",
			"%{!PRINT_ALWAYS}{provide}%{}"
			"     (%{COLOR_TITLE})Provides:()"
			"%{?PRINT_ALWAYS}{provide}%{}"
			"            "
			"<provide>"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with the package provides.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMATLINE_LICENSES",
			"%{!PRINT_ALWAYS}{licenses}%{}"
			"     (%{COLOR_TITLE})License:()"
			"%{?PRINT_ALWAYS}{licenses}%{}"
			"             "
			"<licenses>"
			"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for a line with the package licenses.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMATLINE",
			"%{FORMAT_OVERLAYKEY}: <description>",
			"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
			"It defines the format for diff-eix after the versions.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT",
			"%{FORMATLINE_NAME}"
			"%{FORMATLINE_AVAILABLEVERSIONS}"
			"%{FORMATLINE_INSTALLEDVERSIONS}"
			"%{FORMATLINE_MARKEDVERSIONS}"
			"%{FORMATLINE_HOMEPAGE}"
			"%{FORMATLINE_DESCRIPTION}",
			"Define the format for the normal output of searches.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_COMPACT",
			"%{FORMATLINE_NAME_COMPACT}"
			" \\({marked}(%{COLOR_MARKED_VERSION})<markedversions>(); {}"
			"{installedversionsshort}"
				"%{INSTALLEDVERSIONS_COMPACT}"
				"{recommend} -> %{FORMAT_BESTSLOTS_COMPACT}{}"
			"{else}"
				"%{FORMAT_BESTSLOTS_COMPACT}"
			"{}"
			"()\\): <description>",
			"Define the compact output shown when -c is used.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_VERBOSE",
			"%{FORMATLINE_NAME_VERBOSE}"
			"%{FORMATLINE_AVAILABLEVERSIONS}"
			"%{FORMATLINE_INSTALLEDVERSIONS_VERBOSE}"
			"%{FORMATLINE_BESTSLOTS}"
			"%{FORMATLINE_RECOMMEND}"
			"%{FORMATLINE_MARKEDVERSIONS}"
			"%{FORMATLINE_HOMEPAGE}"
			"%{FORMATLINE_DESCRIPTION}"
			"%{FORMATLINE_PROVIDE}"
			"%{FORMATLINE_LICENSES}",
			"Defines the verbose output for eix (-v).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_NEW",
			"%{DIFF_FORMATLINE_NAME_NEW}"
			"%{DIFF_FORMATLINE_BESTSLOTS}"
			"%{DIFF_FORMATLINE}",
			"Define the format used for new packages.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_DELETE",
			"%{DIFF_FORMATLINE_NAME_DELETE}"
			"%{DIFF_FORMATLINE_BESTSLOTS}"
			"%{DIFF_FORMATLINE}",
			"Define the format used for packages that were deleted.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DIFF_FORMAT_CHANGED",
			"%{DIFF_FORMATLINE_NAME_CHANGED}"
			"%{DIFF_FORMATLINE_CHANGED_VERSIONS}"
			"%{DIFF_FORMATLINE}",
			"Define the format used for packages that were deleted.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_INSTALLATION_DATE",
			"%X %x",
			"strftime() format for printing the installation date in long form")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "FORMAT_SHORT_INSTALLATION_DATE",
			"%x",
			"strftime() format for printing the installation date in short form")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "FORMAT_INSTALLED_USE",
			"%s", "Printf-like format for printing useflags for installed packages")
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
		EixRcOption(EixRcOption::STRING, "COLOR_VIRTUALKEY",
			"purple,1", "Color for the overlaykey for virtual overlays in version listings.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "COLOR_SLOTS",
			"red,1", "Color for slots.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "MARK_INSTALLED",
			"inverse", "How installed packages are marked in version listings.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "MARK_VERSIONS",
			"underline", "How the package versions passed with --pipe are marked in version listings.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "FORCE_USECOLORS",
			"false", "This turns --force-color on for every query.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "COLOR_ORIGINAL",
			"true", "If false, versions are only colored according to the local setting.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "COLOR_LOCAL_MASK",
			"false", "If false, COLOR_ORIGINAL=false has no effect on versions which are only locally masked (i.e. [m]).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "STYLE_VERSION_SORTED",
			"false", "Defines whether --versionsorted is on by default.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "STYLE_VERSION_LINES",
			"false", "Defines whether --versionlines is on by default.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "COLORED_SLOTS",
			"true", "If false, the slotnames appended to versions are not colored.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "COLON_SLOTS",
			"false", "If true, slots are separated from the version names with a colon instead of braces.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "SMALL_EIX_DATABASE",
			"false", "Keep database small at the cost of never honoring keywords of other ARCH")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DEFAULT_IS_OR",
			"false", "Whether default concatenation of queries is -o (or) or -a (and)")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DUP_PACKAGES_ONLY_OVERLAYS",
			"false", "Whether checks for duplicate packages occur only among overlays")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "DUP_VERSIONS_ONLY_OVERLAYS",
			"false", "Whether checks for duplicate versions occur only among overlays")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "OVERLAYS_LIST",
			"all-used-renumbered", "Which overlays to list (all/all-if-used/all-used/all-used-renumbered/no)")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::INTEGER, "LEVENSHTEIN_DISTANCE",
			LEVENSHTEIN_DISTANCE_STR,
			"Defines the default maximal levensthein for which a string is considered a match.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "PORTDIR_CACHE_METHOD",
			PORTDIR_CACHE_METHOD , "Portage cache-backend that should be used for PORTDIR (metadata/flat/cdb/none/backport/eix[:file[:nr]]")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "OVERLAY_CACHE_METHOD",
			"none", "Portage cache-backend that should be used for the overlays (metadata/flat/cdb/none/backport/eix[:file[:nr]]).")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "CACHE_METHOD",
			"", "Overrides OVERLAY_CACHE_METHOD or PORTDIR_CACHE_METHOD for certain directories.\n"
			"This is a list of pairs DIR-PATTERN METHOD. Later entries take precedence.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "ADD_CACHE_METHOD",
			"", "This variable is added to CACHE_METHOD.\n"
			"This variable is meant to be set only locally.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "OVERRIDE_CACHE_METHOD",
			"", "This variable can override the choices of CACHE_METHOD/ADD_CACHE_METHOD\n"
			"and in addition it can override the choices made by KEEP_VIRTUALS.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "ADD_OVERRIDE_CACHE_METHOD",
			"", "This variable is added to OVERRIDE_CACHE_METHOD.\n"
			"This variable is meant to be set only locally.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "EXCLUDE_OVERLAY",
			"", "List of overlays that should be excluded from the index.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "ADD_OVERLAY",
			"", "List of overlays that should be added to the index.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "KEEP_VIRTUALS",
			"false", "Keep virtuals of the old cache file by adding corresponding entries\n"
			"implicitly to the values of ADD_OVERLAY and ADD_CACHE_METHOD")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "LOCAL_PORTAGE_CONFIG",
			"true", "If false, /etc/portage and ACCEPT_KEYWORDS are ignored.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "UPGRADE_ALWAYS_LOCAL",
			"true", "if true, eix -u will match according to local settings")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "RECOMMEND_ALWAYS_LOCAL",
			"true", "if true, up-/downgrade recommendations accord always to local settings")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "UPGRADE_TO_HIGHEST_SLOT",
			"true", "If true, upgrade tests succeed for installed packages with new higher slots")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "PRINT_SLOTS",
			"true", "If false, no slot information is printed.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "CHECK_INSTALLED_OVERLAYS",
			"true", "If true, always check from which overlay a package was installed.\n"
			"Otherwise, only packages appearing in different overlays are checked.\n"
			"This is faster but may sometimes give wrong results.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "PRINT_COUNT_ALWAYS",
			"false", "If true, always print the number of matches in the last line.")
		);

#define TAG_FOR(type, default, comment) \
	eixrc.addDefault( \
		EixRcOption(EixRcOption::STRING, "TAG_FOR_" #type, \
			default, "Tag for " #comment " versions.") \
		);
TAG_FOR(PROFILE, "[P]", "profile masked")
TAG_FOR(MASKED, "[M]", "package.masked")
TAG_FOR(EX_PROFILE, "{P}", "originally profile masked but unmasked")
TAG_FOR(EX_MASKED, "{M}", "originally package.masked but unmasked")
TAG_FOR(LOCALLY_MASKED, "[m]", "only locally masked")
TAG_FOR(STABLE, "", "stable")
TAG_FOR(UNSTABLE, "~", "unstable")
TAG_FOR(MINUS_ASTERISK, "-*", "-*")
TAG_FOR(MINUS_KEYWORD, "-", "-ARCH")
TAG_FOR(ALIEN_STABLE, "*", "ALIENARCH")
TAG_FOR(ALIEN_UNSTABLE, "~*", "~ALIENARCH")
TAG_FOR(MISSING_KEYWORD, "**", "no keyword")
TAG_FOR(EX_UNSTABLE, "(~)", "originally unstable but now stable")
TAG_FOR(EX_MINUS_ASTERISK, "(-*)", "originally -* but now stable")
TAG_FOR(EX_MINUS_KEYWORD, "(-)", "originally -ARCH but now stable")
TAG_FOR(EX_ALIEN_STABLE, "(*)", "originally ALIENARCH but now stable")
TAG_FOR(EX_ALIEN_UNSTABLE, "(~*)", "originally ~ALIENARCH but now stable")
TAG_FOR(EX_MISSING_KEYWORD, "(**)", "originally no keyword but now stable")

/* fancy new feature: change default matchfield depending on the searchstring. */
#define MATCH_IF(field, value) \
	eixrc.addDefault( \
			EixRcOption(EixRcOption::STRING, "MATCH_" #field "_IF", \
				value, "Use " #field " as default matchfield if the search string match the given extended regex.") \
			)

MATCH_IF(NAME,          ".*");
MATCH_IF(DESCRIPTION,   ".*");
MATCH_IF(LICENSE,       ".*");
MATCH_IF(CATEGORY,      ".*");
MATCH_IF(CATEGORY_NAME, "/");
MATCH_IF(HOMEPAGE,      ".*");
MATCH_IF(PROVIDE,       "^virtual/");
MATCH_IF(IUSE,          ".*");

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "MATCH_ORDER",
			"PROVIDE CATEGORY_NAME NAME", "Try the regex from MATCH_(.*)_IF in this order. Use whitespaces as delimiter.")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "TEST_FOR_EMPTY",
			"true", "Defines whether empty entries in /etc/portage/package.* are shown with -t.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "TEST_FOR_NONEXISTENT",
			"true", "Defines whether non-existing installed versions are positive for -T.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "NONEXISTENT_IF_OTHER_OVERLAY",
			"true", "Defines whether versions are non-existent for TEST_FOR_NONEXISTENT\n"
			"if they come from a different overlay than the installed version.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "NONEXISTENT_IF_MASKED",
			"true", "Defines whether masked versions are non-existent for TEST_FOR_NONEXISTENT.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::BOOLEAN, "TEST_FOR_REDUNDANCY",
			"true", "Defines whether redundant entries are positive for -T.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_DOUBLE",
			"some",
			"Applies if /etc/portage/package.keywords lists the same keyword twice\n"
			"for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_DOUBLE_LINE",
			"some",
			"Applies if /etc/portage/package.keywords has two lines for identical target.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_MIXED",
			"some",
			"Applies if /etc/portage/package.keywords lists two different keywords,\n"
			"e.g. ~ARCH and -*, for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_WEAKER",
			"all-installed",
			"Applies if /etc/portage/package.keywords lists a keywords which can\n"
			"be replaced by a weaker keyword, e.g. -* or ~OTHERARCH or OTHERARCH\n"
			"in place of ~ARCH, or ~OTHERARCH in place of OTHERARCH,\n"
			"for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_STRANGE",
			"some",
			"Applies if /etc/portage/package.keywords lists a strange keyword\n"
			"e.g. UNKNOWNARCH (unknown to the .ebuild) or -OTHERARCH,\n"
			"for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_NO_CHANGE",
			"all-installed",
			"Applies if /etc/portage/package.keywords provides keywords which do not\n"
			"change the availability keywords status for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_MASK_NO_CHANGE",
			"all-uninstalled",
			"Applies if /etc/portage/package.mask contains entries\n"
			"which do not change the mask status for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_UNMASK_NO_CHANGE",
			"all-installed",
			"Applies if /etc/portage/package.unmask contains entries\n"
			"which do not change the mask status for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_DOUBLE_MASKED",
			"some",
			"Applies if /etc/portage/package.mask matches twice\n"
			"for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_DOUBLE_UNMASKED",
			"some",
			"Applies if /etc/portage/package.unmask matches twice\n"
			"for the versions in question.")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_DOUBLE_USE",
			"some",
			"Applies if /etc/portage/package.use matches twice")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_DOUBLE_CFLAGS",
			"some",
			"Applies if /etc/portage/package.cflags matches twice")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_IN_KEYWORDS",
			"-some",
			"Applies if /etc/portage/package.keywords contains a matching entry")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_IN_MASK",
			"-some",
			"Applies if /etc/portage/package.mask matches")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_IN_UNMASK",
			"-some",
			"Applies if /etc/portage/package.mask matches")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_IN_USE",
			"-some",
			"Applies if /etc/portage/package.use matches")
		);
eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "REDUNDANT_IF_IN_CFLAGS",
			"-some",
			"Applies if /etc/portage/package.cflags matches")
		);

eixrc.addDefault(
		EixRcOption(EixRcOption::STRING, "DUMMY",
			"",
			"This variable is ignored. You can use it to collect delayed references to\n"
			"locally added (unused) variables so that they are printed with --dump.")
		);
