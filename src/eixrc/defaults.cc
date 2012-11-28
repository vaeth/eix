// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

// This file is supposed to be included by a wrapper which should contain
// #include <config.h>
// #include "eixTk/i18n.h"
// #include <cstdlib>

#define AddOption(opt_type, opt_name, opt_default, opt_description) \
	eixrc->addDefault(EixRcOption(EixRcOption::opt_type, opt_name, \
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
	"EIX_REMOTE_OPTS='-f /var/lib/layman/eix-caches.tar.bz2'"));

AddOption(STRING, "EIX_LAYMAN_OPTS",
	"", _(
	"This variable contains default options for the eix-layman script.\n"
	"Note that its content is evaluated, so quote correctly."));

AddOption(STRING, "EIX_TEST_OBSOLETE_OPTS",
	"-d", _(
	"This variable contains default options for the eix-test-obsolete script."));

AddOption(STRING, "EIX_INSTALLED_AFTER",
	"", _(
	"This variable contains default arguments for the eix-installed-after script."));

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

AddOption(STRING, "EIX_PREVIOUS",
	"%{EPREFIX}" EIX_PREVIOUS, _(
	"This file is the previous eix cache (used by eix-diff and eix-sync)."));

AddOption(STRING, "EIX_REMOTE",
	"%{EPREFIX}" EIX_REMOTECACHEFILE, _(
	"This is the eix cache used when -R is in effect. If the string is nonempty,\n"
	"eix-remote uses this file for the output."));

AddOption(BOOLEAN, "REMOTE_DEFAULT",
	"false", _(
	"Whether eix option -R should be on by default."));

AddOption(STRING, "EIX_REMOTEARCHIVE",
	"%{EPREFIX}" EIX_REMOTEARCHIVE, _(
	"This is a local copy of the remote archive used by eix-remote.\n"
	"If the name is empty, only a temporary file is used."));

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

AddOption(STRING, "EIX_USER",
	"portage", _(
	"Attempt to change to this user if possible. See EIX_UID."));

AddOption(STRING, "EIX_GROUP",
	"%{EIX_USER}", _(
	"Attempt to change to this group if possible. See EIX_GID."));

AddOption(INTEGER, "EIX_UID",
	"250", _(
	"If EIX_USER is empty or nonexistent, use this user id.\n"
	"In this case and if ${EIX_UID} <= 0, the user id is not changed."));

AddOption(INTEGER, "EIX_GID",
	"%{EIX_UID}", _(
	"If EIX_GROUP is empty or nonexistent, use this group id.\n"
	"In this case and if ${EIX_GID} <= 0, the group id is not changed."));

AddOption(STRING, "PORTAGE_ROOTPATH",
	PORTAGE_ROOTPATH_DEFAULT, _(
	"This variable is passed unchanged to ebuild.sh\n"
	"Usually ebuild.sh uses it to calculate the PATH."));

AddOption(STRING, "PORTAGE_BIN_PATH",
	PORTAGE_BIN_PATH_DEFAULT, _(
	"This variable is passed unchanged to ebuild.sh\n"
	"Usually ebuild.sh uses it to calculate the PATH."));

AddOption(STRING, "DEFAULT_ARCH",
	ARCH_DEFAULT, _(
	"The default ARCH if none is specified by the profile."));

AddOption(INTEGER, "NOFOUND_STATUS",
	EXPAND_STRINGIFY(EXIT_FAILURE), _(
	"This value is used as exit status if there are 0 matches.\n"
	"The value of COUNT_ONLY_PRINTED is honoured."));

AddOption(INTEGER, "MOREFOUND_STATUS",
	EXPAND_STRINGIFY(EXIT_SUCCESS), _(
	"This value is used as exit status if there are 2 or more matches.\n"
	"The value of COUNT_ONLY_PRINTED is honoured."));

AddOption(BOOLEAN, "QUICKMODE",
	"false", _(
	"Whether --quick is on by default."));

AddOption(BOOLEAN, "CAREMODE",
	"false", _(
	"Whether --care is on."));

AddOption(BOOLEAN, "USE_BUILD_TIME",
	"true", _(
	"If true, use build time from BUILD_TIME entry instead of reading the install\n"
	"time from the directory timestamp. This is usually preferable but slower.\n"
	"The BUILD_TIME exists only for packages emerged with >=portage-2.2_rc63"));

AddOption(BOOLEAN, "QUIETMODE",
	"false", _(
	"Whether --quiet is on by default."));

AddOption(STRING, "PRINT_APPEND",
	"\\n", _(
	"This string is appended to the output of --print.\n"
	"To read variables in a shell without omitting trailing spaces, use e.g.\n"
	"VAR=`PRINT_APPEND=x eix --print VAR` ; VAR=${VAR%x}"));

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
	"This variable is only used for delayed substitution.\n"
	"If false, RESTRICT and PROPERTIES values are output."));

AddOption(BOOLEAN, "NO_BINARY",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If false, tags are output for packages/versions with *.tbz2 files."));

AddOption(BOOLEAN, "EIX_USE_EXPAND",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, USE_EXPAND variables are output separately."));

AddOption(BOOLEAN, "RESTRICT_INSTALLED",
	"true", _(
	"If true, calculate RESTRICT/PROPERTIES for installed versions."));

AddOption(BOOLEAN, "CARE_RESTRICT_INSTALLED",
	"true", _(
	"If true, read RESTRICT for installed versions always from disk.\n"
	"This is ignored if RESTRICT_INSTALLED=false."));

AddOption(BOOLEAN, "DEP",
	DEP_DEFAULT, _(
	"If true, store/use {R,P,H,}DEPEND (e.g. shown with eix -lv).\n"
	"Usage of DEP roughly doubles disk resp. memory requirements."));

AddOption(STRING, "DEFAULT_FORMAT",
	"normal", _(
	"Defines whether --compact or --verbose is on by default."));

AddOption(STRING, "PRINT_COUNT_ALWAYS",
	"false", _(
	"Allowed values are true/false/never.\n"
	"If true, always print the number of matches (even 0 or 1) in the last line."));

AddOption(BOOLEAN, "NOCOLORS",
	"%{NOCOLOR}", _(
	"Do not output colors."));

AddOption(BOOLEAN, "NOSTATUSLINE",
	"%{NOCOLOR}", _(
	"Do not output status line."));

AddOption(BOOLEAN, "NOPERCENTAGE",
	"%{NOCOLOR}", _(
	"Do not output percentage progress."));

AddOption(BOOLEAN, "FORCE_USECOLORS",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"It is the default for FORCE_{COLORS,STATUSLINE,PERCENTAGE}."));

AddOption(BOOLEAN, "FORCE_COLORS",
	"%{FORCE_USECOLORS}", _(
	"Output colors even if not printing to a terminal."));

AddOption(BOOLEAN, "FORCE_STATUSLINE",
	"%{FORCE_USECOLORS}", _(
	"Output status line even if not printing to a terminal."));

AddOption(BOOLEAN, "FORCE_PERCENTAGE",
	"%{FORCE_USECOLORS}", _(
	"Output percentage progress even if not printing to a terminal."));

AddOption(BOOLEAN, "COLOR_ORIGINAL",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If false, versions are only colored according to the local setting."));

AddOption(BOOLEAN, "COLOR_LOCAL_MASK",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If false, COLOR_ORIGINAL=false has no effect on versions which are\n"
	"only locally masked (i.e. [m])."));

AddOption(BOOLEAN, "STYLE_VERSION_SORTED",
	"false", _(
	"Defines whether --versionsorted is on by default."));

AddOption(BOOLEAN, "STYLE_VERSION_LINES",
	"false", _(
	"Defines whether --versionlines is on by default."));

AddOption(BOOLEAN, "COLORED_SLOTS",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If false, the slotnames appended to versions are not colored."));

AddOption(BOOLEAN, "COLON_SLOTS",
	"false", _(
	"This variable is only used for delayed substitution.\n"
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
	"The default maximal levensthein distance for which a string is\n"
	"considered a match for the fuzzy match algorithm."));

AddOption(BOOLEAN, "UPDATE_VERBOSE",
	"false", _(
	"Whether eix-update -v is on by default (output cache method per ebuild)"));

AddOption(STRING, "CACHE_METHOD_PARSE",
	"#metadata-md5#metadata-flat#assign", _(
	"This string is appended to all cache methods using parse[*] or ebuild[*]."));

AddOption(STRING, "PORTDIR_CACHE_METHOD",
	PORTDIR_CACHE_METHOD, _(
	"Portage cache-backend that should be used for PORTDIR\n"
	"(metadata[:*]/sqlite/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])"));

AddOption(STRING, "OVERLAY_CACHE_METHOD",
	"parse|ebuild*", _(
	"Portage cache-backend that should be used for the overlays.\n"
	"(metadata[:*]/sqlite/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])"));

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

AddOption(STRING, "REPO_NAMES",
	"", _(
	"This is a list of pairs DIR-PATTERN OVERLAY_LABEL.\n"
	"When a new cachefile is created, the overlay matching DIR-PATTERN obtains\n"
	"the label OVERLAY_LABEL, independent of the content of profiles/repo_name\n"
	"or the label associated by KEEP_VIRTUALS.\n"
	"The last matching DIR_PATTERN takes precedence."));
#endif

#if (DEFAULT_PART == 2)

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
	ALWAYS_ACCEPT_KEYWORDS_DEFAULT, _(
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

AddOption(BOOLEAN, "PRINT_ALWAYS",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether all information lines are printed (even if empty)."));

AddOption(BOOLEAN, "COUNT_ONLY_PRINTED",
	"true", _(
	"If false, count independently of whether the matches are printed."));

AddOption(BOOLEAN, "PRINT_BUGS",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether a bug reference is printed in verbose format."));

AddOption(BOOLEAN, "DIFF_PRINT_INSTALLED",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether eix-diff will output installed versions."));

AddOption(STRING, "PRINT_SETNAMES",
	"%{?ALL_SETNAMES}all%{}setnames", _(
	"This variable is only used for delayed substitution.\n"
	"It is the command used to print the package set names."));

AddOption(BOOLEAN, "PRINT_SLOTS",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If false, no slot information is printed."));

AddOption(BOOLEAN, "EIX_PRINT_IUSE",
	"true", _(
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"If false, no IUSE data is printed for eix. See also VERSION_IUSE_*"));

AddOption(BOOLEAN, "DIFF_PRINT_IUSE",
	"false", _(
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"If false, no IUSE data is printed for eix-diff."));

AddOption(BOOLEAN, "UPDATE_PRINT_IUSE",
	"false", _(
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"It is unused by default."));

AddOption(BOOLEAN, "DROP_PRINT_IUSE",
	"false", _(
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"It is unused by default."));

AddOption(BOOLEAN, "PRINT_IUSE",
	"%{*PRINT_IUSE}", _(
	"This variable is only used for delayed substitution.\n"
	"If false, no IUSE data is printed."));

AddOption(BOOLEAN, "VERSION_IUSE_NORMAL",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs IUSE for each version."));

AddOption(BOOLEAN, "VERSION_IUSE_VERBOSE",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs IUSE for each version."));

AddOption(BOOLEAN, "PRINT_KEYWORDS",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If false, KEYWORDS are never output."));

AddOption(BOOLEAN, "VERSION_KEYWORDS_NORMAL",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs KEYWORDS."));

AddOption(BOOLEAN, "VERSION_KEYWORDS_VERBOSE",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs KEYWORDS."));

AddOption(BOOLEAN, "VERSION_DEPS_NORMAL",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs {,R,P}DEPEND."));

AddOption(BOOLEAN, "VERSION_DEPS_VERBOSE",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs {,R,P}DEPEND."));

AddOption(BOOLEAN, "USE_EFFECTIVE_KEYWORDS",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"Print effective keywords (modified by profile) instead of KEYWORDS."));

AddOption(BOOLEAN, "ALL_SETNAMES",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"It defines whether to include \"system\" in package sets output."));

AddOption(BOOLEAN, "NOBEST_COMPACT",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, compact format prints no version if none is installable."));

AddOption(BOOLEAN, "NOBEST_CHANGE",
	"true", _(
	"This variable is only used for delayed substitution.\n"
	"If true, compact format prints no versions if all installable vanished."));

AddOption(BOOLEAN, "DIFF_NOBEST",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix-diff prints no version if none is installable."));

AddOption(BOOLEAN, "DIFF_NOBEST_CHANGE",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"If true, eix-diff prints no versions if all installable vanished."));

AddOption(STRING, "TERM",
	"xterm", _(
	"The current terminal. Usually this is set by the environment variable."));

AddOption(STRING, "TERM_STATUSLINE",
	"xterm screen rxvt aterm konsole gnome Eterm eterm kterm interix", _(
	"If the beginning of TERM matches a word of this space-separated list,\n"
	"it is assumed that the terminal supports a status line."));

AddOption(STRING, "TERM_SOFTSTATUSLINE",
	"screen", _(
	"If the beginning of TERM matches a word of this space-separated list, and\n"
	"if a status line is active, also a soft status line will be output."));

AddOption(STRING, "EXIT_STATUSLINE",
	"", _(
	"If this is nonempty, it is used as the exit statusline.\n"
	"An optional leading space in this string is ignored."));

AddOption(STRING, "BG0",
	"none", _(
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 0. Use \"none\" for no change."));

AddOption(STRING, "BG1",
	"black", _(
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 1. Use \"none\" for no change."));

AddOption(STRING, "BG2",
	"none", _(
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 2. Use \"none\" for no change."));

AddOption(STRING, "BG3",
	"white", _(
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 3. Use \"none\" for no change."));

AddOption(STRING, "TERM_ALT1",
	"256 [aeEkx]term rxvt konsole gnome putty", _(
	"This is a list of regular expressions; if one of it matches TERM, then\n"
	"COLORSCHEME1 is used instead of COLORSCHEME0. The intention is that the\n"
	"specified terminals default to 256 colors."));

AddOption(STRING, "TERM_ALT2",
	"88 rxvt-unicode[^2]*$", _(
	"This is a list of regular expressions; if one of it matches TERM, then\n"
	"COLORSCHEME2 is used instead of COLORSCHEME0. The intention is that the\n"
	"specified terminals default to 88 colors."));

AddOption(STRING, "TERM_ALT3",
	"", _(
	"This is a list of regular expressions; if one of it matches TERM, then\n"
	"COLORSCHEME3 is used instead of COLORSCHEME0. The intention is that\n"
	"these terminals are exceptions made on user request."));

AddOption(BOOLEAN, "DARK",
	"true", _(
	"This variable is only used for delayed substitution in COLORSCHEME*.\n"
	"If true, the \"dark\" color schemes (for black background) are selected."));

AddOption(STRING, "COLORSCHEME0",
	"%{?DARK}0%{else}2%{}", _(
	"If TERM_ALT* does not match, this chooses the corresponding color of\n"
	"color specifications (starting from 0). The intention of this variable\n"
	"is to select the color scheme used for 8/16 color terminals."));

AddOption(STRING, "COLORSCHEME1",
	"%{?DARK}1%{else}3%{}", _(
	"If TERM_ALT1 matches, this chooses the corresponding color of\n"
	"color specifications (starting from 0). The intention of this variable\n"
	"is to select the color scheme used for 256 color terminals."));

AddOption(STRING, "COLORSCHEME2",
	"%{COLORSCHEME0}", _(
	"If TERM_ALT2 matches, this chooses the corresponding color of\n"
	"color specifications (starting from 0). The intention of this variable\n"
	"is to select the color scheme used for 88 color terminals."));

AddOption(STRING, "COLORSCHEME3",
	"%{COLORSCHEME0}", _(
	"If TERM_ALT3 matches, this chooses the corresponding color of\n"
	"color specifications (starting from 0). The intention of this variable\n"
	"is to select the color scheme used for user-specified terminals."));

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

AddOption(STRING, "MATCH_FIELD_CATEGORY_NAME",
	"/", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for category/name."));

AddOption(STRING, "MATCH_FIELD_LICENSE",
	"GPL|BSD|Art", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for license."));

AddOption(STRING, "MATCH_FIELD_DEPS",
	"[<>=!]", _(
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for license."));

AddOption(STRING, "DEFAULT_MATCH_FIELD",
	"%{\\MATCH_FIELD_DESCRIPTION} description "
	"%{\\MATCH_FIELD_SET} set "
	"%{\\MATCH_FIELD_HOMEPAGE} homepage "
	"%{\\MATCH_FIELD_CATEGORY_NAME} category/name "
	"%{\\MATCH_FIELD_LICENSE} license "
	"%{\\MATCH_FIELD_DEPS} deps "
	"name", _(
	"This is a list of strings of the form regexp[ ]match_field.\n"
	"If regexp matches the search pattern, use match_field as the default.\n"
	"A fallback match_field may be specified as the last entry in the list.\n"
	"Admissible values for match_field are: name, category, category/name,\n"
	"description, license, homepage, set, slot, installed-slot, use\n"
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
	"Defines whether /etc/portage/package.accept_keywords is tested with -t."));

AddOption(BOOLEAN, "TEST_MASK",
	"true", _(
	"Defines whether /etc/portage/package.mask is tested with -t."));

AddOption(BOOLEAN, "TEST_UNMASK",
	"true", _(
	"Defines whether /etc/portage/package.unmask is tested with -t."));

AddOption(BOOLEAN, "TEST_USE",
	"true", _(
	"Defines whether /etc/portage/package.use is tested with -t."));

AddOption(BOOLEAN, "TEST_ENV",
	"true", _(
	"Defines whether /etc/portage/package.env is tested with -t."));

AddOption(BOOLEAN, "TEST_LICENSE",
	"true", _(
	"Defines whether /etc/portage/package.license is tested with -t."));

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

AddOption(STRING, "ACCEPT_KEYWORDS_AS_ARCH",
	"full", _(
	"If full or true modify ARCH by ACCEPT_KEYWORDS.\n"
	"This determines which keywords are considered as ARCH or OTHERARCH.\n"
	"The value full also influences the original ARCH keywording."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE",
	"some", _(
	"Applies if /etc/portage/package.accept_keywords lists the same keyword\n"
	"twice for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_LINE",
	"some", _(
	"Applies if /etc/portage/package.accept_keywords has two lines for\n"
	"identical target."));

AddOption(STRING, "REDUNDANT_IF_MIXED",
	"false", _(
	"Applies if /etc/portage/package.accept_keywords lists two different\n"
	"keywords, e.g. ~ARCH and -*, for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_WEAKER",
	"all-installed", _(
	"Applies if /etc/portage/package.accept_keywords lists a keywords which can\n"
	"be replaced by a weaker keyword, e.g. -* or ~OTHERARCH or OTHERARCH\n"
	"in place of ~ARCH, or ~OTHERARCH in place of OTHERARCH,\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_STRANGE",
	"some", _(
	"Applies if /etc/portage/package.accept_keywords lists a strange keyword\n"
	"e.g. UNKNOWNARCH (unknown to the .ebuild and ARCH) or -OTHERARCH,\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_NO_CHANGE",
	"all-installed", _(
	"Applies if /etc/portage/package.accept_keywords provides keywords which do\n"
	"not change the availability keywords status for the versions in question."));

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
	"false", _(
	"Applies if /etc/portage/package.use matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_ENV",
	"false", _(
	"Applies if /etc/portage/package.env matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_LICENSE",
	"some", _(
	"Applies if /etc/portage/package.license matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_CFLAGS",
	"false", _(
	"Applies if /etc/portage/package.cflags matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_IN_KEYWORDS",
	"-some", _(
	"Applies if /etc/portage/package.accept_keywords contains a matching entry."));

AddOption(STRING, "REDUNDANT_IF_IN_MASK",
	"-some", _(
	"Applies if /etc/portage/package.mask matches."));

AddOption(STRING, "REDUNDANT_IF_IN_UNMASK",
	"-some", _(
	"Applies if /etc/portage/package.unmask matches."));

AddOption(STRING, "REDUNDANT_IF_IN_USE",
	"false", _(
	"Applies if /etc/portage/package.use matches."));

AddOption(STRING, "REDUNDANT_IF_IN_ENV",
	"false", _(
	"Applies if /etc/portage/package.env matches."));

AddOption(STRING, "REDUNDANT_IF_IN_LICENSE",
	"-some", _(
	"Applies if /etc/portage/package.license matches."));

AddOption(STRING, "REDUNDANT_IF_IN_CFLAGS",
	"false", _(
	"Applies if /etc/portage/package.cflags matches."));

AddOption(STRING, "EIXCFGDIR",
	"%{PORTAGE_CONFIGROOT}/etc/portage", _(
	"This variable is only used for delayed substitution.\n"
	"It is the directory where eix searches for its package.*.*/sets.eix files."));

AddOption(BOOLEAN, "SLOT_UPGRADE_FORBID",
	"%{\\EIXCFGDIR}/package.slot_upgrade_forbid", _(
	"If UPGRADE_TO_HIGHEST_SLOT=true, then packages listed in these files/dirs\n"
	"are treated as if UPGRADE_TO_HIGHEST_SLOT=false."));
#endif

#if (DEFAULT_PART == 3)

AddOption(BOOLEAN, "SLOT_UPGRADE_ALLOW",
	"%{\\EIXCFGDIR}/package.slot_upgrade_allow", _(
	"If UPGRADE_TO_HIGHEST_SLOT=false, then packages listed in these files/dirs\n"
	"are treated as if UPGRADE_TO_HIGHEST_SLOT=true."));

AddOption(STRING, "KEYWORDS_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.accept_keywords.nonexistent "
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

AddOption(STRING, "ENV_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.env.nonexistent", _(
	"Entries listed in these files/dire are excluded for -t TEST_ENV."));

AddOption(STRING, "LICENSE_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.license.nonexistent", _(
	"Entries listed in these files/dirs are excluded for -t TEST_LICENSE."));

AddOption(STRING, "CFLAGS_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.cflags.nonexistent", _(
	"Entries listed in these files/dirs are excluded for -t TEST_CFLAGS."));

AddOption(STRING, "INSTALLED_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.installed.nonexistent", _(
	"Packages listed in these files/dirs are excluded for -t TEST_REMOVED."));

AddOption(STRING, "ADD_PACKAGE_NOWARN",
	"", _(
	"This variable is only used for delayed substitution in PACKAGE_NOWARN.\n"
	"It is meant to be a local addition to PACKAGE_NOWARN."));

AddOption(BOOLEAN, "OBSOLETE_NOWARN",
	"false", _(
	"This variable is only used for delayed substitution in PACKAGE_NOWARN.\n"
	"If it is set then the files from OBSOLETE_PACKAGE_NOWARN are used."));

AddOption(STRING, "OBSOLETE_PACKAGE_NOWARN",
	"%{\\EIXCFGDIR}/package.accept_keywords.nowarn "
	"%{\\EIXCFGDIR}/package.keywords.nowarn "
	"%{\\EIXCFGDIR}/package.mask.nowarn "
	"%{\\EIXCFGDIR}/package.unmask.nowarn "
	"%{\\EIXCFGDIR}/package.use.nowarn "
	"%{\\EIXCFGDIR}/package.env.nowarn "
	"%{\\EIXCFGDIR}/package.license.nowarn "
	"%{\\EIXCFGDIR}/package.cflags.nowarn "
	"%{\\EIXCFGDIR}/package.installed.nowarn", _(
	"This variable is used for delayed substitution in PACKAGE_NOWARN if the\n"
	"variable OBSOLETE_NOWARN is set."));

AddOption(STRING, "PACKAGE_NOWARN",
	"%{\\EIXCFGDIR}/package.nowarn "
	"%{?OBSOLETE_NOWARN}"
		"%{OBSOLETE_PACKAGE_NOWARN} "
	"%{}"
	"%{ADD_PACKAGE_NOWARN}", _(
	"This file/directory contains exceptions for -T tests."));

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

AddOption(STRING, "COLOR_RESET",
	";%{BG0}|;%{BG1}|;%{BG2}|;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It is the color used for default colors."));

AddOption(STRING, "COLOR_NAME",
	",1;%{BG0}|253,1;%{BG1}|,1;%{BG2}|232;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the name of packages."));

AddOption(STRING, "COLOR_WORLD",
	"green,1;%{BG0}|47,1;%{BG1}|green,1;%{BG2}|22,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the name of world packages."));

AddOption(STRING, "COLOR_WORLD_SETS",
	"yellow,1;%{BG0}|214,1;%{BG1}|blue,1;%{BG2}|33,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the name of world sets packages."));

AddOption(STRING, "COLOR_CATEGORY",
	"%{COLOR_NORMAL}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of packages."));

AddOption(STRING, "COLOR_CATEGORY_SYSTEM",
	"yellow;%{BG0}|154,1;%{BG1}|blue;%{BG2}|57,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of system packages."));

AddOption(STRING, "COLOR_CATEGORY_WORLD",
	"%{COLOR_WORLD}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of world packages."));

AddOption(STRING, "COLOR_CATEGORY_WORLD_SETS",
	"%{COLOR_WORLD_SETS}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of world sets packages."));

AddOption(STRING, "COLOR_UPGRADE_TEXT",
	"cyan,1;%{BG0}|87,1;%{BG1}|blue,1;%{BG2}|21,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation texts."));

AddOption(STRING, "COLOR_DOWNGRADE_TEXT",
	"blue,1;%{BG0}|135,1;%{BG1}|purple,1;%{BG2}|89,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation texts."));

AddOption(STRING, "COLOR_UPGRADE",
	"%{COLOR_UPGRADE_TEXT;inverse}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation tags."));

AddOption(STRING, "COLOR_DOWNGRADE",
	"%{COLOR_DOWNGRADE_TEXT;inverse}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation tags."));

AddOption(STRING, "DIFF_COLOR_UNINST_STABILIZE",
	"%{COLOR_STABLE,1}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for uninstalled packages\n"
	"which have gained a stable version."));

AddOption(STRING, "DIFF_COLOR_INST_STABILIZE",
	"%{DIFF_COLOR_UNINST_STABILIZE;inverse}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for installed packages\n"
	"which have gained a stable version."));

AddOption(STRING, "DIFF_COLOR_BETTER",
	"%{DIFF_COLOR_CHANGED}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"better version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_WORSE",
	"%{DIFF_COLOR_DELETE}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"worse version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_NEW_TAG",
	"%{DIFF_COLOR_NEW}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_NEW",
	"%{COLOR_STABLE,1}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_DELETE",
	"red,1;%{BG0}|197,1;%{BG1}|red,1;%{BG2}|197,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"deleted package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_CHANGED",
	"yellow;%{BG0}|226;%{BG1}|blue;%{BG2}|24;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"changed package\" separators (eix-diff)."));

AddOption(STRING, "COLOR_INST_TAG",
	"%{COLOR_STABLE,1;inverse}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging installed packages."));

AddOption(STRING, "COLOR_UNINST_TAG",
	"%{COLOR_STABLE}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging uninstalled packages."));

AddOption(STRING, "COLOR_DATE",
	"purple,1;%{BG0}|166;%{BG1}|purple,1;%{BG2}|130,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the date."));

AddOption(STRING, "COLOR_DEPEND",
	"none|248;%{BG1}|none|241,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color of the DEPEND output."));

AddOption(STRING, "COLOR_DEPEND_END",
	"none|;%{BG1}|none|;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the end of color of the DEPEND output."));

AddOption(STRING, "COLOR_NORMAL",
	";%{BG0}|252;%{BG1}|,1;%{BG2}|237,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing normal texts."));

AddOption(STRING, "COLOR_NORMAL_END",
	"none|;%{BG1}|none|;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing end of normal texts."));

AddOption(STRING, "COLOR_SET_USE",
	"red,1;%{BG0}|125,1;%{BG1}|red,1;%{BG2}|125,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the set useflags."));

AddOption(STRING, "COLOR_UNSET_USE",
	"blue,1;%{BG0}|33;%{BG1}|blue,1;%{BG2}|17;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the unset useflags."));

AddOption(STRING, "COLOR_VERSION_IUSE",
	",1;%{BG0}|168;%{BG1}|,1;%{BG2}|168,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing IUSE for available versions."));

AddOption(STRING, "COLOR_COLL_IUSE",
	",1;%{BG0}|38;%{BG1}|,1;%{BG2}|61,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing collected IUSE for packages."));

AddOption(STRING, "COLOR_USE_EXPAND_START",
	";%{BG0}|115;%{BG1}|;%{BG2}|95,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing USE_EXPAND variables."));

AddOption(STRING, "COLOR_USE_EXPAND_END",
	"none|;%{BG1}|none|;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing end of USE_EXPAND variables."));

AddOption(STRING, "COLOR_USE_COLL",
	"yellow,1;%{BG0}|252;%{BG1}|blue;%{BG2}|237,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the braces around collected USE."));

AddOption(STRING, "COLOR_INST_VERSION",
	"black,1;blue|33,1;%{BG1}|black;green|30,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the version of installed packages."));

AddOption(STRING, "COLOR_TITLE",
	"green;%{BG0}|34;%{BG1}|;%{BG2}|240,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for packages."));

AddOption(STRING, "COLOR_INST_TITLE",
	"cyan;%{BG0}|67;%{BG1}|cyan,1;%{BG2}|67,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for installed versions."));

AddOption(STRING, "COLOR_AVAILABLE_TITLE",
	"purple;%{BG0}|70;%{BG1}|purple;%{BG2}|65,1;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for available versions."));

AddOption(STRING, "COLOR_MARKED_VERSION",
	"%{COLOR_MARKED_NAME}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked version of a packages."));

AddOption(STRING, "COLOR_PACKAGESETS",
	"%{COLOR_WORLD_SETS}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the package sets."));

AddOption(STRING, "COLOR_MARKED_NAME",
	"red,1;%{BG0};%{MARK_VERSIONS}|207,1;%{BG1};%{MARK_VERSIONS}|red,1;%{BG2};%{MARK_VERSIONS}|164,1;%{BG3};%{MARK_VERSIONS}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked package name."));

AddOption(STRING, "COLOR_OVERLAYKEY",
	"cyan;%{BG0}|87;%{BG1}|cyan,1;%{BG2}|26,1;%{BG3}", _(
	"Color for the overlaykey in version listings."));

AddOption(STRING, "COLOR_VIRTUALKEY",
	"purple;%{BG0}|170;%{BG1}|purple,1;%{BG2}|92,1;%{BG3}", _(
	"Color for the overlaykey for virtual overlays in version listings."));

AddOption(STRING, "COLOR_KEYEND",
	"%{COLOR_RESET}", _(
	"Color after printing an overlay key."));

AddOption(STRING, "COLOR_OVERLAYNAME",
	"%{COLOR_NORMAL}", _(
	"Color for printing an overlay name."));

AddOption(STRING, "COLOR_OVERLAYNAMEEND",
	"%{COLOR_RESET}", _(
	"Color after printing an overlay name."));

AddOption(STRING, "COLOR_NUMBERTEXT",
	"%{COLOR_NORMAL}", _(
	"Color for printing the number of packages."));

AddOption(STRING, "COLOR_SLOTS",
	"red,1;%{BG0}|166,1;%{BG1}|red,1;%{BG2}|166,1;%{BG3}", _(
	"Color for slots. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_BINARY",
	"blue,1;%{BG0}|39,1;%{BG1}|blue,1;%{BG2}|39,1;%{BG3}", _(
	"Color for versions with *.tbz2. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_RESTRICT",
	"red;%{BG0}|99;%{BG1}|red;%{BG2}|53;%{BG3}", _(
	"Color for the restriction tags. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_PROPERTIES",
	"cyan;%{BG0}|143;%{BG1}|cyan,1;%{BG2}|57;%{BG3}", _(
	"Color for the properties tags. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_KEYWORDS",
	"cyan;%{BG0}|73;%{BG1}|cyan,1;%{BG2}|26;%{BG3}", _(
	"Color for keywords. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_KEYWORDSS",
	"%{COLOR_KEYWORDS}", _(
	"Color for keywords*. This is only used for delayed substitution."));

AddOption(STRING, "MARK_INSTALLED",
	"inverse", _(
	"This variable is only used for delayed substitution.\n"
	"It defines how installed versions are marked."));

AddOption(STRING, "MARK_UPGRADE",
	"bold", _(
	"This variable is only used for delayed substitution.\n"
	"It defines how upgrade candidate versions are marked."));

AddOption(STRING, "MARK_VERSIONS",
	"underline", _(
	"This variable is only used for delayed substitution.\n"
	"It defines how the package versions passed with --pipe are marked."));

AddOption(BOOLEAN, "NOCOLOR",
	"false", _(
	"This variable is only used for delayed substitution.\n"
	"It is the default for NO{COLORS,STATUSLINE,PERCENTAGE}."));

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
	"(%{COLOR_UPGRADE})%{CHAR_UPGRADE}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for upgrade recommendations."));

AddOption(STRING, "TAG_DOWNGRADE",
	"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations."));

AddOption(STRING, "TAG_INSTALLED",
	"(%{COLOR_INST_TAG})%{CHAR_INSTALLED}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for installed packages."));

AddOption(STRING, "TAG_UNINSTALLED",
	"(%{COLOR_UNINST_TAG})%{CHAR_UNINSTALLED}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for uninstalled packages."));

AddOption(STRING, "TAG_STABILIZE",
	"{installed}"
		"(%{DIFF_COLOR_INST_STABILIZE})%{DIFF_CHAR_INST_STABILIZE}"
	"{else}"
		"(%{DIFF_COLOR_UNINST_STABILIZE})%{DIFF_CHAR_UNINST_STABILIZE}"
	"{}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag for packages which have gained a stable version."));

AddOption(STRING, "TAG_NEW",
	"(%{DIFF_COLOR_NEW_TAG})%{DIFF_CHAR_NEW}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for new packages (eix-diff)."));

AddOption(STRING, "TAG_BETTER",
	"(%{DIFF_COLOR_BETTER})%{DIFF_CHAR_BETTER}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"better version\" (eix-diff)."));

AddOption(STRING, "TAG_WORSE",
	"(%{DIFF_COLOR_WORSE})%{DIFF_CHAR_WORSE}(%{COLOR_RESET})", _(
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

AddOption(STRING, "FORMAT_NOBEST",
	"%{FORMAT_COLOR_MASKED}"
	"--"
	"(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines what to print if no version number is printed."));

AddOption(STRING, "FORMAT_NOBEST_CHANGE",
	"%{FORMAT_COLOR_MASKED}"
	"??"
	"(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines what to print after \"->\" if there is no installable."));

AddOption(STRING, "TAG_BINARY",
	"\\{tbz2}", _(
	"Tag for versions with *.tbz2. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_FETCH",
	"f", _(
	"Tag for RESTRICT=fetch. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_MIRROR",
	"m", _(
	"Tag for RESTRICT=mirror. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_PRIMARYURI",
	"p", _(
	"Tag for RESTRICT=primaryuri. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_BINCHECKS",
	"b", _(
	"Tag for RESTRICT=binchecks. This is only used for delayed substitution."));
#endif

#if (DEFAULT_PART == 4)

AddOption(STRING, "TAG_RESTRICT_STRIP",
	"s", _(
	"Tag for RESTRICT=strip. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_TEST",
	"t", _(
	"Tag for RESTRICT=test. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_USERPRIV",
	"u", _(
	"Tag for RESTRICT=userpriv. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_INSTALLSOURCES",
	"i", _(
	"Tag for RESTRICT=installsources. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_BINDIST",
	"d", _(
	"Tag for RESTRICT=bindist. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_PARALLEL",
	"P", _(
	"Tag for RESTRICT=parallel. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_INTERACTIVE",
	"i", _(
	"Tag for PROPERTIES=interactive. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_LIVE",
	"l", _(
	"Tag for PROPERTIES=live. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_VIRTUAL",
	"v", _(
	"Tag for PROPERTIES=virtual. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_SET",
	"s", _(
	"Tag for PROPERTIES=set. This is only used for delayed substitution."));

AddOption(STRING, "TAG_FOR_PROFILE",
	"[P]", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"profile masked\" versions."));

AddOption(STRING, "TAG_FOR_MASKED",
	"[M]", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"package.masked\" versions."));

AddOption(STRING, "TAG_FOR_EX_PROFILE",
	"{P}", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally profile masked but unmasked\" versions."));

AddOption(STRING, "TAG_FOR_EX_MASKED",
	"{M}", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally package.masked but unmasked\" versions."));

AddOption(STRING, "TAG_FOR_LOCALLY_MASKED",
	"[m]", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"only locally masked\" versions."));

AddOption(STRING, "TAG_FOR_STABLE",
	"", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"stable\" versions."));

AddOption(STRING, "TAG_FOR_UNSTABLE",
	"~", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"unstable\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_ASTERISK",
	"-*", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"-*\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_UNSTABLE",
	"-~*", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"-~*\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_KEYWORD",
	"-", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"-ARCH\" versions."));

AddOption(STRING, "TAG_FOR_ALIEN_STABLE",
	"*", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"ALIENARCH\" versions."));

AddOption(STRING, "TAG_FOR_ALIEN_UNSTABLE",
	"~*", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"~ALIENARCH\" versions."));

AddOption(STRING, "TAG_FOR_MISSING_KEYWORD",
	"**", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"no keyword\" versions."));

AddOption(STRING, "TAG_FOR_EX_UNSTABLE",
	"(~)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally unstable but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_ASTERISK",
	"(-*)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally -* but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_UNSTABLE",
	"(-~*)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally -~* but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_KEYWORD",
	"(-)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally -ARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_ALIEN_STABLE",
	"(*)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally ALIENARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_ALIEN_UNSTABLE",
	"(~*)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally ~ALIENARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MISSING_KEYWORD",
	"(**)", _(
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally no keyword but now stable\" versions."));

AddOption(STRING, "VERSION_NEWLINE",
	"\\n", _(
	"This variable is used by delayed substitution in version formatters.\n"
	"It prints a newline at the end of a version."));

AddOption(STRING, "NAMEVERSION",
	"<category>/<name>-<version>"
	"%{VERSION_NEWLINE}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <bestslotupgradeversions:NAMEVERSION>\n"
	"or <installedversions:NAMEVERION> or <availableversions:NAMEVERSION>."));

AddOption(STRING, "EQNAMEVERSION",
	"=<category>/<name>-<version>"
	"%{VERSION_NEWLINE}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <bestslotupgradeversions:EQNAMEVERSION>\n"
	"or <installedversions:EQNAMEVERION> or <availableversions:EQNAMEVERSION>."));

AddOption(STRING, "ANAMESLOT",
	"{slotlast}<category>/<name>{slots}:<slot>{}"
	"%{VERSION_NEWLINE}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage as <availableversion:ANAMESLOT:ANAMESLOT>."));

AddOption(STRING, "ANAMEASLOT",
	"{slotlast}<category>/<name>:<slot>{!last}\\n{}{}"
	"%{VERSION_NEWLINE}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <availableversion:ANAMEASLOT:ANAMEASLOT>."));

AddOption(STRING, "NAMESLOT",
	"<category>/<name>{slots}:<slot>{}%{VERSION_NEWLINE}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <installedversions:NAMESLOT>."));

AddOption(STRING, "NAMEASLOT",
	"<category>/<name>:<slot>"
	"%{VERSION_NEWLINE}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <installedversions:NAMEASLOT>."));

AddOption(STRING, "DATESORT_DATE",
	"%s	%x %X", _(
	"strftime() format for printing the installation date in DATESORT"));

AddOption(STRING, "DATESORT",
	"<date:DATESORT_DATE>\\t%{NAMESLOT}", _(
	"This variable is used as a version formatter.\n"
	"It is an example for usage as <installedversions:DATESORT>. Typical usage:\n"
	"eix -'*I' --format '<installedversions:DATESORT>' | sort | cut -f2-3"));

AddOption(STRING, "FORMAT_DEPEND",
	"(%{COLOR_DEPEND})<depend*>(%{COLOR_DEPEND_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the DEPEND output."));

AddOption(STRING, "FORMAT_RDEPEND",
	"(%{COLOR_DEPEND})<rdepend*>(%{COLOR_DEPEND_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the RDEPEND output."));

AddOption(STRING, "FORMAT_PDEPEND",
	"(%{COLOR_DEPEND})<pdepend*>(%{COLOR_DEPEND_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PDEPEND output."));

AddOption(STRING, "FORMAT_HDEPEND",
	"(%{COLOR_DEPEND})<hdepend*>(%{COLOR_DEPEND_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the HDEPEND output."));

AddOption(STRING, "FORMAT_DEPEND_VERBOSE",
	"%{FORMAT_DEPEND}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the DEPEND output with --verbose."));

AddOption(STRING, "FORMAT_RDEPEND_VERBOSE",
	"%{FORMAT_RDEPEND}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the RDEPEND output with --verbose."));

AddOption(STRING, "FORMAT_PDEPEND_VERBOSE",
	"%{FORMAT_PDEPEND}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PDEPEND output with --verbose."));

AddOption(STRING, "FORMAT_HDEPEND_VERBOSE",
	"%{FORMAT_HDEPEND}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the HDEPEND output with --verbose."));

AddOption(STRING, "FORMAT_VERSION_IUSE",
	"(%{COLOR_VERSION_IUSE})%{?EIX_USE_EXPAND}<use*>%{else}<use>%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It is used for colored <use> in available versions; color is not reset."));

AddOption(STRING, "FORMAT_COLLIUSE",
	"(%{COLOR_COLL_IUSE})%{?EIX_USE_EXPAND}<colliuse*>%{else}<colliuse>%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It is used for colored <colliuse>; color is not reset."));

AddOption(STRING, "FORMAT_KEYWORDS",
	"(%{COLOR_KEYWORDS})<versionkeywords>(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It is used for printing colored <keywords>; color is not reset."));

AddOption(STRING, "FORMAT_KEYWORDSS",
	"(%{COLOR_KEYWORDSS})<versionkeywords*>(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It is used for printing colored <keywords*>; color is not reset."));

AddOption(STRING, "FORMAT_KEYWORDS_EQUAL",
	"{versionkeywords} (%{COLOR_KEYWORDSS})$\\{KEYWORDS\\}(%{COLOR_RESET}){}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines what to output for KEYWORDS* if they equal KEYWORDS."));

AddOption(STRING, "FORMAT_MASK_TAG",
	"{!*mask}"
	"{washardmasked}"
		"%{?COLOR_ORIGINAL}{!$color}{*color}%{FORMAT_COLOR_MASKED}{}%{}"
		"{isprofilemasked}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*mask=\"%{TAG_FOR_PROFILE}\"}"
		"{else}{ismasked}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*mask=\"%{TAG_FOR_MASKED}\"}"
		"{else}"
			"{wasprofilemasked}{*mask=\"%{TAG_FOR_EX_PROFILE}\"}"
			"{else}{*mask=\"%{TAG_FOR_EX_MASKED}\"}{}"
		"{}{}"
	"{else}"
		"{ishardmasked}"
			"%{?COLOR_LOCAL_MASK}{!$color}{*color}%{FORMAT_COLOR_MASKED}{}%{}"
			"{*mask=\"%{TAG_FOR_LOCALLY_MASKED}\"}"
		"{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It sets the runtime variable $mask to the masking tag, and unless the\n"
	"runtime variable $color is set, it outputs the color and sets $color."));

AddOption(STRING, "FORMAT_STABILITY_TAG",
	"{isstable}"
		"%{!COLOR_ORIGINAL}"
			"{!$color}{*color}%{FORMAT_COLOR_STABLE}{}"
		"%{}"
		"{wasstable}"
			"{!$color}{*color}%{FORMAT_COLOR_STABLE}{}"
			"{*stable=\"%{TAG_FOR_STABLE}\"}"
		"{else}{wasunstable}"
			"{!$color}{*color}%{FORMAT_COLOR_UNSTABLE}{}"
			"{*stable=\"%{TAG_FOR_EX_UNSTABLE}\"}"
		"{else}{wasmissingkeyword}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*stable=\"%{TAG_FOR_EX_MISSING_KEYWORD}\"}"
		"{else}{wasalienstable}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*stable=\"%{TAG_FOR_EX_ALIEN_STABLE}\"}"
		"{else}{wasalienunstable}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*stable=\"%{TAG_FOR_EX_ALIEN_UNSTABLE}\"}"
		"{else}{wasminuskeyword}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*stable=\"%{TAG_FOR_EX_MINUS_KEYWORD}\"}"
		"{else}{wasminusunstable}"
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*stable=\"%{TAG_FOR_EX_MINUS_UNSTABLE}\"}"
		"{else}"  // {wasminusasterisk}
			"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
			"{*stable=\"%{TAG_FOR_EX_MINUS_ASTERISK}\"}"
		"{}{}{}{}{}{}{}"
	"{else}{isunstable}"
		"{!$color}{*color}%{FORMAT_COLOR_UNSTABLE}{}"
		"{*stable=\"%{TAG_FOR_UNSTABLE}\"}"
	"{else}{ismissingkeyword}"
		"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
		"{*stable=\"%{TAG_FOR_MISSING_KEYWORD}\"}"
	"{else}{isalienstable}"
		"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
		"{*stable=\"%{TAG_FOR_ALIEN_STABLE}\"}"
	"{else}{isalienunstable}"
		"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
		"{*stable=\"%{TAG_FOR_ALIEN_UNSTABLE}\"}"
	"{else}{isminuskeyword}"
		"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
		"{*stable=\"%{TAG_FOR_MINUS_KEYWORD}\"}"
	"{else}{isminusunstable}"
		"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
		"{*stable=\"%{TAG_FOR_MINUS_UNSTABLE}\"}"
	"{else}"  // {isminusasterisk}
		"{!$color}{*color}%{FORMAT_COLOR_MASKED}{}"
		"{*stable=\"%{TAG_FOR_MINUS_ASTERISK}\"}"
	"{}{}{}{}{}{}{}", _(
	"This variable is only used for delayed substitution.\n"
	"It sets the runtime variable $stable to the stability tag, and unless the\n"
	"runtime variable $color is set, it outputs the color and sets $color."));

AddOption(STRING, "FORMAT_STABILITY",
	"{!*color}%{FORMAT_MASK_TAG}%{FORMAT_STABILITY_TAG}<$mask><$stable>", _(
	"This variable is only used for delayed substitution.\n"
	"It outputs the stability tag, changing the color appropriately.\n"
	"It sets the runtime variable $color depending on whether color was changed."));

AddOption(STRING, "FORMAT_PROPERTIESSEPARATOR",
	"<$sep>{!*sep}{*color}(%{COLOR_PROPERTIES})*", _(
	"This variable is only used for delayed substitution.\n"
	"It sets the initial string and $color for PROPERTIES output."));

AddOption(STRING, "FORMAT_RESTRICTSEPARATOR",
	"<$sep>{!*sep}{*color}(%{COLOR_RESTRICT})^", _(
	"This variable is only used for delayed substitution.\n"
	"It sets the initial string and $color for RESTRICT output."));

AddOption(STRING, "FORMAT_PROPERTIES",
	"{properties}%{FORMAT_PROPERTIESSEPARATOR}"
		"{propertiesinteractive}%{TAG_PROPERTIES_INTERACTIVE}{}"
		"{propertieslive}%{TAG_PROPERTIES_LIVE}{}"
		"{propertiesset}%{TAG_PROPERTIES_SET}{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It outputs the PROPERTIES tag, changing the color appropriately.\n"
	"It sets the runtime variable $color if color was changed."));

AddOption(STRING, "FORMAT_RESTRICT",
	"{restrict}%{FORMAT_RESTRICTSEPARATOR}"
		"{restrictfetch}%{TAG_RESTRICT_FETCH}{}"
		"{restrictmirror}%{TAG_RESTRICT_MIRROR}{}"
		"{restrictprimaryuri}%{TAG_RESTRICT_PRIMARYURI}{}"
		"{restrictbinchecks}%{TAG_RESTRICT_BINCHECKS}{}"
		"{restrictstrip}%{TAG_RESTRICT_STRIP}{}"
		"{restricttest}%{TAG_RESTRICT_TEST}{}"
		"{restrictuserpriv}%{TAG_RESTRICT_USERPRIV}{}"
		"{restrictinstallsources}%{TAG_RESTRICT_INSTALLSOURCES}{}"
		"{restrictbindist}%{TAG_RESTRICT_BINDIST}{}"
		"{restrictparallel}%{TAG_RESTRICT_PARALLEL}{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It outputs the RESTRICT tag, changing the color appropriately.\n"
	"It sets the runtime variable $color if color was changed."));

AddOption(STRING, "FORMAT_PROPRESTRICT",
	"%{!NO_RESTRICTIONS}"
		"{!*color}%{FORMAT_PROPERTIES}%{FORMAT_RESTRICT}"
		"{$color}(%{COLOR_RESET}){}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PROPERTIES and RESTRICT of a version\n"
	"and resets the color."));

AddOption(STRING, "FORMAT_BINARY",
	"%{!NO_BINARY}"
		"{isbinary}"
			"<$sep>{!*sep}(%{COLOR_BINARY})%{TAG_BINARY}(%{COLOR_RESET})"
		"{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PROPERTIES and RESTRICT of a version\n"
	"and resets the color."));

AddOption(STRING, "FORMAT_SLOT",
	"(%{COLOR_SLOTS})\\(<slot>\\)(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the slot format printed for slotsorted versions."));

AddOption(STRING, "VERSION",
	"<version>{!last} {}", _(
	"This variable is used for <markedversions:VERSION>.\n"
	"It just gives the plain version name without anything else.\n"
	"You can use it similarly for e.g. <availableversions:VERSION>."));

AddOption(STRING, "FORMAT_PVERSION",
	"{color}"
		"{installedversion}"
			"{*color}(none;%{MARK_INSTALLED})"
		"{else}"
			"{isbestupgradeslot}{*color}(%{MARK_UPGRADE}){}"
		"{}"
		"{markedversion}{*color}(%{MARK_VERSIONS}){}"
	"{}"
	"<version>", _(
	"This variable is only used for delayed substitution.\n"
	"It outputs an available version with various marks and sets the runtime\n"
	"variable $color if a mark was printed.\n"
	"It should be follows by FORMAT_VERSION_END or FORMAT_VERSIONS_END."));

AddOption(STRING, "FORMAT_VERSION_END",
	"{$color}(%{COLOR_RESET}){}", _(
	"This variable is only used for delayed substitution.\n"
	"It resets all colors/markers if the runtime variable $color was set."));

AddOption(STRING, "FORMAT_VERSIONO_END",
	"%{?PRINT_SLOTS}"
		"{issubslot}"
			"%{?COLORED_SLOTS}"
				"{$color}(%{COLOR_RESET}){}"
				"{*color}"
				"%{?COLON_SLOTS}:(%{COLOR_SLOTS})<fullslot>"
				"%{else}<$sep>{!*sep}(%{COLOR_SLOTS})\\(<fullslot>\\)%{}"
			"%{else}"
				"%{?COLON_SLOTS}:<fullslot>"
				"%{else}<$sep>{!*sep}\\(<fullslot>\\)%{}"
			"%{}"
		"{}"
	"%{}"
	"%{FORMAT_VERSION_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It outputs an optional subslot, caring about the runtime variable $color,\n"
	"and then invokes FORMAT_VERSION_END."));

AddOption(STRING, "FORMAT_VERSIONS_END",
	"%{?PRINT_SLOTS}"
		"{isfullslot}"
			"%{?COLORED_SLOTS}"
				"{$color}(%{COLOR_RESET}){}"
				"{*color}"
				"%{?COLON_SLOTS}:(%{COLOR_SLOTS})<fullslot>"
				"%{else}<$sep>{!*sep}(%{COLOR_SLOTS})\\(<fullslot>\\)%{}"
			"%{else}"
				"%{?COLON_SLOTS}:<fullslot>"
				"%{else}<$sep>{!*sep}\\(<fullslot>\\)%{}"
			"%{}"
		"{}"
	"%{}"
	"%{FORMAT_VERSION_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It outputs an optional slot, caring about the runtime variable $color,\n"
	"and then invokes FORMAT_VERSION_END."));

AddOption(STRING, "PVERSION",
	"%{FORMAT_PVERSION}%{FORMAT_VERSION_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version without slot."));

AddOption(STRING, "PVERSIONO",
	"%{FORMAT_PVERSION}%{FORMAT_VERSIONO_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing a plain version with its slot."));

AddOption(STRING, "PVERSIONS",
	"%{FORMAT_PVERSION}%{FORMAT_VERSIONS_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing a plain version with its slot."));

AddOption(STRING, "AVERSION",
	"%{FORMAT_STABILITY}%{FORMAT_PVERSION}%{FORMAT_VERSION_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version without slot."));

AddOption(STRING, "AVERSIONO",
	"%{FORMAT_STABILITY}%{FORMAT_PVERSION}%{FORMAT_VERSIONO_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing an available version\n"
	"with its optional subslot."));

AddOption(STRING, "AVERSIONS",
	"%{FORMAT_STABILITY}%{FORMAT_PVERSION}%{FORMAT_VERSIONS_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing an available version with its slot."));

AddOption(STRING, "IVERSIONS",
	"(%{COLOR_INST_VERSION}){*color}<version>%{FORMAT_VERSIONS_END}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing an installed version with its slot."));

AddOption(STRING, "OVERLAYVER",
	"{overlayver}<$sep>{!*sep}<overlayver>{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing the overlay in versions."));

AddOption(STRING, "PVERSIONS_VERBOSE",
	"%{PVERSIONS}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with most data and slot."));

AddOption(STRING, "AVERSIONS_VERBOSE",
	"%{AVERSIONS}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with most data and slot."));

AddOption(STRING, "IVERSIONS_VERBOSE",
	"%{IVERSIONS}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an installed version with most data and slot."));

AddOption(STRING, "PVERSIONO_VERBOSE",
	"%{PVERSIONO}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with most data, optional subslot."));

AddOption(STRING, "AVERSIONO_VERBOSE",
	"%{AVERSIONO}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with most data,\n"
	"optional subslot."));

AddOption(STRING, "PVERSION_VERBOSE",
	"%{PVERSION}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with most data, no slot."));

AddOption(STRING, "AVERSION_VERBOSE",
	"%{AVERSION}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with most data, no slot."));

AddOption(STRING, "PVERSIONS_COMPACT",
	"%{PVERSIONS}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with important data and slot."));

AddOption(STRING, "AVERSIONS_COMPACT",
	"%{AVERSIONS}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with important data and slot."));

AddOption(STRING, "IVERSIONS_COMPACT",
	"%{IVERSIONS}%{FORMAT_BINARY}%{OVERLAYVER}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an installed version with important data and slot."));

AddOption(STRING, "FORMAT_BEFORE_KEYWORDS",
	"\\t(%{COLOR_NORMAL})\"", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed before KEYWORDS string for a version is output\n"
	"(with --versionlines and nonverbose)"));

AddOption(STRING, "FORMAT_AFTER_KEYWORDS",
	"(%{COLOR_NORMAL})\"(%{COLOR_NORMAL_END})", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed after KEYWORDS string for a version is output.\n"
	"(with --versionlines and nonverbose)"));

AddOption(STRING, "FORMAT_VER_LINESKIP",
	"\\n                          ", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the lineskip used for the versionline appendix."));

AddOption(STRING, "FORMAT_VERSION_KEYWORDS_NORMAL",
	"%{?PRINT_KEYWORDS}"
		"%{?VERSION_KEYWORDS_NORMAL}"
			"%{FORMAT_BEFORE_KEYWORDS}"
			"%{?USE_EFFECTIVE_KEYWORDS}"
				"%{FORMAT_KEYWORDSS}"
			"%{else}"
				"%{FORMAT_KEYWORDS}"
			"%{}"
			"%{FORMAT_AFTER_KEYWORDS}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the normal format for KEYWORDS for an available version."));

AddOption(STRING, "FORMAT_IUSE_NORMAL",
	"%{?PRINT_IUSE}"
		"%{?VERSION_IUSE_NORMAL}"
			"{haveuse}"
				"%{FORMAT_BEFORE_IUSE}"
				"%{FORMAT_VERSION_IUSE}"
				"%{FORMAT_AFTER_IUSE}"
			"{}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the normal format for IUSE for an available version."));

AddOption(STRING, "FORMAT_DEPS_NORMAL",
	"%{?DEP}"
		"%{?VERSION_DEPS_NORMAL}"
			"{havedepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})DEPEND:(%{COLOR_RESET})    %{FORMAT_DEPEND}"
			"{}"
			"{haverdepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})RDEPEND:(%{COLOR_RESET})   %{FORMAT_RDEPEND}"
			"{}"
			"{havepdepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})PDEPEND:(%{COLOR_RESET})   %{FORMAT_PDEPEND}"
			"{}"
			"{havehdepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})HDEPEND:(%{COLOR_RESET})   %{FORMAT_HDEPEND}"
			"{}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for DEPs for an available version."));

AddOption(STRING, "FORMAT_VERSION_KEYWORDS_VERBOSE",
	"%{?PRINT_KEYWORDS}"
		"%{?VERSION_KEYWORDS_VERBOSE}"
			"%{!PRINT_ALWAYS}{versionkeywords}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})KEYWORDS:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{versionkeywords}%{}"
				"  %{FORMAT_KEYWORDS}(%{COLOR_RESET})"
			"{}"
			"%{!PRINT_ALWAYS}{versionekeywords}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})KEYWORDS*:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}"
					"{!versionekeywords}"
						"%{FORMAT_KEYWORDS_EQUAL}"
					"{else}"
				"%{}"
				" %{FORMAT_KEYWORDSS}(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for KEYWORDS for an available version."));

AddOption(STRING, "FORMAT_IUSE_VERBOSE",
	"%{?PRINT_IUSE}"
		"%{?VERSION_IUSE_VERBOSE}"
			"%{!PRINT_ALWAYS}{haveuse}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})IUSE:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{haveuse}%{}"
				"      %{FORMAT_VERSION_IUSE}(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for IUSE for an available version."));

AddOption(STRING, "FORMAT_DEPS_VERBOSE",
	"%{?DEP}"
		"%{?VERSION_DEPS_VERBOSE}"
			"%{!PRINT_ALWAYS}{havedepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})DEPEND:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{depend}%{}"
				"    %{FORMAT_DEPEND_VERBOSE}"
			"{}"
			"%{!PRINT_ALWAYS}{haverdepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})RDEPEND:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{haverdepend}%{}"
				"   %{FORMAT_RDEPEND_VERBOSE}"
			"{}"
			"%{!PRINT_ALWAYS}{havepdepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})PDEPEND:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{havepdepend}%{}"
				"   %{FORMAT_PDEPEND_VERBOSE}"
			"{}"
			"%{!PRINT_ALWAYS}{havehdepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})HDEPEND:(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{havehdepend}%{}"
				"   %{FORMAT_HDEPEND_VERBOSE}"
			"{}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for DEPs for an available version."));

AddOption(STRING, "FORMAT_VERSION_APPENDIX_NORMAL",
	"%{FORMAT_VERSION_KEYWORDS_NORMAL}"
	"%{FORMAT_IUSE_NORMAL}"
	"%{FORMAT_DEPS_NORMAL}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines normal data appended to available versions with --versionlines"));

AddOption(STRING, "FORMAT_VERSION_APPENDIX_VERBOSE",
	"%{FORMAT_VERSION_KEYWORDS_VERBOSE}"
	"%{FORMAT_IUSE_VERBOSE}"
	"%{FORMAT_DEPS_VERBOSE}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines verbose data appended to available versions with --versionlines"));

AddOption(STRING, "FORMAT_VERSION_APPENDIX",
	"{$modus=verbose}"
		"%{FORMAT_VERSION_APPENDIX_VERBOSE}"
	"{else}"
		"%{FORMAT_VERSION_APPENDIX_NORMAL}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the data appended to available versions with --versionlines"));

AddOption(STRING, "FORMAT_SLOTLINESKIP_VERSIONLINES",
	"\\n      %{FORMAT_SLOT}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines lineskip + slot if slotsorted versionlines are used."));

AddOption(STRING, "FORMAT_SLOTLINESKIP",
	"\\n\\t%{FORMAT_SLOT}\\t", _(
	"This variable is only used for delayed substitution.\n"
	"It defines lineskip + slot if slotsort but no versionlines are used."));

AddOption(STRING, "FORMAT_VERSLINESKIP",
	"\\n\\t%{FORMAT_STABILITY}\\t", _(
	"This variable is only used for delayed substitution.\n"
	"It defines lineskip + stability if lineskip is used."));

AddOption(STRING, "FORMAT_INST_LINESKIP",
	"%{FORMAT_VER_LINESKIP}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the lineskip for installed versions."));
#endif

#if (DEFAULT_PART == 5)

AddOption(STRING, "VSORTL",
	"%{FORMAT_VERSLINESKIP}{*sep=\\t}%{PVERSIONS_VERBOSE}{!*sep}"
	"%{FORMAT_VERSION_APPENDIX}{last}{*sorted=version}{}", _(
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; versionsorted with versionlines."));

AddOption(STRING, "VSORT",
	"%{AVERSIONS_VERBOSE}{last}{*sorted=version}{else} {}", _(
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; versionsorted without versionlines."));

AddOption(STRING, "SSORTL",
	"{slotfirst}%{FORMAT_SLOTLINESKIP_VERSIONLINES}{}"
	"%{FORMAT_VERSLINESKIP}{*sep=\\t}%{PVERSIONO_VERBOSE}{!*sep}"
	"%{FORMAT_VERSION_APPENDIX}{last}{*sorted=slot}{}", _(
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; slotsorted with versionlines."));

AddOption(STRING, "SSORT",
	"{slotfirst}"
		"{oneslot}%{FORMAT_SLOT} "
		"{else}%{FORMAT_SLOTLINESKIP}{}"
	"{else} {}"
	"%{AVERSIONO_VERBOSE}{last}{*sorted=slot}{}", _(
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; slotsorted without versionlines."));

AddOption(STRING, "FORMAT_COLL",
	"%{FORMAT_BEFORE_COLL}%{FORMAT_COLLIUSE}%{FORMAT_AFTER_COLL}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data in one line."));

AddOption(STRING, "FORMAT_COLL_SEP",
	"%{FORMAT_BEFORE_COLL_SEP}%{FORMAT_COLLIUSE}%{FORMAT_AFTER_COLL_SEP}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data in a separate line."));

AddOption(STRING, "FORMAT_COLL_VERBOSE",
	"%{!PRINT_ALWAYS}{havecolliuse}%{}"
		"\\n     (%{COLOR_TITLE})IUSE \\(all versions\\):(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{havecolliuse}%{}"
		" %{FORMAT_COLLIUSE}(%{COLOR_RESET})"
	"{}",  _(
	"This variable is only used for delayed substitution.\n"
	"It is the format for package IUSE in single-line verbose mode."));

AddOption(STRING, "FORMAT_COLL_VERBOSE_SEP",
	"%{FORMAT_COLL_VERBOSE}", _(
	"This variable is only used for delayed substitution.\n"
	"It is the format for package IUSE in multiline verbose mode."));

AddOption(STRING, "FORMAT_COLL_IUSE",
	"%{?PRINT_IUSE}"
		"{$modus=verbose}"
			"{$sorted=version}%{FORMAT_COLL_VERBOSE}"
			"{else}%{FORMAT_COLL_VERBOSE_SEP}{}"
		"{else}"
			"{havecolliuse}"
				"{$sorted=version}%{FORMAT_COLL}"
				"{else}%{FORMAT_COLL_SEP}{}"
			"{}"
		"{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data."));

AddOption(STRING, "FORMAT_COLL_IUSE_LIST",
	"%{?PRINT_IUSE}"
		"%{!VERSION_IUSE_NORMAL}"
			"{$modus=normal}"
				"{havecolliuse}%{FORMAT_COLL_SEP}{}"
			"%{?VERSION_IUSE_VERBOSE}{}%{else}{else}%{}"
		"%{else}"
			"%{!VERSION_IUSE_VERBOSE}{$modus=verbose}%{}"
		"%{}"
		"%{!VERSION_IUSE_VERBOSE}"
				"%{FORMAT_COLL_VERBOSE_SEP}"
			"{}"
		"%{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data with --versionlines."));

AddOption(STRING, "INORMAL",
	"%{IVERSIONS_VERBOSE}"
	"(%{COLOR_DATE})\\(<date:FORMAT_INSTALLATION_DATE>\\)(%{COLOR_RESET})"
	"%{FORMAT_INST_USEFLAGS}"
	"{!last}"
		"{versionlines}%{FORMAT_INST_LINESKIP}"
		"{else} {}"
	"{}", _(
	"This variable is used as a version formatter.\n"
	"It defines the normal format of installed versions."));

AddOption(STRING, "ICOMPACT",
	"%{IVERSIONS_COMPACT}"
	"(%{COLOR_NORMAL})@(%{COLOR_DATE})<date:FORMAT_SHORT_INSTALLATION_DATE>(%{COLOR_RESET})"
	"{!last} {}", _(
	"This variable is used as a version formatter.\n"
	"It defines the compact format of installed versions."));

AddOption(STRING, "IVERBOSE",
	"(%{COLOR_INST_TITLE})Version:(%{COLOR_RESET})   "
	"%{IVERSIONS_VERBOSE}(%{COLOR_RESET})"
	"%{FORMAT_INST_LINESKIP}"
	"(%{COLOR_INST_TITLE})Date:(%{COLOR_RESET})      "
	"(%{COLOR_DATE})<date:FORMAT_INSTALLATION_DATE>(%{COLOR_RESET})"
	"{haveuse}"
		"%{FORMAT_INST_LINESKIP}"
		"(%{COLOR_INST_TITLE})USE:(%{COLOR_RESET})       "
		"%{?EIX_USE_EXPAND}<use*>%{else}<use>%{}"
	"{}"
	"{!last}%{FORMAT_INST_LINESKIP}{}", _(
	"This variable is used as a version formatter.\n"
	"It defines the verbose format of installed versions."));

AddOption(STRING, "FORMAT_AVAILABLEVERSIONS",
	"{versionlines}"
		"{slotsorted}<availableversions:VSORTL:SSORTL>"
		"{else}<availableversions:VSORTL>{}"
		"%{FORMAT_COLL_IUSE_LIST}"
	"{else}"
		"{slotsorted}<availableversions:VSORT:SSORT>"
		"{else}<availableversions:VSORT>{}"
		"%{FORMAT_COLL_IUSE}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for printing available versions."));

AddOption(STRING, "FORMAT_INST_USEFLAGS",
	"{haveuse}"
		"(%{COLOR_NORMAL})\\("
		"%{?EIX_USE_EXPAND}<use*>%{else}<use>%{}"
		"(%{COLOR_NORMAL})\\)(%{COLOR_NORMAL_END})"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for useflags in installed versions."));

AddOption(STRING, "INSTALLEDVERSIONS_NORMAL",
	"<installedversions:INORMAL>", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions normally."));

AddOption(STRING, "INSTALLEDVERSIONS_COMPACT",
	"<installedversions:ICOMPACT>", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions compactly."));

AddOption(STRING, "INSTALLEDVERSIONS_VERBOSE",
	"<installedversions:IVERBOSE>", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions verbosely."));

AddOption(STRING, "INSTALLEDVERSIONS",
	"{$modus=verbose}"
		"%{INSTALLEDVERSIONS_VERBOSE}"
	"{else}"
		"%{INSTALLEDVERSIONS_NORMAL}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions."));

AddOption(STRING, "FORMATLINE_INSTALLEDVERSIONS",
	"%{!PRINT_ALWAYS}{installed}%{}"
		"     (%{COLOR_TITLE})Installed versions:(%{COLOR_RESET})"
		"  "
		"%{?PRINT_ALWAYS}{installed}%{}"
			"%{INSTALLEDVERSIONS}"
		"%{?PRINT_ALWAYS}{else}"
			"None{}\\n"
		"%{else}\\n{}"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with installed versions."));

AddOption(STRING, "DIFF_FORMATLINE_INSTALLEDVERSIONS",
	"{installed}%{INSTALLEDVERSIONS_COMPACT}"
	"(%{COLOR_NORMAL});(%{COLOR_NORMAL_END}) {}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for eix-diff for installed versions."));

AddOption(STRING, "FORMAT_FINISH",
	"\\n", _(
	"This variable is only used for delayed substitution.\n"
	"It prints a newline at the end of a package."));

AddOption(STRING, "FORMAT_NAME",
	"{system}(%{COLOR_CATEGORY_SYSTEM})"
	"{else}"
		"{world}(%{COLOR_CATEGORY_WORLD})"
		"{else}"
			"{world_sets}(%{COLOR_CATEGORY_WORLD_SETS})"
			"{else}(%{COLOR_CATEGORY}){}"
		"{}"
	"{}<category>(%{COLOR_NORMAL})/"
	"{marked}(%{COLOR_MARKED_NAME})"
	"{else}"
		"{world}(%{COLOR_WORLD})"
		"{else}"
			"{world_sets}(%{COLOR_WORLD_SETS})"
			"{else}(%{COLOR_NAME}){}"
		"{}"
	"{}<name>(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the printing the package name."));

AddOption(STRING, "FORMAT_HEADER",
	"{installed}"
		"(%{COLOR_NORMAL})[{!*updn}"
		"{upgrade}{*updn}%{TAG_UPGRADE}{}"
		"{downgrade}{*updn}%{TAG_DOWNGRADE}{}"
		"{!$updn}%{TAG_INSTALLED}{}"
		"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})"
	"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the normal header symbols."));

AddOption(STRING, "FORMAT_HEADER_VERBOSE",
	"{installed}(%{COLOR_INST_TAG})%{STRING_PLAIN_INSTALLED}"
	"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}"
	"(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the verbose header symbols."));

AddOption(STRING, "FORMAT_HEADER_COMPACT",
	"(%{COLOR_NORMAL})["
	"{installed}"
		"{!*updn}"
		"{upgrade}{*updn}%{TAG_UPGRADE}{}"
		"{downgrade}{*updn}%{TAG_DOWNGRADE}{}"
		"{!$updn}%{TAG_INSTALLED}{}"
	"{else}%{TAG_UNINSTALLED}{}"
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the compact header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_NEW",
	"(%{COLOR_NORMAL})[{*better=\\ }{*stable=\\ }{!*updn}"
	"{havebest}%{TAG_STABILIZE}{!*better}{}"
	"{upgrade}{*updn}%{TAG_UPGRADE}{}"
	"{downgrade}{$updn}{!*stable}{}{*updn}%{TAG_DOWNGRADE}{}"
	"{!$updn}%{TAG_NEW}{}"
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})<$better><$stable>"
	" (%{DIFF_COLOR_NEW})%{DIFF_STRING_NEW}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-new header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_DELETE",
	"{installed}"
		"(%{COLOR_NORMAL})["
		"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}"
		"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})"
	"{else}"
		"   "
	"{}"
	"   (%{DIFF_COLOR_DELETE})%{DIFF_STRING_DELETE}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-delete header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_CHANGED",
	"(%{COLOR_NORMAL})[{*better=\\ }{*up=\\ }{*down=\\ }"
	"{havebest}{!oldhavebest}%{TAG_STABILIZE}{!*better}{}{}"
	"{upgrade}{!*up}%{TAG_UPGRADE}{}"
	"{downgrade}{!*down}%{TAG_DOWNGRADE}{}"
	"{$up}{better}{!*up}%{TAG_BETTER}{}{}"
	"{$down}{worse}{!*down}%{TAG_WORSE}{}{}"
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})<$better><$up><$down>"
	" (%{DIFF_COLOR_CHANGED})%{DIFF_STRING_CHANGED}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-changed header symbols."));

AddOption(STRING, "FORMAT_BEST_COMPACT",
	"{havebest}"
		"<bestversion:AVERSIONS_COMPACT>"
	"{else}"
		"{havebest*}"
			"<bestversion*:AVERSIONS_COMPACT>"
		"{else}"
			"%{?NOBEST_COMPACT}"
				"%{FORMAT_NOBEST}"
			"%{else}"
				"<availableversions:VSORT>"
			"%{}"
		"{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the compact format for the best version(s)."));

AddOption(STRING, "FORMAT_BEST_CHANGE",
	"{havebest}"
		"<bestslotversions:VSORT>"
	"{else}"
		"{havebest*}"
			"<bestslotversions*:VSORT>"
		"{else}"
			"%{?NOBEST_CHANGE}"
				"%{FORMAT_NOBEST_CHANGE}"
			"%{else}"
				"<availableversions:VSORT>"
			"%{}"
		"{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the compact format for the best version(s) in case of changes."));

AddOption(STRING, "DIFF_FORMAT_BEST",
	"{havebest}"
		"<bestversion:AVERSIONS_COMPACT>"
	"{else}"
		"{havebest*}"
			"<bestversion*:AVERSIONS_COMPACT>"
		"{else}"
			"%{?DIFF_NOBEST}"
				"%{FORMAT_NOBEST}"
			"%{else}"
				"<availableversions:VSORT>"
			"%{}"
		"{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff format for the best version(s)."));

AddOption(STRING, "DIFF_FORMAT_BEST_CHANGE",
	"{havebest}"
		"<bestslotversions:VSORT>"
	"{else}"
		"{havebest*}"
			"<bestslotversions*:VSORT>"
		"{else}"
			"%{?DIFF_NOBEST_CHANGE}"
				"%{FORMAT_NOBEST_CHANGE}"
			"%{else}"
				"<availableversions:VSORT>"
			"%{}"
		"{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff format for the best version(s) in case of changes."));

AddOption(STRING, "DIFF_FORMAT_OLDBEST_CHANGE",
	"{oldhavebest}"
		"<oldbestslotversions:VSORT>"
	"{else}"
		"{oldhavebest*}"
			"<oldbestslotversions*:VSORT>"
		"{else}"
			"%{?DIFF_NOBEST_CHANGE}"
				"%{FORMAT_NOBEST_CHANGE}"
			"%{else}"
				"<oldavailableversions:VSORT>"
			"%{}"
		"{}"
	"{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff format for the old best versions in case of changes."));

AddOption(STRING, "DIFF_FORMAT_CHANGED_VERSIONS",
	"%{?DIFF_PRINT_INSTALLED}"
		"%{DIFF_FORMATLINE_INSTALLEDVERSIONS}"
	"%{}"
	"%{DIFF_FORMAT_OLDBEST_CHANGE}"
	" (%{COLOR_NORMAL})->(%{COLOR_NORMAL_END}) "
	"%{DIFF_FORMAT_BEST_CHANGE}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff format for changed versions."));

AddOption(STRING, "FORMAT_OVERLAYKEY",
	"{overlaykey} <overlaykey>{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the printing the optional overlay key."));

AddOption(STRING, "FORMATLINE_NAME",
	"%{FORMAT_HEADER} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the normal header line."));

AddOption(STRING, "FORMATLINE_NAME_VERBOSE",
	"%{FORMAT_HEADER_VERBOSE} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}\\n", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the verbose header line."));

AddOption(STRING, "FORMATLINE_NAME_COMPACT",
	"%{FORMAT_HEADER_COMPACT} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the compact header line."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_NEW",
	"%{DIFF_FORMAT_HEADER_NEW} %{FORMAT_NAME} ", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-new header."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_DELETE",
	"%{DIFF_FORMAT_HEADER_DELETE} %{FORMAT_NAME} ", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-delete header."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_CHANGED",
	"%{DIFF_FORMAT_HEADER_CHANGED} %{FORMAT_NAME} ", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-changed header."));

AddOption(STRING, "FORMATLINE_AVAILABLEVERSIONS",
	"     (%{COLOR_TITLE})Available versions:(%{COLOR_RESET})  %{FORMAT_AVAILABLEVERSIONS}\\n", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with installed versions."));

AddOption(STRING, "DIFF_FORMATLINE_BEST",
	"(%{COLOR_NORMAL})\\(%{DIFF_FORMAT_BEST}"
	"(%{COLOR_NORMAL}))(%{COLOR_NORMAL_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff line for the best versions/slots."));

AddOption(STRING, "DIFF_FORMATLINE_CHANGED_VERSIONS",
	"(%{COLOR_NORMAL})\\(%{DIFF_FORMAT_CHANGED_VERSIONS}"
		"(%{COLOR_NORMAL}))(%{COLOR_NORMAL_END})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff line for changed versions."));

AddOption(STRING, "FORMATLINE_MARKEDVERSIONS",
	"%{!PRINT_ALWAYS}{havemarkedversion}%{}"
		"     (%{COLOR_TITLE})Marked:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{havemarkedversion}%{}"
		"              "
		"(%{COLOR_MARKED_VERSION})<markedversions:VERSION>(%{COLOR_RESET})"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with marked versions."));

AddOption(STRING, "FORMATLINE_PACKAGESETS",
	"%{!PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
		"     (%{COLOR_TITLE})Package sets:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
		"        "
		"(%{COLOR_PACKAGESETS})<%{PRINT_SETNAMES}>(%{COLOR_RESET})"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with package sets."));

AddOption(STRING, "FORMATLINE_HOMEPAGE",
	"%{!PRINT_ALWAYS}{homepage}%{}"
		"     (%{COLOR_TITLE})Homepage:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{homepage}%{}"
		"            "
		"(%{COLOR_NORMAL})"
		"<homepage>"
		"(%{COLOR_NORMAL_END})"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package homepage."));

AddOption(STRING, "FORMATLINE_BUGS",
	"%{?PRINT_BUGS}"
		"     (%{COLOR_TITLE})Find open bugs:(%{COLOR_RESET})"
		"      "
		"(%{COLOR_NORMAL})"
		"http://bugs.gentoo.org/buglist.cgi?quicksearch="
		"<category>%2F<name>\\n"
		"(%{COLOR_NORMAL_END})"
	"%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package bug-reference."));

AddOption(STRING, "FORMATLINE_DESCRIPTION",
	"%{!PRINT_ALWAYS}{description}%{}"
		"     (%{COLOR_TITLE})Description:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{description}%{}"
		"         "
		"(%{COLOR_NORMAL})"
		"<description>"
		"(%{COLOR_NORMAL_END})"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package description."));

AddOption(STRING, "FORMATLINE_BEST",
	"%{!PRINT_ALWAYS}{havebest}%{}"
		"     (%{COLOR_TITLE})Best versions/slot:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{havebest}%{}"
		"  <bestslotversions:VSORT>"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the best versions/slots."));

AddOption(STRING, "FORMATLINE_RECOMMEND",
	"%{!PRINT_ALWAYS}{recommend}%{}"
		"     (%{COLOR_TITLE})Recommendation:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{recommend}%{}"
		"      "
		"{upgrade}(%{COLOR_UPGRADE_TEXT})Upgrade(%{COLOR_RESET})"
			"{downgrade}"
				" and "
			"{}"
		"{}"
		"{downgrade}(%{COLOR_DOWNGRADE_TEXT})Downgrade(%{COLOR_RESET}){}"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the up-/downgrade recommendations."));

AddOption(STRING, "FORMATLINE_LICENSES",
	"%{!PRINT_ALWAYS}{licenses}%{}"
		"     (%{COLOR_TITLE})License:(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{licenses}%{}"
		"             "
		"(%{COLOR_NORMAL})"
		"<licenses>"
		"(%{COLOR_NORMAL_END})"
	"%{?PRINT_ALWAYS}{}\\n%{else}\\n{}%{}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package licenses."));

AddOption(STRING, "DIFF_FORMATLINE",
	"%{FORMAT_OVERLAYKEY}"
	"(%{COLOR_NORMAL}): <description>(%{COLOR_NORMAL_END})"
	"%{FORMAT_FINISH}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format for eix-diff after the versions."));

AddOption(STRING, "FORMAT_ALL",
	"{*modus=normal}"
	"%{FORMATLINE_NAME}"
	"%{FORMATLINE_AVAILABLEVERSIONS}"
	"%{FORMATLINE_INSTALLEDVERSIONS}"
	"%{FORMATLINE_MARKEDVERSIONS}"
	"%{FORMATLINE_HOMEPAGE}"
	"%{FORMATLINE_DESCRIPTION}"
	"%{FORMAT_FINISH}", _(
	"This format is only used for delayed substitution in FORMAT.\n"
	"It defines the format of the normal output of eix."));

AddOption(STRING, "FORMAT_ALL_COMPACT",
	"{*modus=compact}"
	"%{FORMATLINE_NAME_COMPACT}"
	" (%{COLOR_NORMAL})\\((%{COLOR_NORMAL_END})"
	"{havemarkedversion}(%{COLOR_MARKED_VERSION})<markedversions:VERSION>(%{COLOR_RESET}); {}"
	"{installed}"
		"%{INSTALLEDVERSIONS_COMPACT}"
		"{recommend} -> %{FORMAT_BEST_CHANGE}{}"
	"{else}"
		"%{FORMAT_BEST_COMPACT}"
	"{}"
	"(%{COLOR_NORMAL})\\): <description>(%{COLOR_NORMAL_END})"
	"%{FORMAT_FINISH}", _(
	"This format is only used for delayed substitution in FORMAT_COMPACT.\n"
	"It defines the format of the compact output of eix (option -c)."));

AddOption(STRING, "FORMAT_ALL_VERBOSE",
	"{*modus=verbose}"
	"%{FORMATLINE_NAME_VERBOSE}"
	"%{FORMATLINE_AVAILABLEVERSIONS}"
	"%{FORMATLINE_INSTALLEDVERSIONS}"
	"%{FORMATLINE_BEST}"
	"%{FORMATLINE_RECOMMEND}"
	"%{FORMATLINE_MARKEDVERSIONS}"
	"%{FORMATLINE_PACKAGESETS}"
	"%{FORMATLINE_HOMEPAGE}"
	"%{FORMATLINE_BUGS}"
	"%{FORMATLINE_DESCRIPTION}"
	"%{FORMATLINE_LICENSES}"
	"%{FORMAT_FINISH}", _(
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

AddOption(STRING, "FORMAT_TEST_OBSOLETE",
	"%{FORMAT_ALL_COMPACT}", _(
	"The format used for output by the eix-test-obsolete script.\n"
	"The value %{FORMAT} is not allowed here; use %{FORMAT_ALL} instead."));

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

AddOption(STRING, "XML_KEYWORDS",
	"none", _(
	"Can be full/effective/both/full*/effective*/none.\n"
	"Depending on the value, with --xml the full/effective (or both types)\n"
	"KEYWORDS string is output for each version.\n"
	"With full*/effective* also both types are output if they differ."));

AddOption(STRING, "XML_OVERLAY",
	"false", _(
	"If false, the overlay is not output with --xml.\n"
	"For overlays without label (repository name) the overlay is output anyway."));

AddOption(STRING, "XML_DATE",
	"%s", _(
	"strftime() format for printing the installation date with --xml."));

AddOption(STRING, "FORMAT_BEFORE_SET_USE",
	"(%{COLOR_SET_USE})", _(
	"This string is printed before each set USE flag of an installed version."));

AddOption(STRING, "FORMAT_AFTER_SET_USE",
	"(%{COLOR_RESET})", _(
	"This string is printed after each set USE flag of an installed version."));

AddOption(STRING, "FORMAT_BEFORE_UNSET_USE",
	"(%{COLOR_UNSET_USE})-", _(
	"This string is printed before each unset USE flag of an installed version."));

AddOption(STRING, "FORMAT_AFTER_UNSET_USE",
	"(%{COLOR_RESET})", _(
	"This string is printed after each unset USE flag of an installed version."));

AddOption(STRING, "FORMAT_BEFORE_USE_EXPAND_START",
	"(%{COLOR_USE_EXPAND_START})", _(
	"This string is printed before the variable name of an USE_EXPAND use value."));

AddOption(STRING, "FORMAT_BEFORE_USE_EXPAND_END",
	"=\"", _(
	"This string is printed after the variable name of an USE_EXPAND use value."));

AddOption(STRING, "FORMAT_AFTER_USE_EXPAND",
	"(%{COLOR_USE_EXPAND_START})\"(%{COLOR_USE_EXPAND_END})", _(
	"This string is printed at the end of a USE_EXPAND use value."));

AddOption(STRING, "FORMAT_BEFORE_IUSE_EXPAND_START",
	"(%{COLOR_USE_EXPAND_START})", _(
	"This string is printed before the variable name of an USE_EXPAND iuse value."));

AddOption(STRING, "FORMAT_BEFORE_IUSE_EXPAND_END",
	"=\"(%{COLOR_VERSION_IUSE})", _(
	"This string is printed after the variable name of an USE_EXPAND iuse value."));

AddOption(STRING, "FORMAT_AFTER_IUSE_EXPAND",
	"(%{COLOR_USE_EXPAND_START})\"(%{COLOR_USE_EXPAND_END})", _(
	"This string is printed at the end of a USE_EXPAND iuse value."));

AddOption(STRING, "FORMAT_BEFORE_COLL_EXPAND_START",
	"(%{COLOR_USE_EXPAND_START})", _(
	"This string is printed before the variable name of an USE_EXPAND collected\n"
	"iuse value."));

AddOption(STRING, "FORMAT_BEFORE_COLL_EXPAND_END",
	"=\"(%{COLOR_COLL_IUSE})", _(
	"This string is printed after the variable name of an USE_EXPAND collected\n"
	"iuse value."));

AddOption(STRING, "FORMAT_AFTER_COLL_EXPAND",
	"(%{COLOR_USE_EXPAND_START})\"(%{COLOR_USE_EXPAND_END})", _(
	"This string is printed at the end of a USE_EXPAND collected iuse value."));

AddOption(STRING, "FORMAT_BEFORE_IUSE",
	"{!*colliuse}\\t(%{COLOR_NORMAL})[(%{COLOR_NORMAL_END})", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed before IUSE data for a version is output.\n"
	"(Normally, this is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_AFTER_IUSE",
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed after IUSE data for a version is output.\n"
	"(Normally, this is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_BEFORE_COLL",
	"{*colliuse} (%{COLOR_USE_COLL})\\{", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is meant for printing after all versions in a line)"));

AddOption(STRING, "FORMAT_AFTER_COLL",
	"(%{COLOR_USE_COLL})\\}(%{COLOR_RESET})", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed after IUSE data for all versions is output.\n"
	"(This is meant for printing after all versions in a line)"));

AddOption(STRING, "FORMAT_BEFORE_COLL_SEP",
	"\\n\\t(%{COLOR_USE_COLL})\\{", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is meant for printing in a separate line)"));

AddOption(STRING, "FORMAT_AFTER_COLL_SEP",
	"%{FORMAT_AFTER_COLL}", _(
	"This variable is only used for delayed substitution.\n"
	"This string is printed after IUSE data for all versions is output.\n"
	"(This is meant for printing in a separate line)"));

AddOption(STRING, "COLOR_MASKED",
	"red;%{BG0}|196;%{BG1}|red;%{BG2}|196;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color for masked versions."));

AddOption(STRING, "COLOR_UNSTABLE",
	"yellow;%{BG0}|190;%{BG1}|blue;%{BG2}|21;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color for unstable versions."));

AddOption(STRING, "COLOR_STABLE",
	"green;%{BG0}|46;%{BG1}|;%{BG2}|58;%{BG3}", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the color for stable versions."));

AddOption(STRING, "FORMAT_COLOR_MASKED",
	"(%{COLOR_MASKED})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format to change the color for masked versions."));

AddOption(STRING, "FORMAT_COLOR_UNSTABLE",
	"(%{COLOR_UNSTABLE})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format to change the color for unstable versions."));

AddOption(STRING, "FORMAT_COLOR_STABLE",
	"(%{COLOR_STABLE})", _(
	"This variable is only used for delayed substitution.\n"
	"It defines the format to change the color for stable versions."));

AddOption(STRING, "DUMMY",
	"", _(
	"This variable is ignored. You can use it to collect delayed references to\n"
	"locally added (unused) variables so that they are printed with --dump."));

#endif
