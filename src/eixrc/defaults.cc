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
	"", _(
	"The file which is used instead of /etc/eixrc and ~/.eixrc.\n"
	"This variable can of course only be set in the environment."));

AddOption(PREFIXSTRING, "EIXRC_SOURCE",
	"", _(
	"This path is prepended to source commands in /etc/eixrc.\n"
	"If set in /etc/eixrc it temporarily overrides the environment.\n"
	"You must not use delayed substitution in this variable."));

AddOption(STRING, "EIX_SYNC_OPTS",
	"", _(
	"This variable is used for delayed substitution in EIX_SYNC_CONF.\n"
	"It contains code which is evaluated by eix-sync, so be aware of security!"));

AddOption(STRING, "EIX_SYNC_CONF",
	"%{EIX_SYNC_OPTS}", _(
	"The content of this variable is appended to /etc/eix-sync.conf\n"
	"In particular, it can be used to override options set in that file.\n"
	"Parts of this variable are evaluated in eix-sync: Be aware of security!"));

AddOption(STRING, "EIX_REMOTE_OPTS",
	"", _(
	"This variable contains default options for the eix-remote script.\n"
	"Note that its content is evaluated, so quote correctly. Typical example:\n"
	"EIX_REMOTE_OPTS='-f \"`portageq portdir`/local/layman/eix-caches.tar.bz2\"'"));

AddOption(STRING, "EIX_LAYMAN_OPTS",
	"", _(
	"This variable contains default options for the eix-layman script.\n"
	"Note that its content is evaluated, so quote correctly."));

AddOption(STRING, "EIX_TEST_OBSOLETE_OPTS",
	"", _(
	"This variable contains default options for the eix-test-obsolete script."));

AddOption(PREFIXSTRING, "EIX_PREFIX",
	EIX_PREFIX_DEFAULT, _(
	"If this variable is set in the environment, then it is prefixed\n"
	"to the path where /etc/eixrc is searched. If it is not set in the\n"
	"environment, then PORTAGE_CONFIGROOT is used instead.\n"
	"If both are unset, the EIX_PREFIX default value is used instead.\n"
	"Moreover, EIX_PREFIX is used for delayed substitution for EPREFIX\n"
	"and EPREFIX_SOURCE."));

AddOption(PREFIXSTRING, "ROOT",
	ROOT_DEFAULT, _(
	"This variable is only used for delayed substitution.\n"
	"It influences most paths except for $HOME/.eixrc, the cache file\n"
	"passed in the command line, PORTAGE_PROFILE, PORTDIR,\n"
	"and overlays. In contrast to EPREFIX, further exceptions are:\n"
	"PORTAGE_CONFIGROOT, portage/scripts-internal stuff and the eix cachefile."));

AddOption(PREFIXSTRING, "EPREFIX",
	"%{EIX_PREFIX}" EPREFIX_DEFAULT, _(
	"This variable is used for delayed substitution for path prefixes.\n"
	"It influences most paths except for $HOME/.eixrc, the cache file\n"
	"passed in the command line, PORTAGE_PROFILE, PORTDIR, and overlays."));

AddOption(PREFIXSTRING, "EPREFIX_TREE",
	"", _(
	"This variable is only used for delayed substitution.\n"
	"It is the path prepended to PORTAGE_PROFILE, PORTDIR, and overlays."));

AddOption(PREFIXSTRING, "EPREFIX_ROOT",
	"%{??EPREFIX}%{EPREFIX}%{else}%{ROOT}%{}", _(
	"It applies for those paths for which EPREFIX and ROOT should both apply.\n"
	"So you can decide here what to do if both are nonempty. For instance,\n"
	"the choice %{EPREFIX}%{ROOT} will apply both; the default applies EPREFIX\n"
	"but not ROOT for these paths in such a case (i.e. if both are nonempty)."));

AddOption(PREFIXSTRING, "PORTAGE_CONFIGROOT",
	"%{EPREFIX}", _(
	"This path is prepended to the /etc paths."));

AddOption(PREFIXSTRING, "MAKE_GLOBALS",
	"%{EPREFIX}/usr/share/portage/config/make.globals", _(
	"This file is used instead of %{PORTAGE_CONFIGROOT}/etc/make.globals\n"
	"if it exists. This is reasonable for >=portage-2.2*"));

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_EXEC",
	"%{EPREFIX}", _(
	"This variable is only used for delayed substitution.\n"
	"It is used as prefix in the EXEC_EBUILD* and EBUILD_DEPEND_TEMP defaults."));

AddOption(PREFIXSTRING, "EPREFIX_SOURCE",
	"%{EIX_PREFIX}", _(
	"This path is prepended to source commands in /etc/make.{conf,globals}."));

AddOption(PREFIXSTRING, "EPREFIX_INSTALLED",
	"%{EPREFIX_ROOT}", _(
	"Prefix to the path where eix expects information about installed packages."));

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_CACHE",
	"%{EPREFIX}", _(
	"This prefix is prepended to the portage cache."));

AddOption(PREFIXSTRING, "EPREFIX_ACCESS_OVERLAYS",
	"", _(
	"This prefix is prepended to overlays when their files are accessed."));

AddOption(PREFIXSTRING, "EPREFIX_PORTDIR",
	"%{EPREFIX_TREE}", _(
	"This path is prepended to PORTDIR."));

AddOption(PREFIXSTRING, "EPREFIX_OVERLAYS",
	"%{EPREFIX_TREE}", _(
	"This path is prepended to PORTIDIR_OVERLAY values."));

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_PROFILE",
	"%{EPREFIX_TREE}", _(
	"This path is prepended to PORTAGE_PROFILE."));

AddOption(PREFIXSTRING, "EPREFIX_VIRTUAL",
	"%{EPREFIX_TREE}", _(
	"This is prepended to overlays in eix database to test whether they exist."));

AddOption(STRING, "EIX_CACHEFILE",
	"%{EPREFIX}" EIX_CACHEFILE, _(
	"This file is the default eix cache."));

AddOption(STRING, "EXEC_EBUILD",
	"%{EPREFIX_PORTAGE_EXEC}/usr/bin/ebuild", _(
	"The path to the ebuild executable."));

AddOption(STRING, "EXEC_EBUILD_SH",
	"%{EPREFIX_PORTAGE_EXEC}" EBUILD_SH_DEFAULT, _(
	"The path to the ebuild.sh executable."));

AddOption(STRING, "EBUILD_DEPEND_TEMP",
	"%{EPREFIX_PORTAGE_EXEC}/var/cache/edb/dep/aux_db_key_temp", _(
	"The path to the tempfile generated by \"ebuild depend\"."));

AddOption(STRING, "EIX_WORLD",
	"%{EPREFIX_ROOT}/var/lib/portage/world", _(
	"This file is considered as the world file."));

AddOption(STRING, "EIX_WORLD_SETS",
	"%{EIX_WORLD}_sets", _(
	"This file is considered as the world_sets file."));

AddOption(BOOLEAN, "SAVE_WORLD",
	"true", _(
	"Store the information of the world file in the cache file.\n"
	"Set this to false if you do not want that everybody can get this information."));

AddOption(BOOLEAN, "CURRENT_WORLD",
	"true", _(
	"Prefer the current world file (if readable) over the data in the cachefile."));

AddOption(BOOLEAN, "SKIP_PERMISSION_TESTS",
	"false", _(
	"Whether to test for group and permissions.  You must set this to true\n"
	"if you use more sophisticated permission setups (e.g. NSS/LDAP)."));

AddOption(STRING, "EBUILD_USER",
	"portage", _(
	"The user which is used for running bash on ebuilds when\n"
	"cache method ebuild or ebuild* is used. See EBUILD_UID."));

AddOption(STRING, "EBUILD_GROUP",
	"%{EBUILD_USER}", _(
	"The group which is used for running bash on ebuilds when\n"
	"cache method ebuild or ebuild* is used. See EBUILD_GID."));

AddOption(STRING, "EBUILD_UID",
	"250", _(
	"If EBUILD_USER is empty or nonexistent, use this user id.\n"
	"In this case and if ${EBUILD_UID} <= 0, the user id is not changed."));

AddOption(STRING, "EBUILD_GID",
	"%{EBUILD_UID}", _(
	"If EBUILD_GROUP is empty or nonexistent, use this group id.\n"
	"In this case and if ${EBUILD_GID} <= 0, the group id is not changed."));

AddOption(STRING, "PORTAGE_ROOTPATH",
	PORTAGE_ROOTPATH_DEFAULT, _(
	"This variable is passed unchanged to ebuild.sh\n"
	"Usually ebuild.sh uses it to calculate the PATH."));

AddOption(STRING, "PORTAGE_BIN_PATH",
	PORTAGE_BIN_PATH_DEFAULT, _(
	"This variable is passed unchanged to ebuild.sh\n"
	"Usually ebuild.sh uses it to calculate the PATH."));

AddOption(BOOLEAN, "QUICKMODE",
	"false", _(
	"Whether --quick is on by default."));

AddOption(BOOLEAN, "CAREMODE",
	"false", _(
	"Whether --care is on."));

AddOption(BOOLEAN, "QUIETMODE",
	"false", _(
	"Whether --quiet is on by default."));

AddOption(BOOLEAN, "DIFF_ONLY_INSTALLED",
	"false", _(
	"If true, eix-diff will only consider version changes for installed packages."));

AddOption(BOOLEAN, "DIFF_NO_SLOTS",
	"false", _(
	"If true, eix-diff will not consider slots for version changes."));

AddOption(BOOLEAN, "DIFF_SEPARATE_DELETED",
	"true", _(
	"If false, eix-diff will mix deleted and changed packages"));

AddOption(BOOLEAN, "DIFF_PRINT_HEADER",
	"true", _(
	"Should eix-diff print a header info line?"));

AddOption(BOOLEAN, "NO_RESTRICTIONS",
	"false", _(
	"If false, RESTRICT and PROPERTIES values are output."));

AddOption(BOOLEAN, "RESTRICT_INSTALLED",
	"true", _(
	"If true, calculate RESTRICT/PROPERTIES for installed versions."));

AddOption(BOOLEAN, "CARE_RESTRICT_INSTALLED",
	"true", _(
	"If true, read RESTRICT for installed versions always from disk.\n"
	"This is ignored if PROPERTIES_INSTALLED=false."));

AddOption(STRING, "DEFAULT_FORMAT",
	"normal", _(
	"Defines whether --compact or --verbose is on by default."));

AddOption(BOOLEAN, "PRINT_ALWAYS",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether all information lines are printed (even if empty)."));

AddOption(BOOLEAN, "PRINT_BUGS",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether a bug reference is printed in verbose format."));

AddOption(BOOLEAN, "DIFF_PRINT_INSTALLED",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether eix-diff will output installed versions."));

AddOption(STRING, "COLOR_TITLE",
	"green",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts.");

AddOption(STRING, "COLOR_NAME",
	"default,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the name of packages."));

AddOption(STRING, "COLOR_WORLD",
	"green,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the name of world packages."));

AddOption(STRING, "COLOR_WORLD_SETS",
	"yellow,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the name of world sets packages."));

AddOption(STRING, "COLOR_CATEGORY",
	"", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of packages."));

AddOption(STRING, "COLOR_CATEGORY_SYSTEM",
	"yellow", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of system packages."));

AddOption(STRING, "COLOR_CATEGORY_WORLD",
	"%{COLOR_WORLD}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of world packages."));

AddOption(STRING, "COLOR_CATEGORY_WORLD_SETS",
	"%{COLOR_WORLD_SETS}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of world sets packages."));

AddOption(STRING, "COLOR_UPGRADE_TEXT",
	"cyan,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation texts."));

AddOption(STRING, "COLOR_DOWNGRADE_TEXT",
	"purple,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation texts."));

AddOption(STRING, "COLOR_UPGRADE",
	"%{COLOR_UPGRADE_TEXT};inverse", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation tags."));

AddOption(STRING, "COLOR_DOWNGRADE",
	"%{COLOR_DOWNGRADE_TEXT};inverse", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation tags."));

AddOption(STRING, "DIFF_COLOR_UNINST_STABILIZE",
	"%{COLOR_STABLE},1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for uninstalled packages\n"
	"which have gained a stable version."));

AddOption(STRING, "DIFF_COLOR_INST_STABILIZE",
	"%{DIFF_COLOR_UNINST_STABILIZE},1;inverse", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for installed packages\n"
	"which have gained a stable version."));

AddOption(STRING, "DIFF_COLOR_BETTER",
	"yellow,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"better version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_WORSE",
	"red,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"worse version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_NEW_TAG",
	"%{DIFF_COLOR_NEW}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_NEW",
	"%{COLOR_TITLE},1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_DELETE",
	"red,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"deleted package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_CHANGED",
	"yellow", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"changed package\" separators (eix-diff)."));

AddOption(STRING, "COLOR_INST_TAG",
	"%{COLOR_TITLE},1;inverse", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging installed packages."));

AddOption(STRING, "COLOR_UNINST_TAG",
	"%{COLOR_TITLE}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging uninstalled packages."));

AddOption(STRING, "COLOR_DATE",
	"purple", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the date."));

AddOption(STRING, "COLOR_SET_USE",
	"red,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the set useflags."));

AddOption(STRING, "COLOR_UNSET_USE",
	"black,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the unset useflags."));

AddOption(STRING, "COLOR_INST_VERSION",
	"blue,1;%{MARK_INSTALLED}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the version of installed packages."));

AddOption(STRING, "COLOR_INST_TITLE",
	"blue", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for installed versions."));

AddOption(STRING, "COLOR_MARKED_VERSION",
	"%{COLOR_MARKED_NAME}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked version of a packages."));

AddOption(STRING, "COLOR_PACKAGESETS",
	"yellow,1", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the package sets."));

AddOption(STRING, "COLOR_MARKED_NAME",
	"red,1;%{MARK_VERSIONS}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked package name."));

AddOption(STRING, "CHAR_UPGRADE",
	"U", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for upgrade recommendations."));

AddOption(STRING, "CHAR_DOWNGRADE",
	"D", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for downgrade recommendations."));

AddOption(STRING, "CHAR_INSTALLED",
	"I", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for installed packages."));

AddOption(STRING, "CHAR_UNINSTALLED",
	"%{DIFF_CHAR_NEW}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for uninstalled packages."));

AddOption(STRING, "DIFF_CHAR_UNINST_STABILIZE",
	"%{DIFF_CHAR_INST_STABILIZE}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for uninstalled packages which have gained\n"
	"a stable version."));

AddOption(STRING, "DIFF_CHAR_INST_STABILIZE",
	"*", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for installed packages which have gained\n"
	"a stable version."));

AddOption(STRING, "DIFF_CHAR_NEW",
	"N", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for new packages (eix-diff)."));

AddOption(STRING, "DIFF_CHAR_BETTER",
	">", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character used for \"better version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_CHAR_WORSE",
	"\\<", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the character used for \"worse version\" tags (eix-diff)."));

AddOption(STRING, "TAG_UPGRADE",
	"(%{COLOR_UPGRADE})%{CHAR_UPGRADE}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for upgrade recommendations."));

AddOption(STRING, "TAG_DOWNGRADE",
	"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations."));

AddOption(STRING, "TAG_DOWNGRADE",
	"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations."));

AddOption(STRING, "TAG_INSTALLED",
	"(%{COLOR_INST_TAG})%{CHAR_INSTALLED}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations."));

AddOption(STRING, "TAG_UNINSTALLED",
	"(%{COLOR_UNINST_TAG})%{CHAR_UNINSTALLED}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations."));

AddOption(STRING, "TAG_STABILIZE",
	"{installedversionsshort}"
		"(%{DIFF_COLOR_INST_STABILIZE})%{DIFF_CHAR_INST_STABILIZE}"
	"{else}"
		"(%{DIFF_COLOR_UNINST_STABILIZE})%{DIFF_CHAR_UNINST_STABILIZE}"
	"{}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag for packages which have gained a stable version."));

AddOption(STRING, "TAG_NEW",
	"(%{DIFF_COLOR_NEW_TAG})%{DIFF_CHAR_NEW}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for new packages (eix-diff)."));

AddOption(STRING, "TAG_BETTER",
	"(%{DIFF_COLOR_BETTER})%{DIFF_CHAR_BETTER}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"better version\" (eix-diff)."));

AddOption(STRING, "TAG_WORSE",
	"(%{DIFF_COLOR_WORSE})%{DIFF_CHAR_WORSE}()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"worse version\" (eix-diff)."));

AddOption(STRING, "STRING_PLAIN_INSTALLED",
	"%{STRING_PLAIN_UNINSTALLED}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"plain\" tagging installed packages."));

AddOption(STRING, "STRING_PLAIN_UNINSTALLED",
	"*", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"plain\" tagging uninstalled packages."));

AddOption(STRING, "DIFF_STRING_NEW",
	">>", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"new package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_STRING_DELETE",
	"\\<\\<", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"deleted package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_STRING_CHANGED",
	"==", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"changed package\" separators (eix-diff)."));
#endif

#if (DEFAULT_PART == 2)

AddOption(STRING, "FORMAT_INST_PROPERTIES",
	":(%{COLOR_PROPERTIES_INTERACTIVE})%{TAG_PROPERTIES_INTERACTIVE}"
	":(%{COLOR_PROPERTIES_LIVE})%{TAG_PROPERTIES_LIVE}"
	":(%{COLOR_PROPERTIES_VIRTUAL})%{TAG_PROPERTIES_VIRTUAL}"
	":(%{COLOR_PROPERTIES_SET})%{TAG_PROPERTIES_SET}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the PROPERTIES part of the installedversions format.\n"
	"Do not omit trailing parts here."));

AddOption(STRING, "FORMAT_INST_RESTRICT",
	":(%{COLOR_RESTRICT_FETCH})%{TAG_RESTRICT_FETCH}"
	":(%{COLOR_RESTRICT_MIRROR})%{TAG_RESTRICT_MIRROR}"
	":(%{COLOR_RESTRICT_PRIMARYURI})%{TAG_RESTRICT_PRIMARYURI}"
	":(%{COLOR_RESTRICT_BINCHECKS})%{TAG_RESTRICT_BINCHECKS}"
	":(%{COLOR_RESTRICT_STRIP})%{TAG_RESTRICT_STRIP}"
	":(%{COLOR_RESTRICT_TEST})%{TAG_RESTRICT_TEST}"
	":(%{COLOR_RESTRICT_USERPRIV})%{TAG_RESTRICT_USERPRIV}"
	":(%{COLOR_RESTRICT_INSTALLSOURCES})%{TAG_RESTRICT_INSTALLSOURCES}"
	":(%{COLOR_RESTRICT_BINDIST})%{TAG_RESTRICT_BINDIST}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the RESTRICT part of the installedversions format.\n"
	"Do not omit trailing parts here."));

AddOption(STRING, "FORMAT_INST_PROPRESTRICT",
	"%{FORMAT_INST_PROPERTIES}%{FORMAT_INST_RESTRICT}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the PROPERTIES/RESTRICT part of INSTALLEDVERSIONS.\n"
	"Trailing parts can be omitted."));

AddOption(STRING, "FORMAT_INST_PROPRESTRICT_COMPACT",
	"", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the PROPERTIES/RESTRICT part of INSTALLEDVERSIONS_COMPACT.\n"
	"Trailing parts can be omitted."));

AddOption(STRING, "FORMAT_INST_PROPRESTRICT_VERBOSE",
	"%{FORMAT_INST_PROPERTIES}%{FORMAT_INST_RESTRICT}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the PROPERTIES/RESTRICT part of INSTALLEDVERSIONS_VERBOSE.\n"
	"Do not omit trailing parts here."));

AddOption(STRING, "FORMAT_INST_USEFLAGS",
	":(%{COLOR_SET_USE}):():(%{COLOR_UNSET_USE})-:()", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the Useflags part of INSTALLEDVERSIONS*."));

AddOption(STRING, "INSTALLEDVERSIONS",
	"<installedversions"
	":(%{COLOR_INST_VERSION}):():()"
	":::"
	":(%{COLOR_DATE})\\(:\\)()"
	":\\(:\\)"
	"%{FORMAT_INST_USEFLAGS}"
	"%{FORMAT_INST_PROPRESTRICT}"
	// ":"
	">", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions normally."));

AddOption(STRING, "INSTALLEDVERSIONS_COMPACT",
	"<installedversionsshortdate"
	":(%{COLOR_INST_VERSION}):():()"
	":::"
	":@(%{COLOR_DATE}):()"
	":\\(:\\)"
	"%{FORMAT_INST_USEFLAGS}"
	"%{FORMAT_INST_PROPRESTRICT_COMPACT}"
	// ":"
	">", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions compactly."));

AddOption(STRING, "INSTALLEDVERSIONS_VERBOSE",
	"<installedversions"
	":(%{COLOR_INST_TITLE})Version\\\\:() (%{COLOR_INST_VERSION}):():()"
	":::"
	":\\n                          (%{COLOR_INST_TITLE})Date\\\\:()    "
	"(%{COLOR_DATE}):()"
	":\\n                          (%{COLOR_INST_TITLE})USE\\\\:()     "
	":"
	"%{FORMAT_INST_USEFLAGS}"
	"%{FORMAT_INST_PROPRESTRICT_VERBOSE}"
	":\\n                          "
	">", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions verbosely."));

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
	"%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the \"normal\" format for a line with installed versions."));

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
	"%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the verbose format for a line with installed versions."));

AddOption(STRING, "DIFF_FORMATLINE_INSTALLEDVERSIONS",
	"{installedversionsshort}%{INSTALLEDVERSIONS_COMPACT}; {}"
	"", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for eix-diff for installed versions."));

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
	"{}<name>()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the printing the package name."));

AddOption(STRING, "FORMAT_HEADER",
	"{installedversionsshort}["
	"{upgrade}%{TAG_UPGRADE}{}"
	"{downgrade}%{TAG_DOWNGRADE}{}"
	"{!recommend}%{TAG_INSTALLED}{}"
	"]{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the normal header symbols."));

AddOption(STRING, "FORMAT_HEADER_VERBOSE",
	"{installedversionsshort}(%{COLOR_INST_TAG})%{STRING_PLAIN_INSTALLED}"
	"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}"
	"()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the verbose header symbols."));

AddOption(STRING, "FORMAT_HEADER_COMPACT",
	"[{installedversionsshort}"
		"{upgrade}%{TAG_UPGRADE}{}"
		"{downgrade}%{TAG_DOWNGRADE}{}"
		"{!recommend}%{TAG_INSTALLED}{}"
	"{else}"
		"%{TAG_UNINSTALLED}"
	"{}]", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the compact header symbols."));

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
	" (%{DIFF_COLOR_NEW})%{DIFF_STRING_NEW}()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-new header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_DELETE",
	"{installedversionsshort}"
		"[(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}()]"
	"{else}"
		"   "
	"{}"
	"   (%{DIFF_COLOR_DELETE})%{DIFF_STRING_DELETE}()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-delete header symbols."));

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
	" (%{DIFF_COLOR_CHANGED})%{DIFF_STRING_CHANGED}()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-changed header symbols."));

AddOption(BOOLEAN, "NOBEST_COMPACT",
	"true", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, compact format prints no version if none is installable."));

AddOption(BOOLEAN, "NOBEST_CHANGE",
	"true", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, compact format prints no versions if all installable vanished."));

AddOption(BOOLEAN, "DIFF_NOBEST",
	"false", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, eix-diff prints no version if none is installable."));

AddOption(BOOLEAN, "DIFF_NOBEST_CHANGE",
	"false", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"If true, eix-diff prints no versions if all installable vanished."));

AddOption(STRING, "FORMAT_NOBEST",
	"(%{COLOR_MASKED})"
	"--"
	"()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines what to print if no version number is printed."));

AddOption(STRING, "FORMAT_NOBEST_CHANGE",
	"(%{COLOR_MASKED})"
	"??"
	"()", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines what to print after \"->\" if there is no installable."));

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
	"{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the compact format for the best version(s)."));

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
	"{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the compact format for the best version(s) in case of changes."));

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
	"{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the eix-diff format for the best version(s)."));

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
	"{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the eix-diff format for the best version(s) in case of changes."));

AddOption(STRING, "DIFF_FORMAT_CHANGED_VERSIONS",
	"%{?DIFF_PRINT_INSTALLED}"
		"%{DIFF_FORMATLINE_INSTALLEDVERSIONS}"
	"%{}"
	"{oldbestshort}"
		"<oldbestslots>"
	"{else}"
		"<oldavailableversions>"
	"{}"
	"() -> %{DIFF_FORMAT_BEST_CHANGE}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the eix-diff format for changed versions."));

AddOption(STRING, "FORMAT_OVERLAYKEY",
	"{overlaykey} <overlaykey>{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the printing the optional overlay key."));

AddOption(STRING, "FORMATLINE_NAME",
	"%{FORMAT_HEADER} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the normal header line."));

AddOption(STRING, "FORMATLINE_NAME_VERBOSE",
	"%{FORMAT_HEADER_VERBOSE} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the verbose header line."));

AddOption(STRING, "FORMATLINE_NAME_COMPACT",
	"%{FORMAT_HEADER_COMPACT} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the compact header line."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_NEW",
	"%{DIFF_FORMAT_HEADER_NEW} %{FORMAT_NAME} ", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-new header symbols."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_DELETE",
	"%{DIFF_FORMAT_HEADER_DELETE} %{FORMAT_NAME} ", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-delete header symbols."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_CHANGED",
	"%{DIFF_FORMAT_HEADER_CHANGED} %{FORMAT_NAME} ", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for the diff-changed header symbols."));

AddOption(STRING, "FORMATLINE_AVAILABLEVERSIONS",
	"     (%{COLOR_TITLE})Available versions:()  <availableversions>\\n", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with installed versions."));

AddOption(STRING, "DIFF_FORMATLINE_BEST",
	"\\(%{DIFF_FORMAT_BEST}())", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the eix-diff line for the best versions/slots."));

AddOption(STRING, "DIFF_FORMATLINE_CHANGED_VERSIONS",
	"\\(%{DIFF_FORMAT_CHANGED_VERSIONS})", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the eix-diff line for changed versions."));

AddOption(STRING, "FORMATLINE_MARKEDVERSIONS",
	"%{!PRINT_ALWAYS}{marked}%{}"
	"     (%{COLOR_TITLE})Marked:()"
	"%{?PRINT_ALWAYS}{marked}%{}"
	"              "
	"(%{COLOR_MARKED_VERSION})<markedversions>()"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with marked versions."));

AddOption(BOOLEAN, "ALL_SETNAMES",
	"true", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines whether to include \"system\" in package sets output."));

AddOption(STRING, "PRINT_SETNAMES",
	"%{?ALL_SETNAMES}all%{}setnames", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It is the command used to print the package set names."));

AddOption(STRING, "FORMATLINE_PACKAGESETS",
	"%{!PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
	"     (%{COLOR_TITLE})Package sets:()"
	"%{?PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
	"        "
	"(%{COLOR_PACKAGESETS})<%{PRINT_SETNAMES}>()"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with package sets."));

AddOption(STRING, "FORMATLINE_HOMEPAGE",
	"%{!PRINT_ALWAYS}{homepage}%{}"
	"     (%{COLOR_TITLE})Homepage:()"
	"%{?PRINT_ALWAYS}{homepage}%{}"
	"            "
	"<homepage>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package homepage."));

AddOption(STRING, "FORMATLINE_BUGS",
	"     (%{COLOR_TITLE})Find open bugs:()"
	"      "
	"http://bugs.gentoo.org/buglist.cgi?quicksearch="
	"<category>%2F<name>"
	"\\n", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package bug-reference."));

AddOption(STRING, "FORMATLINE_DESCRIPTION",
	"%{!PRINT_ALWAYS}{description}%{}"
	"     (%{COLOR_TITLE})Description:()"
	"%{?PRINT_ALWAYS}{description}%{}"
	"         "
	"<description>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package description."));

AddOption(STRING, "FORMATLINE_BEST",
	"%{!PRINT_ALWAYS}{bestshort}%{}"
	"     (%{COLOR_TITLE})Best versions/slot:()"
	"%{?PRINT_ALWAYS}{bestshort}%{}"
	"  "
	"<bestslots>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the best versions/slots."));

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
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the up-/downgrade recommendations."));

AddOption(STRING, "FORMATLINE_PROVIDE",
	"%{!PRINT_ALWAYS}{provide}%{}"
	"     (%{COLOR_TITLE})Provides:()"
	"%{?PRINT_ALWAYS}{provide}%{}"
	"            "
	"<provide>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package provides."));

AddOption(STRING, "FORMATLINE_LICENSES",
	"%{!PRINT_ALWAYS}{licenses}%{}"
	"     (%{COLOR_TITLE})License:()"
	"%{?PRINT_ALWAYS}{licenses}%{}"
	"             "
	"<licenses>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for a line with the package licenses."));

AddOption(STRING, "DIFF_FORMATLINE",
	"%{FORMAT_OVERLAYKEY}: <description>", _(
	"This variable is only used for delayed substitution in *FORMAT_* strings.\n"
	"It defines the format for eix-diff after the versions."));
#endif

#if (DEFAULT_PART == 3)

AddOption(STRING, "FORMAT_ALL",
	"%{FORMATLINE_NAME}"
	"%{FORMATLINE_AVAILABLEVERSIONS}"
	"%{FORMATLINE_INSTALLEDVERSIONS}"
	"%{FORMATLINE_MARKEDVERSIONS}"
	"%{FORMATLINE_HOMEPAGE}"
	"%{FORMATLINE_DESCRIPTION}", _(
	"This format is only used for delayed substitution in FORMAT.\n"
	"It defines the format of the normal output of eix."));

AddOption(STRING, "FORMAT_ALL_COMPACT",
	"%{FORMATLINE_NAME_COMPACT}"
	" \\({marked}(%{COLOR_MARKED_VERSION})<markedversions>(); {}"
	"{installedversionsshort}"
		"%{INSTALLEDVERSIONS_COMPACT}"
		"{recommend} -> %{FORMAT_BEST_CHANGE}{}"
	"{else}"
		"%{FORMAT_BEST_COMPACT}"
	"{}"
	"()\\): <description>", _(
	"This format is only used for delayed substitution in FORMAT_COMPACT.\n"
	"It defines the format of the compact output of eix (option -c)."));

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
	"%{FORMATLINE_LICENSES}", _(
	"This format is only used for delayed substitution in FORMAT_VERBOSE.\n"
	"It defines the format of the verbose output of eix (option -v)."));

AddOption(STRING, "FORMAT",
	"%{FORMAT_ALL}", _(
	"The format of the normal output of eix.\n"
	"Do not modify it in a config file; modify FORMAT_ALL instead."));

AddOption(STRING, "FORMAT_COMPACT",
	"%{FORMAT_ALL_COMPACT}", _(
	"The format of the compact output of eix (option -c).\n"
	"Do not modify it in a config file; modify FORMAT_ALL_COMPACT instead."));

AddOption(STRING, "FORMAT_VERBOSE",
	"%{FORMAT_ALL_VERBOSE}", _(
	"The format of the verbose output of eix (option -v).\n"
	"Do not modify it in a config file; modify FORMAT_ALL_VERBOSE instead."));

AddOption(STRING, "DIFF_FORMAT_ALL_NEW",
	"%{DIFF_FORMATLINE_NAME_NEW}"
	"%{DIFF_FORMATLINE_BEST}"
	"%{DIFF_FORMATLINE}", _(
	"This format is only used for delayed substitution in DIFF_FORMAT_NEW.\n"
	"It defines the format used for new packages (eix-diff)."));

AddOption(STRING, "DIFF_FORMAT_ALL_DELETE",
	"%{DIFF_FORMATLINE_NAME_DELETE}"
	"%{DIFF_FORMATLINE_BEST}"
	"%{DIFF_FORMATLINE}", _(
	"This format is only used for delayed substitution in DIFF_FORMAT_DELETE.\n"
	"It defines the format used for packages that were deleted (eix-diff)."));

AddOption(STRING, "DIFF_FORMAT_ALL_CHANGED",
	"%{DIFF_FORMATLINE_NAME_CHANGED}"
	"%{DIFF_FORMATLINE_CHANGED_VERSIONS}"
	"%{DIFF_FORMATLINE}", _(
	"This format is only used for delayed substitution in DIFF_FORMAT_CHANGED.\n"
	"It defines the format used for packages that were changed (eix-diff)."));

AddOption(STRING, "DIFF_FORMAT_NEW",
	"%{DIFF_FORMAT_ALL_NEW}", _(
	"The format used for new packages (eix-diff).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_NEW instead."));

AddOption(STRING, "DIFF_FORMAT_DELETE",
	"%{DIFF_FORMAT_ALL_DELETE}", _(
	"The format used for packages that were deleted (eix-diff).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_DELETE instead."));

AddOption(STRING, "DIFF_FORMAT_CHANGED",
	"%{DIFF_FORMAT_ALL_CHANGED}", _(
	"The format used for packages that were changed (eix-diff).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_CHANGED instead."));

AddOption(STRING, "FORMAT_INSTALLATION_DATE",
	"%X %x", _(
	"strftime() format for printing the installation date in long form"));

AddOption(STRING, "FORMAT_SHORT_INSTALLATION_DATE",
	"%x", _(
	"strftime() format for printing the installation date in short form"));

AddOption(STRING, "FORMAT_BEFORE_KEYWORDS",
	" \"(cyan)", _(
	"This string is printed before KEYWORDS string for a version is output.\n"
	"(This is only used when --versionlines and PRINT_KEYWORDS are active)"));

AddOption(STRING, "FORMAT_AFTER_KEYWORDS",
	"()\"", _(
	"This string is printed after KEYWORDS string for a version is output.\n"
	"(This is only used when --versionlines and PRINT_KEYWORDS are active)"));

AddOption(STRING, "PRINT_EFFECTIVE_KEYWORDS",
	"true", _(
	"Print effective keywords if the profile modified those of the ebuild."));

AddOption(STRING, "FORMAT_BEFORE_EFFECTIVE_KEYWORDS",
	" -> \"(cyan)", _(
	"If PRINT_EFFECTIVE_KEYWORDS applies this string is printed after the\n"
	"keywords and before the effective keywords string.\n"
	"(This is only used when --versionlines and PRINT_KEYWORDS are active)"));

AddOption(STRING, "FORMAT_AFTER_EFFECTIVE_KEYWORDS",
	"()\"", _(
	"This string is printed after the effective keywords (if these are printed).\n"
	"(This is only used when --versionlines and PRINT_KEYWORDS are active)"));

AddOption(STRING, "FORMAT_BEFORE_IUSE",
	" [(blue)", _(
	"This string is printed before IUSE data for a version is output.\n"
	"(This is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_AFTER_IUSE",
	"()]", _(
	"This string is printed after IUSE data for a version is output.\n"
	"(This is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_BEFORE_COLL_IUSE",
	" \\{(blue)", _(
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is only used when --versionlines is inactive and there are no slots)."));

AddOption(STRING, "FORMAT_AFTER_COLL_IUSE",
	"()\\}", _(
	"This string is printed after IUSE data for all versions is output.\n"
	"(This is only used when --versionlines is inactive and there are no slots)."));

AddOption(STRING, "FORMAT_BEFORE_SLOT_IUSE",
	"\\n\\t\\{(blue)", _(
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is only used when --versionlines is inactive and sorting is by slots)."));

AddOption(STRING, "FORMAT_AFTER_SLOT_IUSE",
	"()\\}", _(
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is only used when --versionlines is inactive and sorting is by slots)."));

AddOption(STRING, "COLOR_MASKED",
	"red", _(
	"Define color for masked versions."));

AddOption(STRING, "COLOR_UNSTABLE",
	"yellow", _(
	"Define color for unstable versions"));

AddOption(STRING, "COLOR_STABLE",
	"green", _(
	"Define color for stable versions"));

AddOption(STRING, "COLOR_OVERLAYKEY",
	"cyan,1", _(
	"Color for the overlaykey in version listings."));

AddOption(STRING, "COLOR_VIRTUALKEY",
	"purple,1", _(
	"Color for the overlaykey for virtual overlays in version listings."));

AddOption(STRING, "COLOR_SLOTS",
	"red,1", _(
	"Color for slots."));

AddOption(STRING, "COLOR_RESTRICT",
	"red", _(
	"Color for the restriction tags. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_PROPERTIES",
	"cyan", _(
	"Color for the properties tags. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_RESTRICT_FETCH",
	"%{COLOR_RESTRICT}", _(
	"Color for the fetch restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_MIRROR",
	"%{COLOR_RESTRICT}", _(
	"Color for the mirror restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_PRIMARYURI",
	"%{COLOR_RESTRICT}", _(
	"Color for the primaryuri restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_BINCHECKS",
	"%{COLOR_RESTRICT}", _(
	"Color for the binchecks restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_STRIP",
	"%{COLOR_RESTRICT}", _(
	"Color for the strip restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_TEST",
	"%{COLOR_RESTRICT}", _(
	"Color for the test restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_USERPRIV",
	"%{COLOR_RESTRICT}", _(
	"Color for the userpriv restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_INSTALLSOURCES",
	"%{COLOR_RESTRICT}", _(
	"Color for the installsources restriction tag."));

AddOption(STRING, "COLOR_RESTRICT_BINDIST",
	"%{COLOR_RESTRICT}", _(
	"Color for the bindist restriction tag."));

AddOption(STRING, "COLOR_PROPERTIES_INTERACTIVE",
	"%{COLOR_PROPERTIES}", _(
	"Color for the interactive properties tag."));

AddOption(STRING, "COLOR_PROPERTIES_LIVE",
	"%{COLOR_PROPERTIES}", _(
	"Color for the live properties tag."));

AddOption(STRING, "COLOR_PROPERTIES_VIRTUAL",
	"%{COLOR_PROPERTIES}", _(
	"Color for the virtual properties tag."));

AddOption(STRING, "COLOR_PROPERTIES_SET",
	"%{COLOR_PROPERTIES}", _(
	"Color for the set properties tag."));

AddOption(STRING, "MARK_INSTALLED",
	"inverse", _(
	"How installed packages are marked in version listings."));

AddOption(STRING, "MARK_UPGRADE",
	"bold", _(
	"How upgrade candidates are marked in version listings."));

AddOption(STRING, "MARK_VERSIONS",
	"underline", _(
	"How the package versions passed with --pipe are marked in version listings."));

AddOption(BOOLEAN, "FORCE_USECOLORS",
	"false", _(
	"This turns --force-color on for every query."));

AddOption(BOOLEAN, "FORCE_PERCENTAGE",
	"false", _(
	"Show the percentage progress even in case of redirection."));

AddOption(BOOLEAN, "COLOR_ORIGINAL",
	"true", _(
	"If false, versions are only colored according to the local setting."));

AddOption(BOOLEAN, "COLOR_LOCAL_MASK",
	"false", _(
	"If false, COLOR_ORIGINAL=false has no effect on versions which are\n"
	"only locally masked (i.e. [m])."));

AddOption(BOOLEAN, "STYLE_VERSION_SORTED",
	"false", _(
	"Defines whether --versionsorted is on by default."));

AddOption(BOOLEAN, "PRINT_KEYWORDS",
	"first", _(
	"Defines whether KEYWORDS string is printed in --versionlines mode.\n"
	"\"before\", \"first\", or \"true\" will print KEYWORDS string before IUSE,\n"
	"\"after\" or \"last\" will print it afterwards. \"false\" will not print it."));

AddOption(BOOLEAN, "STYLE_VERSION_LINES",
	"false", _(
	"Defines whether --versionlines is on by default."));

AddOption(BOOLEAN, "COLORED_SLOTS",
	"true", _(
	"If false, the slotnames appended to versions are not colored."));

AddOption(BOOLEAN, "COLON_SLOTS",
	"false", _(
	"If true, separated slots from versions with a colon instead of braces."));

AddOption(BOOLEAN, "DEFAULT_IS_OR",
	"false", _(
	"Whether default concatenation of queries is -o (or) or -a (and)"));

AddOption(BOOLEAN, "DUP_PACKAGES_ONLY_OVERLAYS",
	"false", _(
	"Whether checks for duplicate packages occur only among overlays"));

AddOption(BOOLEAN, "DUP_VERSIONS_ONLY_OVERLAYS",
	"false", _(
	"Whether checks for duplicate versions occur only among overlays"));

AddOption(STRING, "OVERLAYS_LIST",
	"all-used-renumbered", _(
	"Which overlays to list (all/all-if-used/all-used/all-used-renumbered/no)"));

AddOption(INTEGER, "LEVENSHTEIN_DISTANCE",
	LEVENSHTEIN_DISTANCE_DEFAULT, _(
	"The default maximal levensthein distance for which a string is considered a match\n"
	"for the fuzzy match algorithm."));

AddOption(STRING, "CACHE_METHOD_PARSE",
	"#metadata-flat#assign", _(
	"This string is appended to all cache methods using parse[*] or ebuild[*]."));

AddOption(STRING, "PORTDIR_CACHE_METHOD",
	PORTDIR_CACHE_METHOD, _(
	"Portage cache-backend that should be used for PORTDIR\n"
	"(metadata[:*]/sqlite/cdb/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])"));

AddOption(STRING, "OVERLAY_CACHE_METHOD",
	"parse|ebuild*", _(
	"Portage cache-backend that should be used for the overlays.\n"
	"(metadata[:*]/sqlite/cdb/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])"));

AddOption(STRING, "ADD_CACHE_METHOD",
	"", _(
	"This variable is only used for delayed substitution in CACHE_METHOD.\n"
	"It is meant to be a local addition to CACHE_METHOD."));

AddOption(STRING, "CACHE_METHOD",
	"%{ADD_CACHE_METHOD}", _(
	"Overrides OVERLAY_CACHE_METHOD or PORTDIR_CACHE_METHOD for particular paths.\n"
	"This is a list of pairs DIR-PATTERN METHOD. Later entries take precedence."));

AddOption(STRING, "ADD_OVERRIDE_CACHE_METHOD",
	"", _(
	"This variable is only used for delayed substitution in OVERRIDE_CACHE_METHOD.\n"
	"It is meant to be a local addition to OVERRIDE_CACHE_METHOD."));

AddOption(STRING, "OVERRIDE_CACHE_METHOD",
	"%{ADD_OVERRIDE_CACHE_METHOD}", _(
	"This variable can override the choices of CACHE_METHOD,\n"
	"and in addition it can override the choices made by KEEP_VIRTUALS."));

AddOption(STRING, "EXCLUDE_OVERLAY",
	"", _(
	"List of overlays that should be excluded from the index."));

AddOption(STRING, "ADD_OVERLAY",
	"", _(
	"List of overlays that should be added to the index."));

AddOption(BOOLEAN, "EXPORT_PORTDIR_OVERLAY",
	"true", _(
	"If true and overlays are excluded or added, export modified PORTDIR_OVERLAY."));

AddOption(BOOLEAN, "KEEP_VIRTUALS",
	"false", _(
	"Keep virtuals of the old cache file by adding corresponding entries\n"
	"implicitly to the values of ADD_OVERLAY and ADD_CACHE_METHOD"));

AddOption(BOOLEAN, "LOCAL_PORTAGE_CONFIG",
	"true", _(
	"If false, /etc/portage and ACCEPT_KEYWORDS are ignored."));

AddOption(BOOLEAN, "ALWAYS_ACCEPT_KEYWORDS",
	"false", _(
	"If true, ACCEPT_KEYWORDS is used even without LOCAL_PORTAGE_CONFIG,\n"
	"e.g. to determine the \"default\" stability."));

AddOption(BOOLEAN, "UPGRADE_LOCAL_MODE",
	"", _(
	"If +/-, eix -u will match as if LOCAL_PORTAGE_CONFIG=true/false."));

AddOption(BOOLEAN, "RECOMMEND_LOCAL_MODE",
	"", _(
	"If +/-, recommendations for up- or downgrade will act as if\n"
	"LOCAL_PORTAGE_CONFIG=true/false."));

AddOption(BOOLEAN, "RECURSIVE_SETS",
	"true", _(
	"Are packages/sets in included sets part of the parent set?"));

AddOption(BOOLEAN, "UPGRADE_TO_HIGHEST_SLOT",
	"true", _(
	"If true, upgrade tests succeed for installed packages with new higher slots.\n"
	"Use the files SLOT_UPGRADE_FORBID or SLOT_UPGRADE_ALLOW, respectively,\n"
	"to specify exceptions."));

AddOption(BOOLEAN, "PRINT_SLOTS",
	"true", _(
	"If false, no slot information is printed."));

AddOption(BOOLEAN, "EIX_PRINT_IUSE",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If false, no IUSE data is printed for eix."));

AddOption(BOOLEAN, "DIFF_PRINT_IUSE",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If false, no IUSE data is printed for eix-diff."));

AddOption(BOOLEAN, "PRINT_IUSE",
	"%{*PRINT_IUSE}", _(
	"If false, no IUSE data is printed."));

AddOption(BOOLEAN, "SORT_INST_USE_ALPHA",
	"false", _(
	"If false, sort installed useflags by whether they are set."));

AddOption(STRING, "CHECK_INSTALLED_OVERLAYS",
	"repository", _(
	"Allowed values are true/false/repository.\n"
	"If true, always check from which overlay a package version was installed.\n"
	"If false, only packages with versions in at least two trees are checked.\n"
	"The compromise - repository - checks at least always the repository files.\n"
	"Without a check, the assumed overlay may be wrong if the version was\n"
	"actually installed from a place not in the database anymore."));

AddOption(BOOLEAN, "OBSOLETE_MINUSASTERISK",
	"false", _(
	"If true, treat -* in /etc/portage/package.keywords as <=portage-2.1.2\n"
	"Since >=portage-2.1.2-r4, -* is practically obsolete and replaced\n"
	"by ** which accepts anything (note that there are also * and ~*)."));

AddOption(STRING, "PRINT_COUNT_ALWAYS",
	"false", _(
	"Allowed values are true/false/never.\n"
	"If true, always print the number of matches (even 0 or 1) in the last line."));

AddOption(BOOLEAN, "COUNT_ONLY_PRINTED",
	"true", _(
	"If false, count independently of whether the matches are printed."));

AddOption(STRING, "TAG_RESTRICT_FETCH",
	"!f", _(
	"Tag for RESTRICT=fetch."));

AddOption(STRING, "TAG_RESTRICT_MIRROR",
	"!m", _(
	"Tag for RESTRICT=mirror."));

AddOption(STRING, "TAG_RESTRICT_PRIMARYURI",
	"!p", _(
	"Tag for RESTRICT=primaryuri."));

AddOption(STRING, "TAG_RESTRICT_BINCHECKS",
	"!b", _(
	"Tag for RESTRICT=binchecks."));

AddOption(STRING, "TAG_RESTRICT_STRIP",
	"!s", _(
	"Tag for RESTRICT=strip."));

AddOption(STRING, "TAG_RESTRICT_TEST",
	"!t", _(
	"Tag for RESTRICT=test."));

AddOption(STRING, "TAG_RESTRICT_USERPRIV",
	"!u", _(
	"Tag for RESTRICT=userpriv."));

AddOption(STRING, "TAG_RESTRICT_INSTALLSOURCES",
	"!i", _(
	"Tag for RESTRICT=installsources."));

AddOption(STRING, "TAG_RESTRICT_BINDIST",
	"!d", _(
	"Tag for RESTRICT=bindist."));

AddOption(STRING, "TAG_PROPERTIES_INTERACTIVE",
	"+i", _(
	"Tag for PROPERTIES=interactive."));

AddOption(STRING, "TAG_PROPERTIES_LIVE",
	"+l", _(
	"Tag for PROPERTIES=live."));

AddOption(STRING, "TAG_PROPERTIES_VIRTUAL",
	"+v", _(
	"Tag for PROPERTIES=virtual."));

AddOption(STRING, "TAG_PROPERTIES_SET",
	"+s", _(
	"Tag for PROPERTIES=set."));

AddOption(STRING, "TAG_FOR_PROFILE",
	"[P]", _(
	"Tag for \"profile masked\" versions."));

AddOption(STRING, "TAG_FOR_MASKED",
	"[M]", _(
	"Tag for \"package.masked\" versions."));

AddOption(STRING, "TAG_FOR_EX_PROFILE",
	"{P}", _(
	"Tag for \"originally profile masked but unmasked\" versions."));

AddOption(STRING, "TAG_FOR_EX_MASKED",
	"{M}", _(
	"Tag for \"originally package.masked but unmasked\" versions."));

AddOption(STRING, "TAG_FOR_LOCALLY_MASKED",
	"[m]", _(
	"Tag for \"only locally masked\" versions."));

AddOption(STRING, "TAG_FOR_STABLE",
	"", _(
	"Tag for \"stable\" versions."));

AddOption(STRING, "TAG_FOR_UNSTABLE",
	"~", _(
	"Tag for \"unstable\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_ASTERISK",
	"-*", _(
	"Tag for \"-*\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_KEYWORD",
	"-", _(
	"Tag for \"-ARCH\" versions."));

AddOption(STRING, "TAG_FOR_ALIEN_STABLE",
	"*", _(
	"Tag for \"ALIENARCH\" versions."));

AddOption(STRING, "TAG_FOR_ALIEN_UNSTABLE",
	"~*", _(
	"Tag for \"~ALIENARCH\" versions."));

AddOption(STRING, "TAG_FOR_MISSING_KEYWORD",
	"**", _(
	"Tag for \"no keyword\" versions."));

AddOption(STRING, "TAG_FOR_EX_UNSTABLE",
	"(~)", _(
	"Tag for \"originally unstable but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_ASTERISK",
	"(-*)", _(
	"Tag for \"originally -* but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_KEYWORD",
	"(-)", _(
	"Tag for \"originally -ARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_ALIEN_STABLE",
	"(*)", _(
	"Tag for \"originally ALIENARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_ALIEN_UNSTABLE",
	"(~*)", _(
	"Tag for \"originally ~ALIENARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MISSING_KEYWORD",
	"(**)", _(
	"Tag for \"originally no keyword but now stable\" versions."));

#endif

#if (DEFAULT_PART == 4)

AddOption(STRING, "MATCH_FIELD_DESCRIPTION",
	"[ ]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for description."));

AddOption(STRING, "MATCH_FIELD_SET",
	"[@]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for set."));

AddOption(STRING, "MATCH_FIELD_HOMEPAGE",
	"http.*:", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for homepage."));

AddOption(STRING, "MATCH_FIELD_VIRTUAL",
	"vir.*/", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for virtuals,\n"
	"that is for provide or category/name."));

AddOption(STRING, "MATCH_FIELD_PROVIDE",
	"virtual/", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for provide."));

AddOption(STRING, "MATCH_FIELD_CATEGORY_NAME",
	"/", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for category/name."));

AddOption(STRING, "MATCH_FIELD_LICENSE",
	"GPL|BSD|Art", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for license."));

AddOption(STRING, "DEFAULT_MATCH_FIELD",
	"%{\\MATCH_FIELD_DESCRIPTION} description "
	"%{\\MATCH_FIELD_SET} set "
	"%{\\MATCH_FIELD_HOMEPAGE} homepage "
	"%{\\MATCH_FIELD_PROVIDE} virtual "
	"%{\\MATCH_FIELD_PROVIDE} provide "
	"%{\\MATCH_FIELD_CATEGORY_NAME} category/name "
	"%{\\MATCH_FIELD_LICENSE} license "
	"name", _(
	"This is a list of strings of the form regexp[ ]match_field.\n"
	"If regexp matches the search pattern, use match_field as the default.\n"
	"A fallback match_field may be specified as the last entry in the list.\n"
	"Admissible values for match_field are: name, category, category/name,\n"
	"description, license, homepage, provide, set, slot, installed-slot, use\n"
	"with-use, without-use."));

AddOption(STRING, "MATCH_ALGORITHM_REGEX",
	"[][^$|()]|[.][*+?]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_ALGORITHM for regex."));

AddOption(STRING, "MATCH_ALGORITHM_PATTERN1",
	"^[*]|[^][().][*]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_ALGORITHM for pattern."));

AddOption(STRING, "MATCH_ALGORITHM_SUBSTRING",
	"[^][().][+?]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_ALGORITHM for substring."));

AddOption(STRING, "MATCH_ALGORITHM_EXACT",
	"^[@]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_ALGORITHM for exact."));

AddOption(STRING, "MATCH_ALGORITHM_PATTERN2",
	":", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_ALGORITHM for pattern."));

AddOption(STRING, "MATCH_ALGORITHM_BEGIN",
	"/", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_ALGORITHM for begin."));

AddOption(STRING, "DEFAULT_MATCH_ALGORITHM",
	"%{\\MATCH_ALGORITHM_REGEX} regex "
	"%{\\MATCH_ALGORITHM_PATTERN1} pattern "
	"%{\\MATCH_ALGORITHM_SUBSTRING} substring "
	"%{\\MATCH_ALGORITHM_EXACT} exact "
	"%{\\MATCH_ALGORITHM_PATTERN2} pattern "
	"%{\\MATCH_ALGORITHM_BEGIN} begin "
	"regex", _(
	"This is a list of strings of the form regexp[ ]match_algorithm.\n"
	"If regexp matches the search pattern, use match_algorithm as the default.\n"
	"A fallback match_algorithm may be specified as the last entry in the list.\n"
	"Admissible values for match_algorithm are: regex, pattern, substring,\n"
	"begin, end, exact, fuzzy."));

AddOption(BOOLEAN, "TEST_FOR_EMPTY",
	"true", _(
	"Defines whether empty entries in /etc/portage/package.* are shown with -t."));

AddOption(BOOLEAN, "TEST_KEYWORDS",
	"true", _(
	"Defines whether /etc/portage/package.keywords is tested with -t."));

AddOption(BOOLEAN, "TEST_MASK",
	"true", _(
	"Defines whether /etc/portage/package.mask is tested with -t."));

AddOption(BOOLEAN, "TEST_UNMASK",
	"true", _(
	"Defines whether /etc/portage/package.unmask is tested with -t."));

AddOption(BOOLEAN, "TEST_USE",
	"true", _(
	"Defines whether /etc/portage/package.use is tested with -t."));

AddOption(BOOLEAN, "TEST_CFLAGS",
	"true", _(
	"Defines whether /etc/portage/package.cflags is tested with -t."));

AddOption(BOOLEAN, "TEST_REMOVED",
	"true", _(
	"Defines whether removed packages are tested with -t."));

AddOption(BOOLEAN, "TEST_FOR_NONEXISTENT",
	"true", _(
	"Defines whether non-existing installed versions are positive for -T."));

AddOption(BOOLEAN, "NONEXISTENT_IF_OTHER_OVERLAY",
	"true", _(
	"Defines whether versions are non-existent for TEST_FOR_NONEXISTENT\n"
	"if they come from a different overlay than the installed version."));

AddOption(BOOLEAN, "NONEXISTENT_IF_MASKED",
	"true", _(
	"Defines whether masked versions are non-existent for TEST_FOR_NONEXISTENT."));

AddOption(BOOLEAN, "TEST_FOR_REDUNDANCY",
	"true", _(
	"Defines whether redundant entries are positive for -T."));

AddOption(BOOLEAN, "ACCEPT_KEYWORDS_AS_ARCH",
	"true", _(
	"If true modify ARCH by ACCEPT_KEYWORDS.\n"
	"This determines which keywords are considered as ARCH or OTHERARCH."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE",
	"some", _(
	"Applies if /etc/portage/package.keywords lists the same keyword twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_LINE",
	"some", _(
	"Applies if /etc/portage/package.keywords has two lines for identical target."));

AddOption(STRING, "REDUNDANT_IF_MIXED",
	"false", _(
	"Applies if /etc/portage/package.keywords lists two different keywords,\n"
	"e.g. ~ARCH and -*, for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_WEAKER",
	"all-installed", _(
	"Applies if /etc/portage/package.keywords lists a keywords which can\n"
	"be replaced by a weaker keyword, e.g. -* or ~OTHERARCH or OTHERARCH\n"
	"in place of ~ARCH, or ~OTHERARCH in place of OTHERARCH,\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_STRANGE",
	"some", _(
	"Applies if /etc/portage/package.keywords lists a strange keyword\n"
	"e.g. UNKNOWNARCH (unknown to the .ebuild and ARCH) or -OTHERARCH,\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_MINUSASTERISK",
	"some", _(
	"Applies if /etc/portage/package.keywords contains some -* entry.\n"
	"This test only applies if OBSOLETE_MINUSASTERISK is false."));

AddOption(STRING, "REDUNDANT_IF_NO_CHANGE",
	"all-installed", _(
	"Applies if /etc/portage/package.keywords provides keywords which do not\n"
	"change the availability keywords status for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_MASK_NO_CHANGE",
	"all-uninstalled", _(
	"Applies if /etc/portage/package.mask contains entries\n"
	"which do not change the mask status for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_UNMASK_NO_CHANGE",
	"all-installed", _(
	"Applies if /etc/portage/package.unmask contains entries\n"
	"which do not change the mask status for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_MASKED",
	"some", _(
	"Applies if /etc/portage/package.mask matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_UNMASKED",
	"some", _(
	"Applies if /etc/portage/package.unmask matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_USE",
	"some", _(
	"Applies if /etc/portage/package.use matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_CFLAGS",
	"some", _(
	"Applies if /etc/portage/package.cflags matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_IN_KEYWORDS",
	"-some", _(
	"Applies if /etc/portage/package.keywords contains a matching entry."));

AddOption(STRING, "REDUNDANT_IF_IN_MASK",
	"-some", _(
	"Applies if /etc/portage/package.mask matches."));

AddOption(STRING, "REDUNDANT_IF_IN_UNMASK",
	"-some", _(
	"Applies if /etc/portage/package.unmask matches."));

AddOption(STRING, "REDUNDANT_IF_IN_USE",
	"-some", _(
	"Applies if /etc/portage/package.use matches."));

AddOption(STRING, "REDUNDANT_IF_IN_CFLAGS",
	"-some", _(
	"Applies if /etc/portage/package.cflags matches."));

AddOption(STRING, "EIXCFGDIR",
	"%{PORTAGE_CONFIGROOT}/etc/portage", _(
	"This variable is only used for delayed substitution.\n"
	"It is the directory where eix searches for its package.*.*/sets.eix files."));

AddOption(BOOLEAN, "SLOT_UPGRADE_FORBID",
	"%{\\EIXCFGDIR}/package.slot_upgrade_forbid", _(
	"If UPGRADE_TO_HIGHEST_SLOT=true, then packages listed in these files/dirs\n"
	"are treated as if UPGRADE_TO_HIGHEST_SLOT=false."));

AddOption(BOOLEAN, "SLOT_UPGRADE_ALLOW",
	"%{\\EIXCFGDIR}/package.slot_upgrade_allow", _(
	"If UPGRADE_TO_HIGHEST_SLOT=false, then packages listed in these files/dirs\n"
	"are treated as if UPGRADE_TO_HIGHEST_SLOT=true."));

AddOption(STRING, "KEYWORDS_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.keywords.nonexistent", _(
	"Entries listed in these files/dirs are excluded for -t TEST_KEYWORDS."));

AddOption(STRING, "MASK_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.mask.nonexistent", _(
	"Entries listed in these files/dirs are excluded for -t TEST_MASK."));

AddOption(STRING, "UNMASK_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.unmask.nonexistent", _(
	"Entries listed in these files/dirs are excluded for -t TEST_UNMASK."));

AddOption(STRING, "USE_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.use.nonexistent", _(
	"Entries listed in these files/dire are excluded for -t TEST_USE."));

AddOption(STRING, "CFLAGS_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.cflags.nonexistent", _(
	"Entries listed in these files/dirs are excluded for -t TEST_CFLAGS."));

AddOption(STRING, "INSTALLED_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.installed.nonexistent", _(
	"Packages listed in these files/dirs are excluded for -t TEST_REMOVED."));

AddOption(STRING, "KEYWORDS_NOWARN",
	"%{\\EIXCFGDIR}/package.keywords.nowarn", _(
	"Exceptional packages for -T tests of /etc/portage/package.keywords."));

AddOption(STRING, "MASK_NOWARN",
	"%{\\EIXCFGDIR}/package.mask.nowarn", _(
	"Exceptional packages for -T tests of /etc/portage/package.mask."));

AddOption(STRING, "UNMASK_NOWARN",
	"%{\\EIXCFGDIR}/package.unmask.nowarn", _(
	"Exceptional packages for -T tests of /etc/portage/package.unmask."));

AddOption(STRING, "USE_NOWARN",
	"%{\\EIXCFGDIR}/package.use.nowarn", _(
	"Exceptional packages for -T tests of /etc/portage/package.use."));

AddOption(STRING, "CFLAGS_NOWARN",
	"%{\\EIXCFGDIR}/package.cflags.nowarn", _(
	"Exceptional packages for -T tests of /etc/portage/package.cflags."));

AddOption(STRING, "INSTALLED_NOWARN",
	"%{\\EIXCFGDIR}/package.installed.nowarn", _(
	"Exceptional packages for -T tests of installed packages."));

AddOption(STRING, "EIX_LOCAL_SETS_ADD",
	"", _(
	"This variable is only used for delayed substitution.\n"
	"It specifies directories for EIX_LOCAL_SETS."));

AddOption(STRING, "EIX_LOCAL_SETS",
	"%{EIX_LOCAL_SETS_ADD} "
	"%{\\EIXCFGDIR}/sets.eix "
	"%{\\PORTAGE_CONFIGROOT}/etc/portage/sets "
	"*/sets "
	"sets", _(
	"This is a space-separated list of directories containing set definitions."));

AddOption(STRING, "EAPI_REGEX",
	"[0-9]+", _(
	"This regular expression describes the recognized EAPIs in .ebuild suffixes.\n"
	"You might need to modify it according to the installed portage version.\n"
	"Leave it empty if EAPI-suffixed ebuilds (GLEP 55) should be ignored."));

AddOption(STRING, "DUMMY",
	"", _(
	"This variable is ignored. You can use it to collect delayed references to\n"
	"locally added (unused) variables so that they are printed with --dump."));

#endif
