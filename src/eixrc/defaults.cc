// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#define AddOption(opt_type, opt_name, opt_default, opt_description) \
	eixrc.addDefault(EixRcOption(EixRcOption::opt_type, opt_name, \
		opt_default, opt_description))

#if (DEFAULT_PART == 1)

AddOption(STRING, "EIXRC",
	"", "The file which is used instead of /etc/eixrc and ~/.eixrc.\n"
	"This variable can of course only be set in the environment.");

AddOption(PREFIXSTRING, "EIXRC_SOURCE",
	"", "This path is prepended to source commands in /etc/eixrc.\n"
	"If set in /etc/eixrc it temporarily overrides the environment.\n"
	"You must not use delayed substitution in this variable.");

AddOption(PREFIXSTRING, "EPREFIX",
	EPREFIX_DEFAULT, "If this variable is set in the environment and PORTAGE_CONFIGROOT is unset,\n"
	"then this variable is prefixed to the path where /etc/eixrc is searched.\n"
	"Moreover, this variable is used for delayed substitution for path prefixes.\n"
	"It influences most paths except for $HOME/.eixrc, the cache file\n"
	"passed in the command line, PORTAGE_PROFILE, PORTDIR, and overlays.");

AddOption(PREFIXSTRING, "ROOT",
	ROOT_DEFAULT, "This variable is only used for delayed substitution.\n"
	"It influences most paths except for $HOME/.eixrc, the cache file\n"
	"passed in the command line, PORTAGE_PROFILE, PORTDIR,\n"
	"and overlays. In contrast to EPREFIX, further exceptions are:\n"
	"PORTAGE_CONFIGROOT, portage/scripts-internal stuff and the eix cachefile.");

AddOption(PREFIXSTRING, "EPREFIX_TREE",
	"", "This variable is only used for delayed substitution.\n"
	"It is the path prepended to PORTAGE_PROFILE, PORTDIR, and overlays.");

AddOption(PREFIXSTRING, "EPREFIX_ROOT",
	"%{??EPREFIX}%{EPREFIX}%{else}%{ROOT}%{}",
	"This variable is only used for delayed substitution.\n"
	"It applies for those paths for which EPREFIX and ROOT should both apply.\n"
	"So you can decide here what to do if both are nonempty. For instance,\n"
	"the choice %{EPREFIX}%{ROOT} will apply both; the default applies EPREFIX\n"
	"but not ROOT for these paths in such a case (i.e. if both are nonempty).");

AddOption(PREFIXSTRING, "PORTAGE_CONFIGROOT",
	"%{EPREFIX}", "This path is prepended to the /etc paths.");

AddOption(PREFIXSTRING, "MAKE_GLOBALS",
	"%{EPREFIX}/usr/share/portage/config/make.globals",
	"This file is used instead of %{PORTAGE_CONFIGROOT}/etc/make.globals\n"
	"if it exists. This is reasonable for >=portage-2.2*");

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_EXEC",
	"%{EPREFIX}", "This prefix is used in connection with external portage tools.");

AddOption(PREFIXSTRING, "EPREFIX_SOURCE",
	"%{EPREFIX_PORTAGE_EXEC}", "This path is prepended to source commands in /etc/make.{conf,globals}.");

AddOption(PREFIXSTRING, "EPREFIX_INSTALLED",
	"%{EPREFIX_ROOT}", "Prefix to the path where eix expects information about installed packages.");

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_CACHE",
	"%{EPREFIX}", "This prefix is prepended to the portage cache.");

AddOption(PREFIXSTRING, "EPREFIX_ACCESS_OVERLAYS",
	"", "This prefix is prepended to overlays when their files are accessed.");

AddOption(PREFIXSTRING, "EPREFIX_PORTDIR",
	"%{EPREFIX_TREE}", "This path is prepended to PORTDIR.");

AddOption(PREFIXSTRING, "EPREFIX_OVERLAYS",
	"%{EPREFIX_TREE}", "This path is prepended to PORTIDIR_OVERLAY values.");

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_PROFILE",
	"%{EPREFIX_TREE}", "This path is prepended to PORTAGE_PROFILE.");

AddOption(PREFIXSTRING, "EPREFIX_VIRTUAL",
	"%{EPREFIX_TREE}", "This is prepended to overlays in eix database to test whether they exist.");

AddOption(STRING, "EIX_CACHEFILE",
	"%{EPREFIX}" EIX_CACHEFILE, "This file is the default eix cache.");

AddOption(STRING, "EIX_WORLD",
	"%{EPREFIX_ROOT}/var/lib/portage/world", "This file is considered as the world file.");

AddOption(STRING, "EIX_WORLD_SETS",
	"%{EIX_WORLD}_sets", "This file is considered as the world_sets file.");

AddOption(STRING, "EIX_LOCAL_SETS",
	"%{PORTAGE_CONFIGROOT}/etc/portage/sets", "This directory contains the locally defined sets.");

AddOption(BOOLEAN, "SAVE_WORLD",
	"false", "Store the information of the world file in the cache file.\n"
	"Set this only if you want that everybody is be able to get this informations.");

AddOption(BOOLEAN, "CURRENT_WORLD",
	"true", "Prefer the current world file (if readable) over the data in the cachefile.");

AddOption(BOOLEAN, "SKIP_PERMISSION_TESTS",
	"false", "Whether to test for group and permissions.  You must set this to true\n"
	"if you use more sophisticated permission setups (e.g. NSS/LDAP).");

AddOption(STRING, "EBUILD_USER",
	"portage", "The user which is used for running bash on ebuilds when\n"
	"cache method ebuild or ebuild* is used. See EBUILD_UID.");

AddOption(STRING, "EBUILD_GROUP",
	"%{EBUILD_USER}", "The group which is used for running bash on ebuilds when\n"
	"cache method ebuild or ebuild* is used. See EBUILD_UID.");

AddOption(STRING, "EBUILD_UID",
	"250", "If EBUILD_USER is empty or nonexistent, use this user id.\n"
	"In this case and if ${EBUILD_UID} <= 0, the user id is not changed.");

AddOption(STRING, "EBUILD_GID",
	"%{EBUILD_UID}", "If EBUILD_GROUP is empty or nonexistent, use this group id.\n"
	"In this case and if ${EBUILD_UID} <= 0, the group id is not changed.");

AddOption(BOOLEAN, "QUICKMODE",
	"false", "Whether --quick is on by default.");

AddOption(BOOLEAN, "CAREMODE",
	"false", "Whether --care is on.");

AddOption(BOOLEAN, "QUIETMODE",
	"false", "Whether --quiet is on by default.");

AddOption(BOOLEAN, "DIFF_ONLY_INSTALLED",
	"false", "If true, diff-eix will only consider version changes for installed packages.");

AddOption(BOOLEAN, "DIFF_NO_SLOTS",
	"false", "If true, diff-eix will not consider slots for version changes.");

AddOption(BOOLEAN, "DIFF_SEPARATE_DELETED",
	"true", "If false, diff-eix will mix deleted and changed packages");

AddOption(BOOLEAN, "DIFF_PRINT_HEADER",
	"true", "Should diff-eix print a header info line?");

AddOption(BOOLEAN, "NO_RESTRICTIONS",
	"false", "If false, fetch and mirror restrictions are output.");

AddOption(BOOLEAN, "RESTRICT_INSTALLED",
	"true", "If true, calculate fetch and mirror restrictions for installed versions.");

AddOption(BOOLEAN, "CARE_RESTRICT_INSTALLED",
	"true", "If true, read fetch and mirror restrictions for installed versions\n"
	"always from disk. This is ignored if RESTRICT_INSTALLED=false.");

AddOption(STRING, "DEFAULT_FORMAT",
	"normal", "Defines whether --compact or --verbose is on by default.");

AddOption(BOOLEAN, "PRINT_ALWAYS",
	"false",
	"This variable is only used for delayed substitution.\n"
	"It defines whether all information lines are printed (even if empty).");

AddOption(BOOLEAN, "PRINT_BUGS",
	"false",
	"This variable is only used for delayed substitution.\n"
	"It defines whether a bug reference is printed in verbose format.");

AddOption(BOOLEAN, "DIFF_PRINT_INSTALLED",
	"true",
	"This variable is only used for delayed substitution.\n"
	"It defines whether diff-eix will output installed versions.");

AddOption(STRING, "COLOR_TITLE",
	"green",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts.");

AddOption(STRING, "COLOR_NAME",
	"default,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the name of packages.");

AddOption(STRING, "COLOR_WORLD",
	"green,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the name of world packages.");

AddOption(STRING, "COLOR_WORLD_SETS",
	"yellow,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the name of world sets packages.");

AddOption(STRING, "COLOR_CATEGORY",
	"",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of packages.");

AddOption(STRING, "COLOR_CATEGORY_SYSTEM",
	"yellow",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of system packages.");

AddOption(STRING, "COLOR_CATEGORY_WORLD",
	"%{COLOR_WORLD}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of world packages.");

AddOption(STRING, "COLOR_CATEGORY_WORLD_SETS",
	"%{COLOR_WORLD_SETS}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of world sets packages.");

AddOption(STRING, "COLOR_UPGRADE_TEXT",
	"cyan,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation texts.");

AddOption(STRING, "COLOR_DOWNGRADE_TEXT",
	"purple,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation texts.");

AddOption(STRING, "COLOR_UPGRADE",
	"%{COLOR_UPGRADE_TEXT};inverse",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation tags.");

AddOption(STRING, "COLOR_DOWNGRADE",
	"%{COLOR_DOWNGRADE_TEXT};inverse",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation tags.");

AddOption(STRING, "DIFF_COLOR_UNINST_STABILIZE",
	"%{COLOR_STABLE},1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for uninstalled packages\n"
	"which have gained a stable version.");

AddOption(STRING, "DIFF_COLOR_INST_STABILIZE",
	"%{DIFF_COLOR_UNINST_STABILIZE},1;inverse",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for installed packages\n"
	"which have gained a stable version.");

AddOption(STRING, "DIFF_COLOR_BETTER",
	"yellow,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"better version\" tags (diff-eix).");

AddOption(STRING, "DIFF_COLOR_WORSE",
	"red,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"worse version\" tags (diff-eix).");

AddOption(STRING, "DIFF_COLOR_NEW_TAG",
	"%{DIFF_COLOR_NEW}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" tags (diff-eix).");

AddOption(STRING, "DIFF_COLOR_NEW",
	"%{COLOR_TITLE},1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" separators (diff-eix).");

AddOption(STRING, "DIFF_COLOR_DELETE",
	"red,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"deleted package\" separators (diff-eix).");

AddOption(STRING, "DIFF_COLOR_CHANGED",
	"yellow",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"changed package\" separators (diff-eix).");

AddOption(STRING, "COLOR_INST_TAG",
	"%{COLOR_TITLE},1;inverse",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging installed packages.");

AddOption(STRING, "COLOR_UNINST_TAG",
	"%{COLOR_TITLE}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging uninstalled packages.");

AddOption(STRING, "COLOR_DATE",
	"purple",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the date.");

AddOption(STRING, "COLOR_SET_USE",
	"red,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the set useflags.");

AddOption(STRING, "COLOR_UNSET_USE",
	"black,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the unset useflags.");

AddOption(STRING, "COLOR_INST_VERSION",
	"blue,1;%{MARK_INSTALLED}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the version of installed packages.");

AddOption(STRING, "COLOR_INST_TITLE",
	"blue",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for installed versions.");

AddOption(STRING, "COLOR_MARKED_VERSION",
	"%{COLOR_MARKED_NAME}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked version of a packages.");

AddOption(STRING, "COLOR_PACKAGESETS",
	"yellow,1",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the package sets.");

AddOption(STRING, "COLOR_MARKED_NAME",
	"red,1;%{MARK_VERSIONS}",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked package name.");

#endif

#if (DEFAULT_PART == 2)

AddOption(STRING, "CHAR_UPGRADE",
	"U",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for upgrade recommendations.");

AddOption(STRING, "CHAR_DOWNGRADE",
	"D",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for downgrade recommendations.");

AddOption(STRING, "CHAR_INSTALLED",
	"I",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for installed packages.");

AddOption(STRING, "CHAR_UNINSTALLED",
	"%{DIFF_CHAR_NEW}",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for uninstalled packages.");

AddOption(STRING, "DIFF_CHAR_UNINST_STABILIZE",
	"%{DIFF_CHAR_INST_STABILIZE}",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for uninstalled packages which have gained\n"
	"a stable version.");

AddOption(STRING, "DIFF_CHAR_INST_STABILIZE",
	"*",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for installed packages which have gained\n"
	"a stable version.");

AddOption(STRING, "DIFF_CHAR_NEW",
	"N",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for new packages (diff-eix).");

AddOption(STRING, "DIFF_CHAR_BETTER",
	">",
	"This variable is only used for delayed substitution.\n"
	"It defines the character used for \"better version\" tags (diff-eix).");

AddOption(STRING, "DIFF_CHAR_WORSE",
	"\\<",
	"This variable is only used for delayed substitution.\n"
	"It defines the character used for \"worse version\" tags (diff-eix).");

AddOption(STRING, "TAG_UPGRADE",
	"(%{COLOR_UPGRADE})%{CHAR_UPGRADE}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for upgrade recommendations.");

AddOption(STRING, "TAG_DOWNGRADE",
	"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations.");

AddOption(STRING, "TAG_DOWNGRADE",
	"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations.");

AddOption(STRING, "TAG_INSTALLED",
	"(%{COLOR_INST_TAG})%{CHAR_INSTALLED}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations.");

AddOption(STRING, "TAG_UNINSTALLED",
	"(%{COLOR_UNINST_TAG})%{CHAR_UNINSTALLED}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations.");

AddOption(STRING, "TAG_STABILIZE",
	"{installedversionsshort}(%{DIFF_COLOR_INST_STABILIZE})%{DIFF_CHAR_INST_STABILIZE}{else}(%{DIFF_COLOR_UNINST_STABILIZE})%{DIFF_CHAR_UNINST_STABILIZE}{}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag for packages which have gained a stable version.");

AddOption(STRING, "TAG_NEW",
	"(%{DIFF_COLOR_NEW_TAG})%{DIFF_CHAR_NEW}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for new packages (diff-eix).");

AddOption(STRING, "TAG_BETTER",
	"(%{DIFF_COLOR_BETTER})%{DIFF_CHAR_BETTER}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"better version\" (diff-eix).");

AddOption(STRING, "TAG_WORSE",
	"(%{DIFF_COLOR_WORSE})%{DIFF_CHAR_WORSE}()",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"worse version\" (diff-eix).");

AddOption(STRING, "STRING_PLAIN_INSTALLED",
	"%{STRING_PLAIN_UNINSTALLED}",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"plain\" tagging installed packages.");

AddOption(STRING, "STRING_PLAIN_UNINSTALLED",
	"*",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"plain\" tagging unintalled packages.");

AddOption(STRING, "DIFF_STRING_NEW",
	">>",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"new package\" separators (diff-eix).");

AddOption(STRING, "DIFF_STRING_DELETE",
	"\\<\\<",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"deleted package\" separators (diff-eix).");

AddOption(STRING, "DIFF_STRING_CHANGED",
	"==",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"changed package\" separators (diff-eix).");

AddOption(STRING, "INSTALLEDVERSIONS",
	"<installedversions"
	":(%{COLOR_INST_VERSION}):():()"
	":(%{COLOR_FETCH})%{TAG_FETCH}"
	":(%{COLOR_MIRROR})%{TAG_MIRROR}"
	":::"
	":(%{COLOR_DATE})\\(:\\)()"
	":\\(:\\)"
	":(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()>",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions normally.");

AddOption(STRING, "INSTALLEDVERSIONS_COMPACT",
	"<installedversionsshortdate"
	":(%{COLOR_INST_VERSION}):():()"
	"::"
	":::"
	":@(%{COLOR_DATE}):()"
	":\\(:\\)"
	":(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()>",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions compactly.");

AddOption(STRING, "INSTALLEDVERSIONS_VERBOSE",
	"<installedversions"
	":(%{COLOR_INST_TITLE})Version\\\\:() (%{COLOR_INST_VERSION}):():()"
	":(%{COLOR_FETCH})%{TAG_FETCH}"
	":(%{COLOR_MIRROR})%{TAG_MIRROR}"
	":::"
	":\\n                          (%{COLOR_INST_TITLE})Date\\\\:()    "
	"(%{COLOR_DATE}):()"
	":\\n                          (%{COLOR_INST_TITLE})USE\\\\:()     "
	"::(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()"
	":\\n                          >",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions verbosely.");

AddOption(STRING, "FORMATLINE_INSTALLEDVERSIONS",
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
	"It defines the \"normal\" format for a line with installed versions.");

AddOption(STRING, "FORMATLINE_INSTALLEDVERSIONS_VERBOSE",
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
	"It defines the verbose format for a line with installed versions.");

AddOption(STRING, "DIFF_FORMATLINE_INSTALLEDVERSIONS",
	"{installedversionsshort}%{INSTALLEDVERSIONS_COMPACT}; {}"
	"",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for diff-eix for installed versions.");

AddOption(STRING, "FORMAT_NAME",
	"{system}"
		"(%{COLOR_CATEGORY_SYSTEM})"
	"{else}"
		"{world}"
			"(%{COLOR_CATEGORY_WORLD})"
		"{else}"
			"{world_sets}"
				"(%{COLOR_CATEGORY_WORLD_SETS})"
			"{else}"
				"(%{COLOR_CATEGORY})"
			"{}"
		"{}"
	"{}<category>()/"
	"{marked}"
		"(%{COLOR_MARKED_NAME})"
	"{else}"
		"{world}"
			"(%{COLOR_WORLD})"
		"{else}"
			"{world_sets}"
				"(%{COLOR_WORLD_SETS})"
			"{else}"
				"(%{COLOR_NAME})"
			"{}"
		"{}"
	"{}<name>()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the printing the package name.");

AddOption(STRING, "FORMAT_HEADER",
	"{installedversionsshort}["
	"{upgrade}%{TAG_UPGRADE}{}"
	"{downgrade}%{TAG_DOWNGRADE}{}"
	"{!recommend}%{TAG_INSTALLED}{}"
	"]{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the normal header symbols.");

AddOption(STRING, "FORMAT_HEADER_VERBOSE",
	"{installedversionsshort}(%{COLOR_INST_TAG})%{STRING_PLAIN_INSTALLED}"
	"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}"
	"()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the verbose header symbols.");

AddOption(STRING, "FORMAT_HEADER_COMPACT",
	"[{installedversionsshort}"
		"{upgrade}%{TAG_UPGRADE}{}"
		"{downgrade}%{TAG_DOWNGRADE}{}"
		"{!recommend}%{TAG_INSTALLED}{}"
	"{else}"
		"%{TAG_UNINSTALLED}"
	"{}]",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the compact header symbols.");

AddOption(STRING, "DIFF_FORMAT_HEADER_NEW",
	"["
	"{bestshort}%{TAG_STABILIZE}{}"
	"{upgrade}%{TAG_UPGRADE}{}"
	"{downgrade}%{TAG_DOWNGRADE}{}"
	"{!recommend}%{TAG_NEW}{}"
	"]"
	"{!bestshort} {}"
	"{upgrade}"
		"{!downgrade} {}"
	"{else}"
		" "
	"{}"
	" (%{DIFF_COLOR_NEW})%{DIFF_STRING_NEW}()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-new header symbols.");

AddOption(STRING, "DIFF_FORMAT_HEADER_DELETE",
	"{installedversionsshort}"
		"[(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()]"
	"{else}"
		"   "
	"{}"
	"   (%{DIFF_COLOR_DELETE})%{DIFF_STRING_DELETE}()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-delete header symbols.");

AddOption(STRING, "DIFF_FORMAT_HEADER_CHANGED",
	"["
	"{bestshort}{!oldbestshort}%{TAG_STABILIZE}{}{}"
	"{upgrade}%{TAG_UPGRADE}{}"
	"{downgrade}%{TAG_DOWNGRADE}{}"
	"{!upgrade}{better}%{TAG_BETTER}{}{}"
	"{!downgrade}{worse}%{TAG_WORSE}{}{}"
	"]"
	"{bestshort}{oldbestshort} {else}{}{else} {}"
	"{!upgrade}{!better} {}{}"
	"{!downgrade}{!worse} {}{}"
	" (%{DIFF_COLOR_CHANGED})%{DIFF_STRING_CHANGED}()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-changed header symbols.");
#endif

#if (DEFAULT_PART == 3)

AddOption(BOOLEAN, "NOBEST_COMPACT",
	"true",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, compact format prints no version if none is installable.");

AddOption(BOOLEAN, "NOBEST_CHANGE",
	"true",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, compact format prints no versions if all installable vanished.");

AddOption(BOOLEAN, "DIFF_NOBEST",
	"false",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, diff-eix prints no version if none is installable.");

AddOption(BOOLEAN, "DIFF_NOBEST_CHANGE",
	"false",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, diff-eix prints no versions if all installable vanished.");

AddOption(STRING, "FORMAT_NOBEST",
	"(%{COLOR_MASKED})"
	"--"
	"()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines what to print if no version number is printed.");

AddOption(STRING, "FORMAT_NOBEST_CHANGE",
	"(%{COLOR_MASKED})"
	"??"
	"()",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines what to print after \"->\" if there is no installable.");

AddOption(STRING, "FORMAT_BEST_COMPACT",
	"{bestshort}"
		"<best>"
	"{else}"
		"{bestshort*}"
			"<bestshort*>"
		"{else}"
			"%{?NOBEST_COMPACT}"
				"%{FORMAT_NOBEST}"
			"%{else}"
				"<availableversions>"
			"%{}"
		"{}"
	"{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the compact format for the best version(s).");

AddOption(STRING, "FORMAT_BEST_CHANGE",
	"{bestshort}"
		"<bestslots>"
	"{else}"
		"{bestshort*}"
			"<bestslots*>"
		"{else}"
			"%{?NOBEST_CHANGE}"
				"%{FORMAT_NOBEST_CHANGE}"
			"%{else}"
				"<availableversions>"
			"%{}"
		"{}"
	"{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the compact format for the best version(s) in case of changes.");

AddOption(STRING, "DIFF_FORMAT_BEST",
	"{bestshort}"
		"<best>"
	"{else}"
		"{bestshort*}"
			"<best*>"
		"{else}"
			"%{?DIFF_NOBEST}"
				"%{FORMAT_NOBEST}"
			"%{else}"
				"<availableversions>"
			"%{}"
		"{}"
	"{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the diff-eix format for the best version(s).");

AddOption(STRING, "DIFF_FORMAT_BEST_CHANGE",
	"{bestshort}"
		"<bestslots>"
	"{else}"
		"{bestshort*}"
			"<bestslots*>"
		"{else}"
			"%{?DIFF_NOBEST_CHANGE}"
				"%{FORMAT_NOBEST_CHANGE}"
			"%{else}"
				"<availableversions>"
			"%{}"
		"{}"
	"{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the diff-eix format for the best version(s) in case of changes.");

AddOption(STRING, "DIFF_FORMAT_CHANGED_VERSIONS",
	"%{?DIFF_PRINT_INSTALLED}"
		"%{DIFF_FORMATLINE_INSTALLEDVERSIONS}"
	"%{}"
	"{oldbestshort}"
		"<oldbestslots>"
	"{else}"
		"<oldavailableversions>"
	"{}"
	"() -> %{DIFF_FORMAT_BEST_CHANGE}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the diff-eix format for changed versions.");

AddOption(STRING, "FORMAT_OVERLAYKEY",
	"{overlaykey} <overlaykey>{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the printing the optional overlay key.");

AddOption(STRING, "FORMATLINE_NAME",
	"%{FORMAT_HEADER} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the normal header line.");

AddOption(STRING, "FORMATLINE_NAME_VERBOSE",
	"%{FORMAT_HEADER_VERBOSE} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the verbose header line.");

AddOption(STRING, "FORMATLINE_NAME_COMPACT",
	"%{FORMAT_HEADER_COMPACT} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the compact header line.");

AddOption(STRING, "DIFF_FORMATLINE_NAME_NEW",
	"%{DIFF_FORMAT_HEADER_NEW} %{FORMAT_NAME} ",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-new header symbols.");

AddOption(STRING, "DIFF_FORMATLINE_NAME_DELETE",
	"%{DIFF_FORMAT_HEADER_DELETE} %{FORMAT_NAME} ",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-delete header symbols.");

AddOption(STRING, "DIFF_FORMATLINE_NAME_CHANGED",
	"%{DIFF_FORMAT_HEADER_CHANGED} %{FORMAT_NAME} ",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-changed header symbols.");

AddOption(STRING, "FORMATLINE_AVAILABLEVERSIONS",
	"     (%{COLOR_TITLE})Available versions:()  <availableversions>\\n",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with installed versions.");

AddOption(STRING, "DIFF_FORMATLINE_BEST",
	"\\(%{DIFF_FORMAT_BEST}())",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the diff-eix line for the best versions/slots.");

AddOption(STRING, "DIFF_FORMATLINE_CHANGED_VERSIONS",
	"\\(%{DIFF_FORMAT_CHANGED_VERSIONS})",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the diff-eix line for changed versions.");

AddOption(STRING, "FORMATLINE_MARKEDVERSIONS",
	"%{!PRINT_ALWAYS}{marked}%{}"
	"     (%{COLOR_TITLE})Marked:()"
	"%{?PRINT_ALWAYS}{marked}%{}"
	"              "
	"(%{COLOR_MARKED_VERSION})<markedversions>()"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with marked versions.");

AddOption(BOOLEAN, "ALL_SETNAMES",
	"true",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines whether to include \"system\" in package sets output.");

AddOption(STRING, "PRINT_SETNAMES",
	"%{?ALL_SETNAMES}all%{}setnames",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It is the command used to print the package set names.");

AddOption(STRING, "FORMATLINE_PACKAGESETS",
	"%{!PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
	"     (%{COLOR_TITLE})Package sets:()"
	"%{?PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
	"        "
	"(%{COLOR_PACKAGESETS})<%{PRINT_SETNAMES}>()"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with package sets.");

AddOption(STRING, "FORMATLINE_HOMEPAGE",
	"%{!PRINT_ALWAYS}{homepage}%{}"
	"     (%{COLOR_TITLE})Homepage:()"
	"%{?PRINT_ALWAYS}{homepage}%{}"
	"            "
	"<homepage>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package homepage.");

AddOption(STRING, "FORMATLINE_BUGS",
	"     (%{COLOR_TITLE})Find open bugs:()"
	"      "
	"http://bugs.gentoo.org/buglist.cgi?quicksearch="
	"<category>%2F<name>"
	"\\n",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package bug-reference.");

AddOption(STRING, "FORMATLINE_DESCRIPTION",
	"%{!PRINT_ALWAYS}{description}%{}"
	"     (%{COLOR_TITLE})Description:()"
	"%{?PRINT_ALWAYS}{description}%{}"
	"         "
	"<description>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package description.");

AddOption(STRING, "FORMATLINE_BEST",
	"%{!PRINT_ALWAYS}{bestshort}%{}"
	"     (%{COLOR_TITLE})Best versions/slot:()"
	"%{?PRINT_ALWAYS}{bestshort}%{}"
	"  "
	"<bestslots>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the best versions/slots.");

AddOption(STRING, "FORMATLINE_RECOMMEND",
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
	"It defines the format for a line with the up-/downgrade recommendations.");

AddOption(STRING, "FORMATLINE_PROVIDE",
	"%{!PRINT_ALWAYS}{provide}%{}"
	"     (%{COLOR_TITLE})Provides:()"
	"%{?PRINT_ALWAYS}{provide}%{}"
	"            "
	"<provide>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package provides.");

AddOption(STRING, "FORMATLINE_LICENSES",
	"%{!PRINT_ALWAYS}{licenses}%{}"
	"     (%{COLOR_TITLE})License:()"
	"%{?PRINT_ALWAYS}{licenses}%{}"
	"             "
	"<licenses>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package licenses.");

AddOption(STRING, "DIFF_FORMATLINE",
	"%{FORMAT_OVERLAYKEY}: <description>",
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for diff-eix after the versions.");

AddOption(STRING, "FORMAT_ALL",
	"%{FORMATLINE_NAME}"
	"%{FORMATLINE_AVAILABLEVERSIONS}"
	"%{FORMATLINE_INSTALLEDVERSIONS}"
	"%{FORMATLINE_MARKEDVERSIONS}"
	"%{FORMATLINE_HOMEPAGE}"
	"%{FORMATLINE_DESCRIPTION}",
	"This format is only used for delayed substitution in FORMAT.\n"
	"It defines the format of the normal output of eix.");

AddOption(STRING, "FORMAT_ALL_COMPACT",
	"%{FORMATLINE_NAME_COMPACT}"
	" \\({marked}(%{COLOR_MARKED_VERSION})<markedversions>(); {}"
	"{installedversionsshort}"
		"%{INSTALLEDVERSIONS_COMPACT}"
		"{recommend} -> %{FORMAT_BEST_CHANGE}{}"
	"{else}"
		"%{FORMAT_BEST_COMPACT}"
	"{}"
	"()\\): <description>",
	"This format is only used for delayed substitution in FORMAT_COMPACT.\n"
	"It defines the format of the compact output of eix (option -c).");

AddOption(STRING, "FORMAT_ALL_VERBOSE",
	"%{FORMATLINE_NAME_VERBOSE}"
	"%{FORMATLINE_AVAILABLEVERSIONS}"
	"%{FORMATLINE_INSTALLEDVERSIONS_VERBOSE}"
	"%{FORMATLINE_BEST}"
	"%{FORMATLINE_RECOMMEND}"
	"%{FORMATLINE_MARKEDVERSIONS}"
	"%{FORMATLINE_PACKAGESETS}"
	"%{FORMATLINE_HOMEPAGE}"
	"%{?PRINT_BUGS}%{FORMATLINE_BUGS}%{}"
	"%{FORMATLINE_DESCRIPTION}"
	"%{FORMATLINE_PROVIDE}"
	"%{FORMATLINE_LICENSES}",
	"This format is only used for delayed substitution in FORMAT_VERBOSE.\n"
	"It defines the format of the verbose output of eix (option -v).");

AddOption(STRING, "FORMAT",
	"%{FORMAT_ALL}",
	"The format of the normal output of eix.\n"
	"Do not modify it in a config file; modify FORMAT_ALL instead.");

AddOption(STRING, "FORMAT_COMPACT",
	"%{FORMAT_ALL_COMPACT}",
	"The format of the compact output of eix (option -c).\n"
	"Do not modify it in a config file; modify FORMAT_ALL_COMPACT instead.");

AddOption(STRING, "FORMAT_VERBOSE",
	"%{FORMAT_ALL_VERBOSE}",
	"The format of the verbose output of eix (option -v).\n"
	"Do not modify it in a config file; modify FORMAT_ALL_VERBOSE instead.");

AddOption(STRING, "DIFF_FORMAT_ALL_NEW",
	"%{DIFF_FORMATLINE_NAME_NEW}"
	"%{DIFF_FORMATLINE_BEST}"
	"%{DIFF_FORMATLINE}",
	"This format is only used for delayed substitution in DIFF_FORMAT_NEW.\n"
	"It defines the format used for new packages (diff-eix).");

AddOption(STRING, "DIFF_FORMAT_ALL_DELETE",
	"%{DIFF_FORMATLINE_NAME_DELETE}"
	"%{DIFF_FORMATLINE_BEST}"
	"%{DIFF_FORMATLINE}",
	"This format is only used for delayed substitution in DIFF_FORMAT_DELETE.\n"
	"It defines the format used for packages that were deleted (diff-eix).");

AddOption(STRING, "DIFF_FORMAT_ALL_CHANGED",
	"%{DIFF_FORMATLINE_NAME_CHANGED}"
	"%{DIFF_FORMATLINE_CHANGED_VERSIONS}"
	"%{DIFF_FORMATLINE}",
	"This format is only used for delayed substitution in DIFF_FORMAT_CHANGED.\n"
	"It defines the format used for packages that were changed (diff-eix).");

AddOption(STRING, "DIFF_FORMAT_NEW",
	"%{DIFF_FORMAT_ALL_NEW}",
	"The format used for new packages (diff-eix).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_NEW instead.");

AddOption(STRING, "DIFF_FORMAT_DELETE",
	"%{DIFF_FORMAT_ALL_DELETE}",
	"The format used for packages that were deleted (diff-eix).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_DELETE instead.");

AddOption(STRING, "DIFF_FORMAT_CHANGED",
	"%{DIFF_FORMAT_ALL_CHANGED}",
	"The format used for packages that were changed (diff-eix).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_CHANGED instead.");

AddOption(STRING, "FORMAT_INSTALLATION_DATE",
	"%X %x",
	"strftime() format for printing the installation date in long form");

AddOption(STRING, "FORMAT_SHORT_INSTALLATION_DATE",
	"%x",
	"strftime() format for printing the installation date in short form");

AddOption(STRING, "FORMAT_BEFORE_KEYWORDS",
	" \"(cyan)", "This string is printed before KEYWORDS string for a version is output.\n"
	"(this is only used when --versionlines and PRINT_KEYWORDS is active)");

AddOption(STRING, "FORMAT_AFTER_KEYWORDS",
	"()\"", "This string is printed after KEYWORDS string for a version is output.\n"
	"(this is only used when --versionlines and PRINT_KEYWORDS is active)");

AddOption(STRING, "FORMAT_BEFORE_IUSE",
	" [(blue)", "This string is printed before IUSE data for a version is output.\n"
	"(this is only used when --versionlines is active)");

AddOption(STRING, "FORMAT_AFTER_IUSE",
	"()]", "This string is printed after IUSE data for a version is output.\n"
	"(this is only used when --versionlines is active)");

AddOption(STRING, "FORMAT_BEFORE_COLL_IUSE",
	" \\{(blue)", "This string is printed before IUSE data for all versions is output.\n"
	"(this is only used when --versionlines is inactive and there are no slots).");

AddOption(STRING, "FORMAT_AFTER_COLL_IUSE",
	"()\\}", "This string is printed before IUSE data for all versions is output.\n"
	"(this is only used when --versionlines is inactive and there are no slots).");

AddOption(STRING, "FORMAT_BEFORE_SLOT_IUSE",
	"\\n\\t\\{(blue)", "This string is printed before IUSE data for all versions is output.\n"
	"(this is only used when --versionlines is inactive and sorting is by slots).");

AddOption(STRING, "FORMAT_AFTER_SLOT_IUSE",
	"()\\}", "This string is printed before IUSE data for all versions is output.\n"
	"(this is only used when --versionlines is inactive and sorting is by slots).");

AddOption(STRING, "COLOR_MASKED",
	"red", "Define color for masked versions.");

AddOption(STRING, "COLOR_UNSTABLE",
	"yellow", "Define color for unstable versions");

AddOption(STRING, "COLOR_STABLE",
	"green", "Define color for stable versions");

AddOption(STRING, "COLOR_OVERLAYKEY",
	"cyan,1", "Color for the overlaykey in version listings.");

AddOption(STRING, "COLOR_VIRTUALKEY",
	"purple,1", "Color for the overlaykey for virtual overlays in version listings.");

AddOption(STRING, "COLOR_SLOTS",
	"red,1", "Color for slots.");

AddOption(STRING, "COLOR_FETCH",
	"red", "Color for the fetch restriction tag.");

AddOption(STRING, "COLOR_MIRROR",
	"cyan", "Color for the mirror restriction tag.");

AddOption(STRING, "MARK_INSTALLED",
	"inverse", "How installed packages are marked in version listings.");

AddOption(STRING, "MARK_VERSIONS",
	"underline", "How the package versions passed with --pipe are marked in version listings.");

AddOption(BOOLEAN, "FORCE_USECOLORS",
	"false", "This turns --force-color on for every query.");

AddOption(BOOLEAN, "COLOR_ORIGINAL",
	"true", "If false, versions are only colored according to the local setting.");

AddOption(BOOLEAN, "COLOR_LOCAL_MASK",
	"false", "If false, COLOR_ORIGINAL=false has no effect on versions which are\n"
	"only locally masked (i.e. [m]).");

AddOption(BOOLEAN, "STYLE_VERSION_SORTED",
	"false", "Defines whether --versionsorted is on by default.");

AddOption(BOOLEAN, "PRINT_KEYWORDS",
	"first", "Defines whether KEYWORDS string is printed in --versionlines mode.\n"
	"\"before\", \"first\", or \"true\" will print KEYWORDS string before IUSE,\n"
	"\"after\" or \"last\" will print it afterwards. \"false\" will not print it.");

AddOption(BOOLEAN, "STYLE_VERSION_LINES",
	"false", "Defines whether --versionlines is on by default.");

AddOption(BOOLEAN, "COLORED_SLOTS",
	"true", "If false, the slotnames appended to versions are not colored.");

AddOption(BOOLEAN, "COLON_SLOTS",
	"false", "If true, separated slots from versions with a colon instead of braces.");

AddOption(BOOLEAN, "DEFAULT_IS_OR",
	"false", "Whether default concatenation of queries is -o (or) or -a (and)");

AddOption(BOOLEAN, "DUP_PACKAGES_ONLY_OVERLAYS",
	"false", "Whether checks for duplicate packages occur only among overlays");

AddOption(BOOLEAN, "DUP_VERSIONS_ONLY_OVERLAYS",
	"false", "Whether checks for duplicate versions occur only among overlays");

AddOption(STRING, "OVERLAYS_LIST",
	"all-used-renumbered", "Which overlays to list (all/all-if-used/all-used/all-used-renumbered/no)");

AddOption(INTEGER, "LEVENSHTEIN_DISTANCE",
	LEVENSHTEIN_DISTANCE_STR,
	"The default maximal levensthein for which a string is considered a match.");
#endif

#if (DEFAULT_PART == 4)

AddOption(STRING, "PORTDIR_CACHE_METHOD",
	PORTDIR_CACHE_METHOD , "Portage cache-backend that should be used for PORTDIR\n"
	"(metadata[:*]/sqlite/cdb/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])");

AddOption(STRING, "OVERLAY_CACHE_METHOD",
	"parse|ebuild*", "Portage cache-backend that should be used for the overlays.\n"
	"(metadata[:*]/sqlite/cdb/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])");

AddOption(STRING, "CACHE_METHOD",
	"", "Overrides OVERLAY_CACHE_METHOD or PORTDIR_CACHE_METHOD for certain paths.\n"
	"This is a list of pairs DIR-PATTERN METHOD. Later entries take precedence.");

AddOption(STRING, "ADD_CACHE_METHOD",
	"", "This variable is added to CACHE_METHOD.\n"
	"This variable is meant to be set only locally.");

AddOption(STRING, "OVERRIDE_CACHE_METHOD",
	"", "This variable can override the choices of CACHE_METHOD/ADD_CACHE_METHOD\n"
	"and in addition it can override the choices made by KEEP_VIRTUALS.");

AddOption(STRING, "ADD_OVERRIDE_CACHE_METHOD",
	"", "This variable is added to OVERRIDE_CACHE_METHOD.\n"
	"This variable is meant to be set only locally.");

AddOption(STRING, "EXCLUDE_OVERLAY",
	"", "List of overlays that should be excluded from the index.");

AddOption(STRING, "ADD_OVERLAY",
	"", "List of overlays that should be added to the index.");

AddOption(BOOLEAN, "KEEP_VIRTUALS",
	"false", "Keep virtuals of the old cache file by adding corresponding entries\n"
	"implicitly to the values of ADD_OVERLAY and ADD_CACHE_METHOD");

AddOption(BOOLEAN, "LOCAL_PORTAGE_CONFIG",
	"true", "If false, /etc/portage and ACCEPT_KEYWORDS are ignored.");

AddOption(BOOLEAN, "ALWAYS_ACCEPT_KEYWORDS",
	"false", "If true, ACCEPT_KEYWORDS is used even without LOCAL_PORTAGE_CONFIG,\n"
	"e.g. to determine the \"default\" stability.");

AddOption(BOOLEAN, "UPGRADE_LOCAL_MODE",
	"", "if +/-, eix -u will match as if LOCAL_PORTAGE_CONFIG=true/false.");

AddOption(BOOLEAN, "RECOMMEND_LOCAL_MODE",
	"", "if +/-, recommendations for up- or downgrade will act as if\n"
	"LOCAL_PORTAGE_CONFIG=true/false");

AddOption(BOOLEAN, "UPGRADE_TO_HIGHEST_SLOT",
	"true", "If true, upgrade tests succeed for installed packages with new higher slots");

AddOption(BOOLEAN, "PRINT_SLOTS",
	"true", "If false, no slot information is printed.");

AddOption(BOOLEAN, "EIX_PRINT_IUSE",
	"true", "This variable is only used for delayed substitution.\n"
	"If false, no IUSE data is printed for eix");

AddOption(BOOLEAN, "DIFF_PRINT_IUSE",
	"false", "This variable is only used for delayed substitution.\n"
	"If false, no IUSE data is printed for diff-eix");

AddOption(BOOLEAN, "PRINT_IUSE",
	"%{*PRINT_IUSE}", "If false, no IUSE data is printed.");

AddOption(BOOLEAN, "SORT_INST_USE_ALPHA",
	"false", "If false, sort installed useflags by whether they are set.");

AddOption(STRING, "CHECK_INSTALLED_OVERLAYS",
	"repository", "Allowed values are true/false/repository.\n"
	"If true, always check from which overlay a package version was installed.\n"
	"If false, only packages with versions in at least two trees are checked.\n"
	"The compromise - repository - checks at least always the repository files.\n"
	"Without a check, the assumed overlay may be wrong if the version was\n"
	"actually installed from a place not in the database anymore.");

AddOption(BOOLEAN, "OBSOLETE_MINUSASTERISK",
	"false", "If true, treat -* in /etc/portage/package.keywords as <=portage-2.1.2\n"
	"Since >=portage-2.1.2-r4, -* is practically obsolete and replaced\n"
	"by ** which accepts anything (note that there are also * and ~*).");

AddOption(STRING, "PRINT_COUNT_ALWAYS",
	"false", "Allowed values are true/false/never.\n"
	"If true, always print the number of matches (even 0 or 1) in the last line.");

AddOption(BOOLEAN, "COUNT_ONLY_PRINTED",
	"true", "If false, count independently of whether the matches are printed.");

AddOption(STRING, "TAG_FETCH",
	"!f", "Tag for fetch-restricted versions.");

AddOption(STRING, "TAG_MIRROR",
	"!m", "Tag for mirror-restricted versions.");

#define TAG_FOR(opt_type, opt_default, opt_comment) \
	AddOption(STRING, "TAG_FOR_" #opt_type, \
		opt_default, "Tag for " #opt_comment " versions.")

TAG_FOR(PROFILE, "[P]", "profile masked");
TAG_FOR(MASKED, "[M]", "package.masked");
TAG_FOR(EX_PROFILE, "{P}", "originally profile masked but unmasked");
TAG_FOR(EX_MASKED, "{M}", "originally package.masked but unmasked");
TAG_FOR(LOCALLY_MASKED, "[m]", "only locally masked");
TAG_FOR(STABLE, "", "stable");
TAG_FOR(UNSTABLE, "~", "unstable");
TAG_FOR(MINUS_ASTERISK, "-*", "-*");
TAG_FOR(MINUS_KEYWORD, "-", "-ARCH");
TAG_FOR(ALIEN_STABLE, "*", "ALIENARCH");
TAG_FOR(ALIEN_UNSTABLE, "~*", "~ALIENARCH");
TAG_FOR(MISSING_KEYWORD, "**", "no keyword");
TAG_FOR(EX_UNSTABLE, "(~)", "originally unstable but now stable");
TAG_FOR(EX_MINUS_ASTERISK, "(-*)", "originally -* but now stable");
TAG_FOR(EX_MINUS_KEYWORD, "(-)", "originally -ARCH but now stable");
TAG_FOR(EX_ALIEN_STABLE, "(*)", "originally ALIENARCH but now stable");
TAG_FOR(EX_ALIEN_UNSTABLE, "(~*)", "originally ~ALIENARCH but now stable");
TAG_FOR(EX_MISSING_KEYWORD, "(**)", "originally no keyword but now stable");

/* fancy new feature: change default matchfield depending on the searchstring. */
#define MATCH_IF(field, value) \
	AddOption(STRING, "MATCH_" #field "_IF", \
		value, "Use " #field " as default matchfield if the search string matches\n" \
		"the given extended regular expression.")

MATCH_IF(NAME,          ".*");
MATCH_IF(DESCRIPTION,   ".*");
MATCH_IF(LICENSE,       ".*");
MATCH_IF(CATEGORY,      ".*");
MATCH_IF(CATEGORY_NAME, "/");
MATCH_IF(HOMEPAGE,      ".*");
MATCH_IF(PROVIDE,       "^virtual/");
MATCH_IF(IUSE,          ".*");
MATCH_IF(SET,           ".*");
MATCH_IF(SLOT,          ".*");
MATCH_IF(INSTALLED_SLOT,".*");

AddOption(STRING, "MATCH_ORDER",
	"PROVIDE CATEGORY_NAME NAME", "Try the regex from MATCH_(.*)_IF in this order. Use whitespaces as delimiter.");

AddOption(BOOLEAN, "TEST_FOR_EMPTY",
	"true", "Defines whether empty entries in /etc/portage/package.* are shown with -t.");

AddOption(BOOLEAN, "TEST_KEYWORDS",
	"true", "Defines whether /etc/portage/package.keywords is tested with -t.");

AddOption(BOOLEAN, "TEST_MASK",
	"true", "Defines whether /etc/portage/package.mask is tested with -t.");

AddOption(BOOLEAN, "TEST_UNMASK",
	"true", "Defines whether /etc/portage/package.unmask is tested with -t.");

AddOption(BOOLEAN, "TEST_USE",
	"true", "Defines whether /etc/portage/package.use is tested with -t.");

AddOption(BOOLEAN, "TEST_CFLAGS",
	"true", "Defines whether /etc/portage/package.cflags is tested with -t.");

AddOption(BOOLEAN, "TEST_REMOVED",
	"true", "Defines whether removed packages are tested with -t.");

AddOption(BOOLEAN, "TEST_FOR_NONEXISTENT",
	"true", "Defines whether non-existing installed versions are positive for -T.");

AddOption(BOOLEAN, "NONEXISTENT_IF_OTHER_OVERLAY",
	"true",
	"Defines whether versions are non-existent for TEST_FOR_NONEXISTENT\n"
	"if they come from a different overlay than the installed version.");

AddOption(BOOLEAN, "NONEXISTENT_IF_MASKED",
	"true", "Defines whether masked versions are non-existent for TEST_FOR_NONEXISTENT.");

AddOption(BOOLEAN, "TEST_FOR_REDUNDANCY",
	"true", "Defines whether redundant entries are positive for -T.");

AddOption(BOOLEAN, "ACCEPT_KEYWORDS_AS_ARCH",
	"true", "If true modify ARCH by ACCEPT_KEYWORDS.\n"
	"This determines which keywords are considered as ARCH or OTHERARCH.");

AddOption(STRING, "REDUNDANT_IF_DOUBLE",
	"some",
	"Applies if /etc/portage/package.keywords lists the same keyword twice\n"
	"for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_DOUBLE_LINE",
	"some",
	"Applies if /etc/portage/package.keywords has two lines for identical target.");

AddOption(STRING, "REDUNDANT_IF_MIXED",
	"false",
	"Applies if /etc/portage/package.keywords lists two different keywords,\n"
	"e.g. ~ARCH and -*, for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_WEAKER",
	"all-installed",
	"Applies if /etc/portage/package.keywords lists a keywords which can\n"
	"be replaced by a weaker keyword, e.g. -* or ~OTHERARCH or OTHERARCH\n"
	"in place of ~ARCH, or ~OTHERARCH in place of OTHERARCH,\n"
	"for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_STRANGE",
	"some",
	"Applies if /etc/portage/package.keywords lists a strange keyword\n"
	"e.g. UNKNOWNARCH (unknown to the .ebuild and ARCH) or -OTHERARCH,\n"
	"for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_MINUSASTERISK",
	"some",
	"Applies if /etc/portage/package.keywords contains some -* entry.\n"
	"This test only applies if OBSOLETE_MINUSASTERISK is false.");

AddOption(STRING, "REDUNDANT_IF_NO_CHANGE",
	"all-installed",
	"Applies if /etc/portage/package.keywords provides keywords which do not\n"
	"change the availability keywords status for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_MASK_NO_CHANGE",
	"all-uninstalled",
	"Applies if /etc/portage/package.mask contains entries\n"
	"which do not change the mask status for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_UNMASK_NO_CHANGE",
	"all-installed",
	"Applies if /etc/portage/package.unmask contains entries\n"
	"which do not change the mask status for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_DOUBLE_MASKED",
	"some",
	"Applies if /etc/portage/package.mask matches twice\n"
	"for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_DOUBLE_UNMASKED",
	"some",
	"Applies if /etc/portage/package.unmask matches twice\n"
	"for the versions in question.");

AddOption(STRING, "REDUNDANT_IF_DOUBLE_USE",
	"some",
	"Applies if /etc/portage/package.use matches twice");

AddOption(STRING, "REDUNDANT_IF_DOUBLE_CFLAGS",
	"some",
	"Applies if /etc/portage/package.cflags matches twice");

AddOption(STRING, "REDUNDANT_IF_IN_KEYWORDS",
	"-some",
	"Applies if /etc/portage/package.keywords contains a matching entry");

AddOption(STRING, "REDUNDANT_IF_IN_MASK",
	"-some",
	"Applies if /etc/portage/package.mask matches");

AddOption(STRING, "REDUNDANT_IF_IN_UNMASK",
	"-some",
	"Applies if /etc/portage/package.mask matches");

AddOption(STRING, "REDUNDANT_IF_IN_USE",
	"-some",
	"Applies if /etc/portage/package.use matches");

AddOption(STRING, "REDUNDANT_IF_IN_CFLAGS",
	"-some",
	"Applies if /etc/portage/package.cflags matches");

AddOption(STRING, "EIXCFGDIR",
	"%{PORTAGE_CONFIGROOT}/etc/portage",
	"This variable is only used for delayed substitution.\n"
	"It is the directory where eix searches for its package.*.* config files");

AddOption(STRING, "KEYWORDS_NONEXISTENT",
	"%{EIXCFGDIR}/package.keywords.nonexistent",
	"Entries listed in this file/dir are excluded for -t TEST_KEYWORDS");

AddOption(STRING, "MASK_NONEXISTENT",
	"%{EIXCFGDIR}/package.mask.nonexistent",
	"Entries listed in this file/dir are excluded for -t TEST_MASK");

AddOption(STRING, "UNMASK_NONEXISTENT",
	"%{EIXCFGDIR}/package.unmask.nonexistent",
	"Entries listed in this file/dir are excluded for -t TEST_UNMASK");

AddOption(STRING, "USE_NONEXISTENT",
	"%{EIXCFGDIR}/package.use.nonexistent",
	"Entries listed in this file/dir are excluded for -t TEST_USE");

AddOption(STRING, "CFLAGS_NONEXISTENT",
	"%{EIXCFGDIR}/package.cflags.nonexistent",
	"Entries listed in this file/dir are excluded for -t TEST_CFLAGS");

AddOption(STRING, "INSTALLED_NONEXISTENT",
	"%{EIXCFGDIR}/package.installed.nonexistent",
	"Packages listed in this file/dir are excluded for -t TEST_REMOVED");

AddOption(STRING, "KEYWORDS_NOWARN",
	"%{EIXCFGDIR}/package.keywords.nowarn",
	"Exceptional packages for -T tests of /etc/portage/package.keywords");

AddOption(STRING, "MASK_NOWARN",
	"%{EIXCFGDIR}/package.mask.nowarn",
	"Exceptional packages for -T tests of /etc/portage/package.mask");

AddOption(STRING, "UNMASK_NOWARN",
	"%{EIXCFGDIR}/package.unmask.nowarn",
	"Exceptional packages for -T tests of /etc/portage/package.unmask");

AddOption(STRING, "USE_NOWARN",
	"%{EIXCFGDIR}/package.use.nowarn",
	"Exceptional packages for -T tests of /etc/portage/package.use");

AddOption(STRING, "CFLAGS_NOWARN",
	"%{EIXCFGDIR}/package.cflags.nowarn",
	"Exceptional packages for -T tests of /etc/portage/package.cflags");

AddOption(STRING, "INSTALLED_NOWARN",
	"%{EIXCFGDIR}/package.installed.nowarn",
	"Exceptional packages for -T tests of installed packages");

AddOption(STRING, "DUMMY",
	"",
	"This variable is ignored. You can use it to collect delayed references to\n"
	"locally added (unused) variables so that they are printed with --dump.");

#endif
