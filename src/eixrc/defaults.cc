// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <martin@mvath.de>

// check_includes: #include <config.h> include <cstdlib> include "eixTk/i18n.h" include "eixTk/dialect.h"

#if (DEFAULT_PART == 1)

AddOption(STRING, "EIXRC",
	"", P_("EIXRC",
	"The file which is used instead of /etc/eixrc and ~/.eixrc.\n"
	"This variable can of course only be set in the environment."));

AddOption(BOOLEAN, "WIDETERM",
	"", P_("WIDETERM",
	"This variable is only used for delayed substitution.\n"
	"It defines whether you have a wide terminal (>80 columns).\n"
	"An empty value or \"auto\" means that COLUMNS is used/calculated."));

AddOption(INTEGER, "COLUMNS",
	"", P_("COLUMNS",
	"Set the terminal width to define the WIDETERM expansion.\n"
	"An empty value or \"auto\" means that a heuristic is used instead."));

AddOption(PREFIXSTRING, "EIXRC_SOURCE",
	"", P_("EIXRC_SOURCE",
	"This path is prepended to source commands in /etc/eixrc.\n"
	"If set in /etc/eixrc it temporarily overrides the environment.\n"
	"You must not use delayed substitution in this variable."));

AddOption(STRING, "EIX_SYNC_OPTS",
	"", P_("EIX_SYNC_OPTS",
	"This variable is used for delayed substitution in EIX_SYNC_CONF.\n"
	"It contains code which is evaluated by eix-sync, so be aware of security!"));

AddOption(STRING, "EIX_SYNC_CONF",
	"%{EIX_SYNC_OPTS}", P_("EIX_SYNC_CONF",
	"The content of this variable is appended to /etc/eix-sync.conf\n"
	"In particular, it can be used to override options set in that file.\n"
	"Parts of this variable are evaluated in eix-sync: Be aware of security!"));

AddOption(STRING, "EIX_REMOTE_OPTS",
	"", P_("EIX_REMOTE_OPTS",
	"This variable contains default options for the eix-remote script.\n"
	"Note that its content is evaluated, so quote correctly. Typical example:\n"
	"EIX_REMOTE_OPTS='-f /var/lib/layman/eix-caches.tar.bz2'"));

AddOption(STRING, "EIX_LAYMAN_OPTS",
	"", P_("EIX_LAYMAN_OPTS",
	"This variable contains default options for the eix-layman script.\n"
	"Note that its content is evaluated, so quote correctly."));

AddOption(STRING, "EIX_TEST_OBSOLETE_OPTS",
	"-d", P_("EIX_TEST_OBSOLETE_OPTS",
	"This variable contains default options for the eix-test-obsolete script."));

AddOption(STRING, "EIX_INSTALLED_AFTER",
	"", P_("EIX_INSTALLED_AFTER",
	"This variable contains default arguments for the eix-installed-after script."));

AddOption(PREFIXSTRING, "EIX_PREFIX",
	EIX_PREFIX_DEFAULT, P_("EIX_PREFIX",
	"If this variable is set in the environment, then it is prefixed\n"
	"to the path where /etc/eixrc is searched. If it is not set in the\n"
	"environment, then PORTAGE_CONFIGROOT is used instead.\n"
	"If both are unset, the EIX_PREFIX default value is used instead.\n"
	"Moreover, EIX_PREFIX is used for delayed substitution for EPREFIX\n"
	"and EPREFIX_SOURCE."));

AddOption(PREFIXSTRING, "ROOT",
	ROOT_DEFAULT, P_("ROOT",
	"This variable is only used for delayed substitution.\n"
	"It influences most paths except for $HOME/.eixrc, the cache file\n"
	"passed in the command line, PORTAGE_PROFILE, PORTDIR,\n"
	"and overlays. In contrast to EPREFIX, further exceptions are:\n"
	"PORTAGE_CONFIGROOT, portage/scripts-internal stuff and the eix cachefile."));

AddOption(PREFIXSTRING, "EPREFIX",
	"%{EIX_PREFIX}" EPREFIX_DEFAULT, P_("EPREFIX",
	"This variable is used for delayed substitution for path prefixes.\n"
	"It influences most paths except for $HOME/.eixrc, the cache file\n"
	"passed in the command line, PORTAGE_PROFILE, PORTDIR, and overlays."));

AddOption(PREFIXSTRING, "EPREFIX_TREE",
	"", P_("EPREFIX_TREE",
	"This variable is only used for delayed substitution.\n"
	"It is the path prepended to PORTAGE_PROFILE, PORTDIR, and overlays."));

AddOption(PREFIXSTRING, "EPREFIX_ROOT",
	"%{??EPREFIX}%{EPREFIX}%{else}%{ROOT}%{}", P_("EPREFIX_ROOT",
	"It applies for those paths for which EPREFIX and ROOT should both apply.\n"
	"So you can decide here what to do if both are nonempty. For instance,\n"
	"the choice %{EPREFIX}%{ROOT} will apply both; the default applies EPREFIX\n"
	"but not ROOT for these paths in such a case (i.e. if both are nonempty)."));

AddOption(PREFIXSTRING, "PORTAGE_CONFIGROOT",
	"%{EPREFIX}", P_("PORTAGE_CONFIGROOT",
	"This path is prepended to the /etc paths."));

AddOption(PREFIXSTRING, "PORTAGE_DEFAULTS",
	"%{EPREFIX}/usr/share/portage/config", P_("PORTAGE_DEFAULTS",
	"This variable is only used for delayed substitution.\n"
	"It is the path prepended to MAKE_GLOBALS and PORTAGE_REPOS_CONF."));

AddOption(STRING, "MAKE_GLOBALS",
	"%{PORTAGE_DEFAULTS}/make.globals", P_("MAKE_GLOBALS",
	"This file is used instead of %{PORTAGE_CONFIGROOT}/etc/make.globals\n"
	"if it exists. This is reasonable for >=portage-2.2*"));

AddOption(STRING, "PORTAGE_REPOS_CONF",
	"%{PORTAGE_DEFAULTS}/repos.conf", P_("PORTAGE_REPOS_CONF",
	"This file is used to set the default repository information.\n"
	"The file /etc/portage/repos.conf is read only after this."));

AddOption(STRING, "PORTAGE_REPOSITORIES",
	"", P_("PORTAGE_REPOSITORIES",
	"If this variable is nonempty, its content is used as a substitute\n"
	"for all repos.conf files."));

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_EXEC",
	"%{EPREFIX}", P_("EPREFIX_PORTAGE_EXEC",
	"This might be used as prefix for /usr/bin/ebuild.\n"
	"It is also used as a prefix in the EBUILD_DEPEND_TEMP default."));

AddOption(PREFIXSTRING, "EPREFIX_SOURCE",
	"%{EIX_PREFIX}", P_("EPREFIX_SOURCE",
	"This path is prepended to source commands in /etc/make.{conf,globals}."));

AddOption(PREFIXSTRING, "EPREFIX_INSTALLED",
	"%{EPREFIX_ROOT}", P_("EPREFIX_INSTALLED",
	"Prefix to the path where eix expects information about installed packages."));

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_CACHE",
	"%{EPREFIX}", P_("EPREFIX_PORTAGE_CACHE",
	"This prefix is prepended to the portage cache."));

AddOption(PREFIXSTRING, "EPREFIX_ACCESS_OVERLAYS",
	"", P_("EPREFIX_ACCESS_OVERLAYS",
	"This prefix is prepended to overlays when their files are accessed."));

AddOption(PREFIXSTRING, "EPREFIX_PORTDIR",
	"%{EPREFIX_TREE}", P_("EPREFIX_PORTDIR",
	"This path is prepended to PORTDIR."));

AddOption(PREFIXSTRING, "EPREFIX_OVERLAYS",
	"%{EPREFIX_TREE}", P_("EPREFIX_OVERLAYS",
	"This path is prepended to PORTIDIR_OVERLAY values."));

AddOption(PREFIXSTRING, "EPREFIX_PORTAGE_PROFILE",
	"%{EPREFIX_TREE}", P_("EPREFIX_PORTAGE_PROFILE",
	"This path is prepended to PORTAGE_PROFILE."));

AddOption(PREFIXSTRING, "EPREFIX_VIRTUAL",
	"%{EPREFIX_TREE}", P_("EPREFIX_VIRTUAL",
	"This is prepended to overlays in eix database to test whether they exist."));

AddOption(STRING, "TMPDIR",
	"", P_("TMPDIR",
	"This variable is used for delayed substitution in EIX_TMPDIR.\n"
	"Usually this is set by the environment variable.\n"
	"eix exports this variable initialized to EIX_TMPDIR."));

AddOption(STRING, "EIX_TMPDIR",
	"%{TMPDIR}", P_("EIX_TMPDIR",
	"If this variable is nonempty, this directory is used instead of /tmp for\n"
	"temporary files. eix exports this variable as TMPDIR."));

AddOption(STRING, "EIX_CACHEFILE",
	"%{EPREFIX}" EIX_CACHEFILE, P_("EIX_CACHEFILE",
	"This file is the default eix cache."));

AddOption(STRING, "EIX_PREVIOUS",
	"%{EPREFIX}" EIX_PREVIOUS, P_("EIX_PREVIOUS",
	"This file is the previous eix cache (used by eix-diff and eix-sync)."));

AddOption(STRING, "EIX_REMOTE1",
	"%{EPREFIX}" EIX_REMOTECACHEFILE1, P_("EIX_REMOTE1",
	"This is the eix cache used when -R is in effect. If the string is nonempty,\n"
	"eix-remote uses this file for the output."));

AddOption(STRING, "EIX_REMOTE2",
	"%{EPREFIX}" EIX_REMOTECACHEFILE2, P_("EIX_REMOTE2",
	"This is the eix cache used when -Z is in effect. If the string is nonempty,\n"
	"eix-remote uses this file for the output."));

AddOption(INTEGER, "REMOTE_DEFAULT",
	"0", P_("REMOTE_DEFAULT",
	"1, 2, or 0 means that eix option -R, -Z, or neither is the default."));

AddOption(STRING, "EIX_REMOTEARCHIVE1",
	"%{EPREFIX}" EIX_REMOTEARCHIVE1, P_("EIX_REMOTEARCHIVE1",
	"This is a local copy of the remote archive used by eix-remote.\n"
	"If the name is empty, only a temporary file is used."));

AddOption(STRING, "EIX_REMOTEARCHIVE2",
	"%{EPREFIX}" EIX_REMOTEARCHIVE2, P_("EIX_REMOTEARCHIVE2",
	"This is a local copy of the remote archive used by eix-remote.\n"
	"If the name is empty, only a temporary file is used."));

AddOption(STRING, "LOCAL_LAYMAN",
	"", P_("LOCAL_LAYMAN",
	"If nonempty, this will override the heuristics for eix-remote -L."));

AddOption(STRING, "EBUILD_DEPEND_TEMP",
	"%{EPREFIX_PORTAGE_EXEC}/var/cache/edb/dep/aux_db_key_temp", P_("EBUILD_DEPEND_TEMP",
	"The path to the tempfile generated by \"ebuild depend\"."));

AddOption(STRING, "EIX_WORLD",
	"%{EPREFIX_ROOT}/var/lib/portage/world", P_("EIX_WORLD",
	"This file is considered as the world file."));

AddOption(STRING, "EIX_WORLD_SETS",
	"%{EIX_WORLD}_sets", P_("EIX_WORLD_SETS",
	"This file is considered as the world_sets file."));

AddOption(BOOLEAN, "SAVE_WORLD",
	"true", P_("SAVE_WORLD",
	"Store the information of the world file in the cache file.\n"
	"Set this to false if you do not want that everybody can get this information."));

AddOption(BOOLEAN, "CURRENT_WORLD",
	"true", P_("CURRENT_WORLD",
	"Prefer the current world file (if readable) over the data in the cachefile."));

AddOption(STRING, "EIX_USER",
	"portage", P_("EIX_USER",
	"Attempt to change to this user if possible. See EIX_UID."));

AddOption(STRING, "EIX_GROUP",
	"%{EIX_USER}", P_("EIX_GROUP",
	"Attempt to change to this group if possible. See EIX_GID."));

AddOption(INTEGER, "EIX_UID",
	"250", P_("EIX_UID",
	"If EIX_USER is empty or nonexistent, use this user id.\n"
	"In this case and if $EIX_UID <= 0, the user id is not changed."));

AddOption(STRING, "REQUIRE_DROP",
	"root", P_("REQUIRE_DROP",
	"If true it is required that dropping of permission succeeds.\n"
	"The special value root means: true for UID 0, false otherwise."));

AddOption(BOOLEAN, "NODROP_FATAL",
	"false", P_("NODROP_FATAL",
	"If true, a negative result of REQUIRE_DROP raises an error."));

AddOption(INTEGER, "EIX_GID",
	"%{EIX_UID}", P_("EIX_GID",
	"If EIX_GROUP is empty or nonexistent, use this group id.\n"
	"In this case and if $EIX_GID <= 0, the group id is not changed."));

AddOption(STRING, "PORTAGE_ROOTPATH",
	PORTAGE_ROOTPATH_DEFAULT, P_("PORTAGE_ROOTPATH",
	"This variable is passed unchanged to ebuild.sh\n"
	"Usually ebuild.sh uses it to calculate the PATH."));

AddOption(STRING, "DEFAULT_ARCH",
	ARCH_DEFAULT, P_("DEFAULT_ARCH",
	"The default ARCH if none is specified by the profile."));

AddOption(INTEGER, "NOFOUND_STATUS",
	EXPAND_STRINGIFY(EXIT_FAILURE), P_("NOFOUND_STATUS",
	"This value is used as exit status if there are 0 matches.\n"
	"The value of COUNT_ONLY_PRINTED is honoured."));

AddOption(INTEGER, "MOREFOUND_STATUS",
	EXPAND_STRINGIFY(EXIT_SUCCESS), P_("MOREFOUND_STATUS",
	"This value is used as exit status if there are 2 or more matches.\n"
	"The value of COUNT_ONLY_PRINTED is honoured."));

AddOption(INTEGER, "EIX_LIMIT",
	"50", P_("EIX_LIMIT",
	"The maximal number of matches shown on terminal in non-compact mode.\n"
	"The value 0 means all matches are shown."));

AddOption(INTEGER, "EIX_LIMIT_COMPACT",
	"200", P_("EIX_LIMIT_COMPACT",
	"The maximal number of matches shown on terminal in compact mode.\n"
	"The value 0 means all matches are shown."));

AddOption(BOOLEAN, "QUICKMODE",
	"false", P_("QUICKMODE",
	"Whether --quick is on by default."));

AddOption(BOOLEAN, "CAREMODE",
	"false", P_("CAREMODE",
	"Whether --care is on."));

AddOption(BOOLEAN, "USE_BUILD_TIME",
	"true", P_("USE_BUILD_TIME",
	"If true, use build time from BUILD_TIME entry instead of reading the install\n"
	"time from the directory timestamp. This is usually preferable but slower.\n"
	"The BUILD_TIME exists only for packages emerged with >=portage-2.2_rc63"));

AddOption(BOOLEAN, "QUIETMODE",
	"false", P_("QUIETMODE",
	"Whether --quiet is on by default."));

AddOption(STRING, "PRINT_APPEND",
	"\\n", P_("PRINT_APPEND",
	"This string is appended to the output of --print.\n"
	"To read variables in a shell without omitting trailing spaces, use e.g.\n"
	"VAR=`PRINT_APPEND=x eix --print VAR` ; VAR=${VAR%x}"));

AddOption(BOOLEAN, "DIFF_ONLY_INSTALLED",
	"false", P_("DIFF_ONLY_INSTALLED",
	"If true, eix-diff will only consider version changes for installed packages."));

AddOption(BOOLEAN, "DIFF_NO_SLOTS",
	"false", P_("DIFF_NO_SLOTS",
	"If true, eix-diff will not consider slots for version changes."));

AddOption(BOOLEAN, "DIFF_SEPARATE_DELETED",
	"true", P_("DIFF_SEPARATE_DELETED",
	"If false, eix-diff will mix deleted and changed packages"));

AddOption(BOOLEAN, "NO_RESTRICTIONS",
	"false", P_("NO_RESTRICTIONS",
	"This variable is only used for delayed substitution.\n"
	"If false, RESTRICT and PROPERTIES values are output."));

AddOption(BOOLEAN, "NO_BINARY",
	"false", P_("NO_BINARY",
	"This variable is only used for delayed substitution.\n"
	"If false, tags are output for packages/versions with *.tbz2 files."));

AddOption(BOOLEAN, "EIX_USE_EXPAND",
	"true", P_("EIX_USE_EXPAND",
	"This variable is only used for delayed substitution.\n"
	"If true, USE_EXPAND variables are output separately."));

AddOption(BOOLEAN, "OMIT_EXPAND",
	"false", P_("OMIT_EXPAND",
	"This variable is only used for delayed substitution.\n"
	"If true, USE_EXPAND variables are omitted."));

AddOption(BOOLEAN, "RESTRICT_INSTALLED",
	"true", P_("RESTRICT_INSTALLED",
	"If true, calculate RESTRICT/PROPERTIES for installed versions."));

AddOption(BOOLEAN, "CARE_RESTRICT_INSTALLED",
	"true", P_("CARE_RESTRICT_INSTALLED",
	"If true, read RESTRICT for installed versions always from disk.\n"
	"This is ignored if RESTRICT_INSTALLED=false."));

AddOption(BOOLEAN, "DEPS_INSTALLED",
	"true", P_("DEPS_INSTALLED",
	"If true, read *DEPEND for installed versions always from disk.\n"
	"This is ignored if DEP=false."));

AddOption(BOOLEAN, "DEP",
	DEP_DEFAULT, P_("DEP",
	"If true, store/use {R,P,H,}DEPEND (e.g. shown with eix -lv).\n"
	"Usage of DEP roughly doubles disk resp. memory requirements."));

AddOption(BOOLEAN, "REQUIRED_USE",
	REQUIRED_USE_DEFAULT, P_("REQUIRED_USE",
	"If true, store/use REQUIRED_USE. Usage increases disk/memory requirements."));

AddOption(STRING, "DEFAULT_FORMAT",
	"normal", P_("DEFAULT_FORMAT",
	"Defines whether --compact or --verbose is on by default."));

AddOption(STRING, "PRINT_COUNT_ALWAYS",
	"false", P_("PRINT_COUNT_ALWAYS",
	"Allowed values are true/false/never.\n"
	"If true, always print the number of matches (even 0 or 1) in the last line."));

AddOption(BOOLEAN, "NOCOLORS",
	"%{NOCOLOR}", P_("NOCOLORS",
	"Do not output colors."));

AddOption(BOOLEAN, "NOSTATUSLINE",
	"%{NOCOLOR}", P_("NOSTATUSLINE",
	"Do not output status line."));

AddOption(BOOLEAN, "NOPERCENTAGE",
	"%{NOCOLOR}", P_("NOPERCENTAGE",
	"Do not output percentage progress."));

AddOption(BOOLEAN, "FORCE_USECOLORS",
	"false", P_("FORCE_USECOLORS",
	"This variable is only used for delayed substitution.\n"
	"It is the default for FORCE_{COLORS,STATUSLINE,PERCENTAGE}."));

AddOption(BOOLEAN, "FORCE_COLORS",
	"%{FORCE_USECOLORS}", P_("FORCE_COLORS",
	"Output colors even if not printing to a terminal."));

AddOption(BOOLEAN, "FORCE_STATUSLINE",
	"%{FORCE_USECOLORS}", P_("FORCE_STATUSLINE",
	"Output status line even if not printing to a terminal."));

AddOption(BOOLEAN, "FORCE_PERCENTAGE",
	"%{FORCE_USECOLORS}", P_("FORCE_PERCENTAGE",
	"Output percentage progress even if not printing to a terminal."));

AddOption(BOOLEAN, "COLOR_ORIGINAL",
	"true", P_("COLOR_ORIGINAL",
	"This variable is only used for delayed substitution.\n"
	"If false, versions are only colored according to the local setting."));

AddOption(BOOLEAN, "COLOR_LOCAL_MASK",
	"false", P_("COLOR_LOCAL_MASK",
	"This variable is only used for delayed substitution.\n"
	"If false, COLOR_ORIGINAL=false has no effect on versions which are\n"
	"only locally masked (i.e. [m])."));

AddOption(BOOLEAN, "STYLE_VERSION_SORTED",
	"false", P_("STYLE_VERSION_SORTED",
	"Defines whether --versionsorted is on by default."));

AddOption(BOOLEAN, "STYLE_VERSION_LINES",
	"false", P_("STYLE_VERSION_LINES",
	"Defines whether --versionlines is on by default."));

AddOption(BOOLEAN, "COLORED_SLOTS",
	"true", P_("COLORED_SLOTS",
	"This variable is only used for delayed substitution.\n"
	"If false, the slotnames appended to versions are not colored."));

AddOption(BOOLEAN, "COLON_SLOTS",
	"false", P_("COLON_SLOTS",
	"This variable is only used for delayed substitution.\n"
	"If true, separated slots from versions with a colon instead of braces."));

AddOption(BOOLEAN, "DEFAULT_IS_OR",
	"false", P_("DEFAULT_IS_OR",
	"Whether default concatenation of queries is -o (or) or -a (and)"));

AddOption(BOOLEAN, "DUP_PACKAGES_ONLY_OVERLAYS",
	"false", P_("DUP_PACKAGES_ONLY_OVERLAYS",
	"Whether checks for duplicate packages occur only among overlays"));

AddOption(BOOLEAN, "DUP_VERSIONS_ONLY_OVERLAYS",
	"false", P_("DUP_VERSIONS_ONLY_OVERLAYS",
	"Whether checks for duplicate versions occur only among overlays"));

AddOption(STRING, "OVERLAYS_LIST",
	"all-used-renumbered", P_("OVERLAYS_LIST",
	"Which overlays to list (all/all-if-used/all-used/all-used-renumbered/no)"));

AddOption(INTEGER, "LEVENSHTEIN_DISTANCE",
	LEVENSHTEIN_DISTANCE_DEFAULT, P_("LEVENSHTEIN_DISTANCE",
	"The default maximal levensthein distance for which a string is\n"
	"considered a match for the fuzzy match algorithm."));

AddOption(BOOLEAN, "UPDATE_VERBOSE",
	"false", P_("UPDATE_VERBOSE",
	"Whether eix-update -v is on by default (output cache method per ebuild)"));

AddOption(STRING, "CACHE_METHOD_PARSE",
	"#metadata-md5#metadata-flat#assign", P_("CACHE_METHOD_PARSE",
	"This string is appended to all cache methods using parse[*] or ebuild[*]."));

AddOption(STRING, "PORTDIR_CACHE_METHOD",
	PORTDIR_CACHE_METHOD, P_("PORTDIR_CACHE_METHOD",
	"Portage cache-backend that should be used for PORTDIR\n"
	"(metadata[:*]/sqlite/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])"));

AddOption(STRING, "OVERLAY_CACHE_METHOD",
	"parse|ebuild*", P_("OVERLAY_CACHE_METHOD",
	"Portage cache-backend that should be used for the overlays.\n"
	"(metadata[:*]/sqlite/flat[:*]/portage-2.1/parse[*][|]ebuild[*]/eix[*][:*])"));

AddOption(STRING, "ADD_CACHE_METHOD",
	"", P_("ADD_CACHE_METHOD",
	"This variable is only used for delayed substitution in CACHE_METHOD.\n"
	"It is meant to be a local addition to CACHE_METHOD."));

AddOption(STRING, "CACHE_METHOD",
	"%{ADD_CACHE_METHOD}", P_("CACHE_METHOD",
	"Overrides OVERLAY_CACHE_METHOD or PORTDIR_CACHE_METHOD for particular paths.\n"
	"This is a list of pairs DIR-PATTERN METHOD. Later entries take precedence."));

AddOption(STRING, "ADD_OVERRIDE_CACHE_METHOD",
	"", P_("ADD_OVERRIDE_CACHE_METHOD",
	"This variable is only used for delayed substitution in OVERRIDE_CACHE_METHOD.\n"
	"It is meant to be a local addition to OVERRIDE_CACHE_METHOD."));

AddOption(STRING, "OVERRIDE_CACHE_METHOD",
	"%{ADD_OVERRIDE_CACHE_METHOD}", P_("OVERRIDE_CACHE_METHOD",
	"This variable can override the choices of CACHE_METHOD,\n"
	"and in addition it can override the choices made by KEEP_VIRTUALS."));

AddOption(STRING, "REPO_NAMES",
	"", P_("REPO_NAMES",
	"This is a list of pairs DIR-PATTERN OVERLAY_LABEL.\n"
	"When a new cachefile is created, the overlay matching DIR-PATTERN obtains\n"
	"the label OVERLAY_LABEL, independent of the content of profiles/repo_name\n"
	"or the label associated by KEEP_VIRTUALS.\n"
	"The last matching DIR_PATTERN takes precedence."));

AddOption(STRING, "EXCLUDE_OVERLAY",
	"", P_("EXCLUDE_OVERLAY",
	"List of overlays that should be excluded from the index."));

AddOption(STRING, "ADD_OVERLAY",
	"", P_("ADD_OVERLAY",
	"List of overlays that should be added to the index."));

AddOption(BOOLEAN, "EXPORT_PORTDIR_OVERLAY",
	"true", P_("EXPORT_PORTDIR_OVERLAY",
	"If true and overlays are excluded or added, export modified PORTDIR_OVERLAY."));

AddOption(BOOLEAN, "KEEP_VIRTUALS",
	"false", P_("KEEP_VIRTUALS",
	"Keep virtuals of the old cache file by adding corresponding entries\n"
	"implicitly to the values of ADD_OVERLAY and ADD_CACHE_METHOD"));

AddOption(BOOLEAN, "LOCAL_PORTAGE_CONFIG",
	"true", P_("LOCAL_PORTAGE_CONFIG",
	"If false, /etc/portage and ACCEPT_KEYWORDS are ignored."));

AddOption(BOOLEAN, "ALWAYS_ACCEPT_KEYWORDS",
	ALWAYS_ACCEPT_KEYWORDS_DEFAULT, P_("ALWAYS_ACCEPT_KEYWORDS",
	"If true, ACCEPT_KEYWORDS is used even without LOCAL_PORTAGE_CONFIG,\n"
	"e.g. to determine the \"default\" stability."));

AddOption(BOOLEAN, "UPGRADE_LOCAL_MODE",
	"", P_("UPGRADE_LOCAL_MODE",
	"If +/-, eix -u will match as if LOCAL_PORTAGE_CONFIG=true/false."));

AddOption(BOOLEAN, "RECOMMEND_LOCAL_MODE",
	"", P_("RECOMMEND_LOCAL_MODE",
	"If +/-, recommendations for up- or downgrade will act as if\n"
	"LOCAL_PORTAGE_CONFIG=true/false."));

AddOption(BOOLEAN, "RECURSIVE_SETS",
	"true", P_("RECURSIVE_SETS",
	"Are packages/sets in included sets part of the parent set?"));
#endif

#if (DEFAULT_PART == 2)

AddOption(BOOLEAN, "UPGRADE_TO_HIGHEST_SLOT",
	"true", P_("UPGRADE_TO_HIGHEST_SLOT",
	"If true, upgrade tests succeed for installed packages with new higher slots.\n"
	"Use the files SLOT_UPGRADE_FORBID or SLOT_UPGRADE_ALLOW, respectively,\n"
	"to specify exceptions."));

AddOption(BOOLEAN, "PRINT_ALWAYS",
	"false", P_("PRINT_ALWAYS",
	"This variable is only used for delayed substitution.\n"
	"It defines whether all information lines are printed (even if empty)."));

AddOption(BOOLEAN, "COUNT_ONLY_PRINTED",
	"true", P_("COUNT_ONLY_PRINTED",
	"If false, count independently of whether the matches are printed."));

AddOption(BOOLEAN, "PRINT_BUGS",
	"true", P_("PRINT_BUGS",
	"This variable is only used for delayed substitution.\n"
	"It defines whether a bug reference is printed in verbose format."));

AddOption(BOOLEAN, "DIFF_PRINT_INSTALLED",
	"true", P_("DIFF_PRINT_INSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines whether eix-diff will output installed versions."));

AddOption(STRING, "PRINT_SETNAMES",
	"%{?ALL_SETNAMES}all%{}setnames", P_("PRINT_SETNAMES",
	"This variable is only used for delayed substitution.\n"
	"It is the command used to print the package set names."));

AddOption(BOOLEAN, "PRINT_SLOTS",
	"true", P_("PRINT_SLOTS",
	"This variable is only used for delayed substitution.\n"
	"If false, no slot information is printed."));

AddOption(BOOLEAN, "EIX_PRINT_IUSE",
	"true", P_("EIX_PRINT_IUSE",
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"If false, no IUSE data is printed for eix. See also VERSION_IUSE_*"));

AddOption(BOOLEAN, "DIFF_PRINT_IUSE",
	"false", P_("DIFF_PRINT_IUSE",
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"If false, no IUSE data is printed for eix-diff."));

AddOption(BOOLEAN, "UPDATE_PRINT_IUSE",
	"false", P_("UPDATE_PRINT_IUSE",
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"It is unused by default."));

AddOption(BOOLEAN, "DROP_PRINT_IUSE",
	"false", P_("DROP_PRINT_IUSE",
	"This variable is only used for delayed substitution in PRINT_IUSE.\n"
	"It is unused by default."));

AddOption(BOOLEAN, "PRINT_IUSE",
	"%{*PRINT_IUSE}", P_("PRINT_IUSE",
	"This variable is only used for delayed substitution.\n"
	"If false, no IUSE data is printed."));

AddOption(BOOLEAN, "VERSION_IUSE_NORMAL",
	"true", P_("VERSION_IUSE_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs IUSE for each version."));

AddOption(BOOLEAN, "VERSION_IUSE_VERBOSE",
	"true", P_("VERSION_IUSE_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs IUSE for each version."));

AddOption(BOOLEAN, "PRINT_KEYWORDS",
	"true", P_("PRINT_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"If false, KEYWORDS are never output."));

AddOption(BOOLEAN, "VERSION_KEYWORDS_NORMAL",
	"false", P_("VERSION_KEYWORDS_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs KEYWORDS."));

AddOption(BOOLEAN, "VERSION_KEYWORDS_VERBOSE",
	"true", P_("VERSION_KEYWORDS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs KEYWORDS."));

AddOption(BOOLEAN, "PRINT_EAPI",
	"true", P_("PRINT_EAPI",
	"This variable is only used for delayed substitution.\n"
	"If false, EAPI is never output."));

AddOption(BOOLEAN, "PRINT_MASKREASONS",
	"true", P_("PRINT_MASKREASONS",
	"This variable is only used for delayed substitution.\n"
	"If false, mask reasons are never output."));

AddOption(BOOLEAN, "MASKREASONS_NORMAL",
	"true", P_("MASKREASONS_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs mask reasons."));

AddOption(BOOLEAN, "MASKREASONS_VERBOSE",
	"true", P_("MASKREASONS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs mask reasons."));

AddOption(BOOLEAN, "VERSION_DEPS_NORMAL",
	"false", P_("VERSION_DEPS_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines (nonverbose) outputs {,R,P}DEPEND."));

AddOption(BOOLEAN, "VERSION_DEPS_VERBOSE",
	"true", P_("VERSION_DEPS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"If true, eix --versionlines --verbose outputs {,R,P}DEPEND."));

AddOption(BOOLEAN, "USE_EFFECTIVE_KEYWORDS",
	"false", P_("USE_EFFECTIVE_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"Print effective keywords (modified by profile) instead of KEYWORDS."));

AddOption(BOOLEAN, "ALL_SETNAMES",
	"true", P_("ALL_SETNAMES",
	"This variable is only used for delayed substitution.\n"
	"It defines whether to include \"system\" in package sets output."));

AddOption(BOOLEAN, "NOBEST_COMPACT",
	"true", P_("NOBEST_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"If true, compact format prints no version if none is installable."));

AddOption(BOOLEAN, "NOBEST_CHANGE",
	"true", P_("NOBEST_CHANGE",
	"This variable is only used for delayed substitution.\n"
	"If true, compact format prints no versions if all installable vanished."));

AddOption(BOOLEAN, "DIFF_NOBEST",
	"false", P_("DIFF_NOBEST",
	"This variable is only used for delayed substitution.\n"
	"If true, eix-diff prints no version if none is installable."));

AddOption(BOOLEAN, "DIFF_NOBEST_CHANGE",
	"false", P_("DIFF_NOBEST_CHANGE",
	"This variable is only used for delayed substitution.\n"
	"If true, eix-diff prints no versions if all installable vanished."));

AddOption(STRING, "TERM",
	"xterm", P_("TERM",
	"The current terminal. Usually this is set by the environment variable."));

AddOption(STRING, "TERM_STATUSLINE",
	"xterm screen tmux rxvt aterm konsole gnome Eterm eterm kterm interix", P_("TERM_STATUSLINE",
	"If the beginning of TERM matches a word of this space-separated list,\n"
	"it is assumed that the terminal supports a status line."));

AddOption(STRING, "TERM_SOFTSTATUSLINE",
	"screen tmux", P_("TERM_SOFTSTATUSLINE",
	"If the beginning of TERM matches a word of this space-separated list, and\n"
	"if a status line is active, also a soft status line will be output."));

AddOption(STRING, "EXIT_STATUSLINE",
	"", P_("EXIT_STATUSLINE",
	"If this is nonempty, it is used as the exit statusline.\n"
	"An optional leading space in this string is ignored."));

AddOption(BOOLEAN, "RESET_ALL_LINES",
	"true", P_("RESET_ALL_LINES",
	"This variable is only used for delayed substitution.\n"
	"It decides whether background colors are reset on every newline.\n"
	"This may be desired if you set BG? to a value differrent than \"none\"."));

AddOption(STRING, "BG0",
	"none", P_("BG0",
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 0. Use \"none\" for no change."));

AddOption(STRING, "BG1",
	"black", P_("BG1",
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 1. Use \"none\" for no change."));

AddOption(STRING, "BG2",
	"none", P_("BG2",
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 2. Use \"none\" for no change."));

AddOption(STRING, "BG3",
	"white", P_("BG3",
	"This variable is only used for delayed substitution.\n"
	"It is the background color for color scheme 3. Use \"none\" for no change."));

AddOption(STRING, "BG0S",
	"%{!SOLARIZED}%{BG0}%{else}none%{}", P_("BG0S",
	"This variable is only used for delayed substitution.\n"
	"It is used instead of BG0 to ignore BG0 if SOLARIZED is set."));

AddOption(STRING, "TERM_ALT1",
	"256 [aeEkx]term konsole gnome putty %{TERM_ALT1_ADD}", P_("TERM_ALT1",
	"This is a list of regular expressions; if one of it matches TERM, then\n"
	"COLORSCHEME1 is used instead of COLORSCHEME0. The intention is that the\n"
	"specified terminals default to 256 colors."));

AddOption(STRING, "TERM_ALT1_ADD",
	"", P_("TERM_ALT1_ADD",
	"This variable is only used for delayed substitution of TERM_ALT1."));

AddOption(STRING, "TERM_ALT2",
	"88 rxvt-unicode[^2]*$ %{TERM_ALT2_ADD}", P_("TERM_ALT2",
	"This is a list of regular expressions; if one of it matches TERM, then\n"
	"COLORSCHEME2 is used instead of COLORSCHEME0. The intention is that the\n"
	"specified terminals default to 88 colors."));

AddOption(STRING, "TERM_ALT2_ADD",
	"", P_("TERM_ALT2_ADD",
	"This variable is only used for delayed substitution of TERM_ALT2."));

AddOption(STRING, "TERM_ALT3",
	"", P_("TERM_ALT3",
	"This is a list of regular expressions; if one of it matches TERM, then\n"
	"COLORSCHEME3 is used instead of COLORSCHEME0. The intention is that\n"
	"these terminals are exceptions made on user request."));

AddOption(STRING, "DARK",
	"auto", P_("DARK",
	"The value \"true\" or \"false\" means that the first or second entry of the\n"
	"COLORSCHEME* variables is used (meaning black or white background, resp.).\n"
	"The value \"auto\" means a heuristic based on TERM_DARK and COLORFGBG_DARK."));

AddOption(STRING, "TERM_DARK",
	"linux true cygwin true putty true true*",
	/* xgettext: no-space-ellipsis-check */
	P_("TERM_DARK", "A list of pairs regexp1 darkmode1 regexp2 darkmode2 ... [default darkmode].\n"
	"If DARK=auto the first darkmode for which TERM matches regexp is assumed.\n"
	"In modes true* and false* also COLORGBGB is respected by COLORFGBG_DARK."));

AddOption(STRING, "COLORFGBG_DARK",
	";0?[02-689]$ ;1[0-46]$ ;[a-km-vxz][a-z]*$", P_("COLORFGBG_DARK",
	"This is a list of regular expressions.\n"
	"If modes true* and false* are active in TERM_DARK and COLORFGBG is nonempty,\n"
	"then the darkmode depends on whether COLORFGBG matches at least one entry."));

AddOption(STRING, "COLORFGBG",
	"", P_("COLORFGBG",
	"Usually, this is set in the environment by some terminals like rxvt.\n"
	"It is only used if DARK=auto, see COLORFGBG_BLACK."));

AddOption(BOOLEAN, "SOLARIZED",
	"false", P_("SOLARIZED",
	"This variable is only used in delayed substitution in COLORSCHEME?.\n"
	"Set it (e.g. in your environment) if your terminal is configured to use\n"
	"the solarized color scheme of Ethan Schoonover."));

AddOption(STRING, "COLORSCHEME0",
	"%{!SOLARIZED}0 2%{else}5%{}", P_("COLORSCHEME0",
	"This is one or two numbers. If TERM_ALT? does not match, this chooses the\n"
	"corresponding number in color specifications (starting from 0).\n"
	"If two numbers are specified, the choice depends on DARK.\n"
	"The intention of this variable is to select the color scheme used for\n"
	"8/16 color terminals."));

AddOption(STRING, "COLORSCHEME1",
	"%{!SOLARIZED}1 3%{else}4%{}", P_("COLORSCHEME1",
	"This is one or two numbers. If TERM_ALT1 matches, this chooses the\n"
	"corresponding number in color specifications (starting from 0).\n"
	"If two numbers are specified, the choice depends on DARK.\n"
	"The intention of this variable is to select the color scheme used for\n"
	"256 color terminals."));

AddOption(STRING, "COLORSCHEME2",
	"%{COLORSCHEME0}", P_("COLORSCHEME2",
	"This is one or two numbers. If TERM_ALT2 matches, this chooses the\n"
	"corresponding number in color specifications (starting from 0).\n"
	"If two numbers are specified, the choice depends on DARK.\n"
	"The intention of this variable is to select the color scheme used for\n"
	"88 color terminals."));

AddOption(STRING, "COLORSCHEME3",
	"%{COLORSCHEME0}", P_("COLORSCHEME3",
	"This is one or two numbers. If TERM_ALT3 matches, this chooses the\n"
	"corresponding number in color specifications (starting from 0).\n"
	"If two numbers are specified, the choice depends on DARK.\n"
	"The intention of this variable is to select the color scheme used for\n"
	"user-specified terminals."));

AddOption(BOOLEAN, "SORT_INST_USE_ALPHA",
	"false", P_("SORT_INST_USE_ALPHA",
	"If false, sort installed useflags by whether they are set."));

AddOption(STRING, "CHECK_INSTALLED_OVERLAYS",
	"repository", P_("CHECK_INSTALLED_OVERLAYS",
	"Allowed values are true/false/repository.\n"
	"If true, always check from which overlay a package version was installed.\n"
	"If false, only packages with versions in at least two trees are checked.\n"
	"The compromise - repository - checks at least always the repository files.\n"
	"Without a check, the assumed overlay may be wrong if the version was\n"
	"actually installed from a place not in the database anymore."));

AddOption(STRING, "MATCH_FIELD_DESCRIPTION",
	"[ ]", P_("MATCH_FIELD_DESCRIPTION",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for description."));

AddOption(STRING, "MATCH_FIELD_SET",
	"[@]", P_("MATCH_FIELD_SET",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for set."));

AddOption(STRING, "MATCH_FIELD_HOMEPAGE",
	"http.*:", P_("MATCH_FIELD_HOMEPAGE",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for homepage."));

AddOption(STRING, "MATCH_FIELD_CATEGORY_NAME",
	"/", P_("MATCH_FIELD_CATEGORY_NAME",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for category/name."));

AddOption(STRING, "MATCH_FIELD_LICENSE",
	"%{MATCH_FIELD_CATEGORY_NAME}", P_("MATCH_FIELD_LICENSE",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for license.\n"
	"If it is identical to MATCH_FIELD_CATEGORY_NAME, it is de facto disabled,\n"
	"because the latter takes precedence."));

AddOption(STRING, "MATCH_FIELD_DEPS",
	"[<>=!]", P_("MATCH_FIELD_DEPS",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for deps."));

AddOption(STRING, "MATCH_FIELD_EAPI",
	"%{MATCH_FIELD_CATEGORY_NAME}", P_("MATCH_FIELD_EAPI",
	"This variable is only used for delayed substitution.\n"
	"It is a regular expression used in DEFAULT_MATCH_FIELD for eapi.\n"
	"If it is identical to MATCH_FIELD_CATEGORY_NAME, it is de facto disabled,\n"
	"because the latter takes precedence."));

AddOption(STRING, "DEFAULT_MATCH_FIELD",
	"%{\\MATCH_FIELD_DESCRIPTION} description "
	"%{\\MATCH_FIELD_SET} set "
	"%{\\MATCH_FIELD_HOMEPAGE} homepage "
	"%{\\MATCH_FIELD_CATEGORY_NAME} category/name "
	"%{\\MATCH_FIELD_LICENSE} license "
	"%{\\MATCH_FIELD_DEPS} deps "
	"%{\\MATCH_FIELD_EAPI} eapi "
	"name", P_("DEFAULT_MATCH_FIELD",
	"This is a list of strings of the form regexp[ ]match_field.\n"
	"If regexp matches the search pattern, use match_field as the default.\n"
	"A fallback match_field may be specified as the last entry in the list.\n"
	"Admissible values for match_field are: name, category, category/name,\n"
	"description, license, homepage, set, slot, installed-slot, use\n"
	"with-use, without-use, error."));

AddOption(STRING, "MATCH_ALGORITHM_REGEX",
	"[][^$|()]|[.][*+?]", P_("MATCH_ALGORITHM_REGEX",
	"This variable is only used for delayed substitution.\n"
	"It is the criterion used in DEFAULT_MATCH_ALGORITHM for regex."));

AddOption(STRING, "MATCH_ALGORITHM_PATTERN",
	"^[*]|[^][().][*]|[][?]", P_("MATCH_ALGORITHM_PATTERN",
	"This variable is only used for delayed substitution.\n"
	"It is the criterion used in DEFAULT_MATCH_ALGORITHM for pattern."));

AddOption(STRING, "MATCH_ALGORITHM_SUBSTRING",
	"(!category-name)/", P_("MATCH_ALGORITHM_SUBSTRING",
	"This variable is only used for delayed substitution.\n"
	"It is the criterion used in DEFAULT_MATCH_ALGORITHM for substring."));

AddOption(STRING, "MATCH_ALGORITHM_EXACT",
	"(!use!set!eapi!installed-eapi!slot!installed-slot!use!with-use!without-use)", P_("MATCH_ALGORITHM_EXACT",
	"This variable is only used for delayed substitution.\n"
	"It is the criterion used in DEFAULT_MATCH_ALGORITHM for exact."));

AddOption(STRING, "MATCH_ALGORITHM_BEGIN",
	"^$", P_("MATCH_ALGORITHM_BEGIN",
	"This variable is only used for delayed substitution.\n"
	"It is the criterion used in DEFAULT_MATCH_ALGORITHM for begin."));

AddOption(STRING, "DEFAULT_MATCH_ALGORITHM",
	"%{\\MATCH_ALGORITHM_REGEX} regex "
	"%{\\MATCH_ALGORITHM_PATTERN} pattern "
	"%{\\MATCH_ALGORITHM_SUBSTRING} substring "
	"%{\\MATCH_ALGORITHM_EXACT} exact "
	"%{\\MATCH_ALGORITHM_BEGIN} begin "
	"regex", P_("DEFAULT_MATCH_ALGORITHM",
	"This is a list of strings of the form (spec)regexp[ ]match_algorithm.\n"
	"If spec matches the match field(s) and regexp matches the search pattern,\n"
	"use match_algorithm as the default.\n"
	"A fallback match_algorithm may be specified as the last entry in the list.\n"
	"Admissible values for match_algorithm are: regex, pattern, substring,\n"
	"begin, end, exact, fuzzy, error."));

AddOption(BOOLEAN, "TEST_FOR_EMPTY",
	"true", P_("TEST_FOR_EMPTY",
	"Defines whether empty entries in /etc/portage/package.* are shown with -t."));

AddOption(BOOLEAN, "TEST_KEYWORDS",
	"true", P_("TEST_KEYWORDS",
	"Defines whether /etc/portage/package.accept_keywords is tested with -t."));

AddOption(BOOLEAN, "TEST_MASK",
	"true", P_("TEST_MASK",
	"Defines whether /etc/portage/package.mask is tested with -t."));

AddOption(BOOLEAN, "TEST_UNMASK",
	"true", P_("TEST_UNMASK",
	"Defines whether /etc/portage/package.unmask is tested with -t."));

AddOption(BOOLEAN, "TEST_USE",
	"true", P_("TEST_USE",
	"Defines whether /etc/portage/package.use is tested with -t."));

AddOption(BOOLEAN, "TEST_ENV",
	"true", P_("TEST_ENV",
	"Defines whether /etc/portage/package.env is tested with -t."));

AddOption(BOOLEAN, "TEST_LICENSE",
	"true", P_("TEST_LICENSE",
	"Defines whether /etc/portage/package.license is tested with -t."));

AddOption(BOOLEAN, "TEST_RESTRICT",
	"true", P_("TEST_RESTRICT",
	"Defines whether /etc/portage/package.accept_restrict is tested with -t."));

AddOption(BOOLEAN, "TEST_CFLAGS",
	"true", P_("TEST_CFLAGS",
	"Defines whether /etc/portage/package.cflags is tested with -t."));

AddOption(BOOLEAN, "TEST_REMOVED",
	"true", P_("TEST_REMOVED",
	"Defines whether removed packages are tested with -t."));

AddOption(BOOLEAN, "TEST_FOR_NONEXISTENT",
	"true", P_("TEST_FOR_NONEXISTENT",
	"Defines whether non-existing installed versions are positive for -T."));

AddOption(BOOLEAN, "NONEXISTENT_IF_OTHER_OVERLAY",
	"true", P_("NONEXISTENT_IF_OTHER_OVERLAY",
	"Defines whether versions are non-existent for TEST_FOR_NONEXISTENT\n"
	"if they come from a different overlay than the installed version."));

AddOption(BOOLEAN, "NONEXISTENT_IF_MASKED",
	"true", P_("NONEXISTENT_IF_MASKED",
	"Defines whether masked versions are non-existent for TEST_FOR_NONEXISTENT."));

AddOption(BOOLEAN, "TEST_FOR_REDUNDANCY",
	"true", P_("TEST_FOR_REDUNDANCY",
	"Defines whether redundant entries are positive for -T."));

AddOption(STRING, "ACCEPT_KEYWORDS_AS_ARCH",
	"full", P_("ACCEPT_KEYWORDS_AS_ARCH",
	"If full or true modify ARCH by ACCEPT_KEYWORDS.\n"
	"This determines which keywords are considered as ARCH or OTHERARCH.\n"
	"The value full also influences the original ARCH keywording."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE",
	"some", P_("REDUNDANT_IF_DOUBLE",
	"Applies if /etc/portage/package.accept_keywords lists the same keyword\n"
	"twice for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_LINE",
	"some", P_("REDUNDANT_IF_DOUBLE_LINE",
	"Applies if /etc/portage/package.accept_keywords has two lines for\n"
	"identical target."));

AddOption(STRING, "REDUNDANT_IF_MIXED",
	"false", P_("REDUNDANT_IF_MIXED",
	"Applies if /etc/portage/package.accept_keywords lists two different\n"
	"keywords, e.g. ~ARCH and -*, for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_WEAKER",
	"all-installed", P_("REDUNDANT_IF_WEAKER",
	"Applies if /etc/portage/package.accept_keywords lists a keywords which can\n"
	"be replaced by a weaker keyword, e.g. -* or ~OTHERARCH or OTHERARCH\n"
	"in place of ~ARCH, or ~OTHERARCH in place of OTHERARCH,\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_STRANGE",
	"some", P_("REDUNDANT_IF_STRANGE",
	"Applies if /etc/portage/package.accept_keywords lists a strange keyword\n"
	"e.g. UNKNOWNARCH (unknown to the .ebuild and ARCH) or -OTHERARCH,\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_NO_CHANGE",
	"all-installed", P_("REDUNDANT_IF_NO_CHANGE",
	"Applies if /etc/portage/package.accept_keywords provides keywords which do\n"
	"not change the availability keywords status for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_MASK_NO_CHANGE",
	"all-uninstalled", P_("REDUNDANT_IF_MASK_NO_CHANGE",
	"Applies if /etc/portage/package.mask contains entries\n"
	"which do not change the mask status for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_UNMASK_NO_CHANGE",
	"all-installed", P_("REDUNDANT_IF_UNMASK_NO_CHANGE",
	"Applies if /etc/portage/package.unmask contains entries\n"
	"which do not change the mask status for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_MASKED",
	"some", P_("REDUNDANT_IF_DOUBLE_MASKED",
	"Applies if /etc/portage/package.mask matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_UNMASKED",
	"some", P_("REDUNDANT_IF_DOUBLE_UNMASKED",
	"Applies if /etc/portage/package.unmask matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_USE",
	"false", P_("REDUNDANT_IF_DOUBLE_USE",
	"Applies if /etc/portage/package.use matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_ENV",
	"false", P_("REDUNDANT_IF_DOUBLE_ENV",
	"Applies if /etc/portage/package.env matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_LICENSE",
	"some", P_("REDUNDANT_IF_DOUBLE_LICENSE",
	"Applies if /etc/portage/package.license matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_RESTRICT",
	"some", P_("REDUNDANT_IF_DOUBLE_RESTRICT",
	"Applies if /etc/portage/package.accept_restrict matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_DOUBLE_CFLAGS",
	"false", P_("REDUNDANT_IF_DOUBLE_CFLAGS",
	"Applies if /etc/portage/package.cflags matches twice\n"
	"for the versions in question."));

AddOption(STRING, "REDUNDANT_IF_IN_KEYWORDS",
	"-some", P_("REDUNDANT_IF_IN_KEYWORDS",
	"Applies if /etc/portage/package.accept_keywords contains a matching entry."));

AddOption(STRING, "REDUNDANT_IF_IN_MASK",
	"-some", P_("REDUNDANT_IF_IN_MASK",
	"Applies if /etc/portage/package.mask matches."));

AddOption(STRING, "REDUNDANT_IF_IN_UNMASK",
	"-some", P_("REDUNDANT_IF_IN_UNMASK",
	"Applies if /etc/portage/package.unmask matches."));

AddOption(STRING, "REDUNDANT_IF_IN_USE",
	"false", P_("REDUNDANT_IF_IN_USE",
	"Applies if /etc/portage/package.use matches."));

AddOption(STRING, "REDUNDANT_IF_IN_ENV",
	"false", P_("REDUNDANT_IF_IN_ENV",
	"Applies if /etc/portage/package.env matches."));

AddOption(STRING, "REDUNDANT_IF_IN_LICENSE",
	"-some", P_("REDUNDANT_IF_IN_LICENSE",
	"Applies if /etc/portage/package.license matches."));

AddOption(STRING, "REDUNDANT_IF_IN_RESTRICT",
	"-some", P_("REDUNDANT_IF_IN_RESTRICT",
	"Applies if /etc/portage/package.accept_restrict matches."));

AddOption(STRING, "REDUNDANT_IF_IN_CFLAGS",
	"false", P_("REDUNDANT_IF_IN_CFLAGS",
	"Applies if /etc/portage/package.cflags matches."));

AddOption(STRING, "EIXCFGDIR",
	"%{PORTAGE_CONFIGROOT}/etc/portage", P_("EIXCFGDIR",
	"This variable is only used for delayed substitution.\n"
	"It is the directory where eix searches for its package.*.*/sets.eix files."));

AddOption(BOOLEAN, "SLOT_UPGRADE_FORBID",
	"%{\\EIXCFGDIR}/package.slot_upgrade_forbid", P_("SLOT_UPGRADE_FORBID",
	"If UPGRADE_TO_HIGHEST_SLOT=true, then packages listed in these files/dirs\n"
	"are treated as if UPGRADE_TO_HIGHEST_SLOT=false."));
#endif

#if (DEFAULT_PART == 3)

AddOption(BOOLEAN, "SLOT_UPGRADE_ALLOW",
	"%{\\EIXCFGDIR}/package.slot_upgrade_allow", P_("SLOT_UPGRADE_ALLOW",
	"If UPGRADE_TO_HIGHEST_SLOT=false, then packages listed in these files/dirs\n"
	"are treated as if UPGRADE_TO_HIGHEST_SLOT=true."));

AddOption(STRING, "KEYWORDS_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.accept_keywords.nonexistent "
	"%{\\EIXCFGDIR}/package.keywords.nonexistent", P_("KEYWORDS_NONEXISTENT",
	"Entries listed in these files/dirs are excluded for -t TEST_KEYWORDS."));

AddOption(STRING, "MASK_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.mask.nonexistent", P_("MASK_NONEXISTENT",
	"Entries listed in these files/dirs are excluded for -t TEST_MASK."));

AddOption(STRING, "UNMASK_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.unmask.nonexistent", P_("UNMASK_NONEXISTENT",
	"Entries listed in these files/dirs are excluded for -t TEST_UNMASK."));

AddOption(STRING, "USE_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.use.nonexistent", P_("USE_NONEXISTENT",
	"Entries listed in these files/dire are excluded for -t TEST_USE."));

AddOption(STRING, "ENV_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.env.nonexistent", P_("ENV_NONEXISTENT",
	"Entries listed in these files/dire are excluded for -t TEST_ENV."));

AddOption(STRING, "LICENSE_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.license.nonexistent", P_("LICENSE_NONEXISTENT",
	"Entries listed in these files/dirs are excluded for -t TEST_LICENSE."));

AddOption(STRING, "RESTRICT_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.accept_restrict.nonexistent", P_("RESTRICT_NONEXISTENT",
	"Entries listed in these files/dirs are excluded for -t TEST_RESTRICT."));

AddOption(STRING, "CFLAGS_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.cflags.nonexistent", P_("CFLAGS_NONEXISTENT",
	"Entries listed in these files/dirs are excluded for -t TEST_CFLAGS."));

AddOption(STRING, "INSTALLED_NONEXISTENT",
	"%{\\EIXCFGDIR}/package.installed.nonexistent", P_("INSTALLED_NONEXISTENT",
	"Packages listed in these files/dirs are excluded for -t TEST_REMOVED."));

AddOption(STRING, "ADD_PACKAGE_NOWARN",
	"", P_("ADD_PACKAGE_NOWARN",
	"This variable is only used for delayed substitution in PACKAGE_NOWARN.\n"
	"It is meant to be a local addition to PACKAGE_NOWARN."));

AddOption(BOOLEAN, "OBSOLETE_NOWARN",
	"false", P_("OBSOLETE_NOWARN",
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
	"%{\\EIXCFGDIR}/package.accept_restrict.nowarn "
	"%{\\EIXCFGDIR}/package.cflags.nowarn "
	"%{\\EIXCFGDIR}/package.installed.nowarn", P_("OBSOLETE_PACKAGE_NOWARN",
	"This variable is used for delayed substitution in PACKAGE_NOWARN if the\n"
	"variable OBSOLETE_NOWARN is set."));

AddOption(STRING, "PACKAGE_NOWARN",
	"%{\\EIXCFGDIR}/package.nowarn "
	"%{?OBSOLETE_NOWARN}"
		"%{OBSOLETE_PACKAGE_NOWARN} "
	"%{}"
	"%{ADD_PACKAGE_NOWARN}", P_("PACKAGE_NOWARN",
	"This file/directory contains exceptions for -T tests."));

AddOption(STRING, "EIX_LOCAL_SETS_ADD",
	"", P_("EIX_LOCAL_SETS_ADD",
	"This variable is only used for delayed substitution.\n"
	"It specifies directories for EIX_LOCAL_SETS."));

AddOption(STRING, "EIX_LOCAL_SETS",
	"%{EIX_LOCAL_SETS_ADD} "
	"%{\\EIXCFGDIR}/sets.eix "
	"%{\\PORTAGE_CONFIGROOT}/etc/portage/sets "
	"*/sets "
	"sets", P_("EIX_LOCAL_SETS",
	"This is a space-separated list of directories containing set definitions."));

AddOption(STRING, "EAPI_REGEX",
	"[0-9]+", P_("EAPI_REGEX",
	"This regular expression describes the recognized EAPIs in .ebuild suffixes.\n"
	"You might need to modify it according to the installed portage version.\n"
	"Leave it empty if EAPI-suffixed ebuilds (GLEP 55) should be ignored."));

AddOption(STRING, "COLOR_RESET",
	";%{BG0S}|;%{BG1}|;%{BG2}|;%{BG3}", P_("COLOR_RESET",
	"This variable is only used for delayed substitution.\n"
	"It is the color used for default colors."));

AddOption(STRING, "COLOR_RESET_BG",
	"none;%{BG0S}|none;%{BG1}|none;%{BG2}|none;%{BG3}", P_("COLOR_RESET_BG",
	"This variable is only used for delayed substitution.\n"
	"It is the color used for default colors."));

AddOption(STRING, "COLOR_NAME",
	",1;%{BG0S}|253,1;%{BG1}|,1;%{BG2}|232;%{BG3}", P_("COLOR_NAME",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the name of packages."));

AddOption(STRING, "COLOR_WORLD",
	"green,1;%{BG0S}|47,1;%{BG1}|green,1;%{BG2}|22,1;%{BG3}|cyan,1|cyan,1", P_("COLOR_WORLD",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the name of world packages."));

AddOption(STRING, "COLOR_WORLD_SETS",
	"yellow,1;%{BG0S}|214,1;%{BG1}|blue,1;%{BG2}|33,1;%{BG3}|purple,1|purple,1", P_("COLOR_WORLD_SETS",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the name of world sets packages."));

AddOption(STRING, "COLOR_CATEGORY",
	"%{COLOR_NORMAL}", P_("COLOR_CATEGORY",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of packages."));

AddOption(STRING, "COLOR_CATEGORY_SYSTEM",
	"yellow;%{BG0S}|154,1;%{BG1}|blue;%{BG2}|69,1;%{BG3}", P_("COLOR_CATEGORY_SYSTEM",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of system packages."));

AddOption(STRING, "COLOR_CATEGORY_PROFILE",
	"cyan;%{BG0S}|81,1;%{BG1}|cyan;%{BG2}|57,1;%{BG3}", P_("COLOR_CATEGORY_PROFILE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of profile packages."));

AddOption(STRING, "COLOR_CATEGORY_WORLD",
	"%{COLOR_WORLD}", P_("COLOR_CATEGORY_WORLD",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the category of world packages."));

AddOption(STRING, "COLOR_CATEGORY_WORLD_SETS",
	"%{COLOR_WORLD_SETS}", P_("COLOR_CATEGORY_WORLD_SETS",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the category of world sets packages."));

AddOption(STRING, "COLOR_UPGRADE_TEXT",
	"cyan,1;%{BG0S}|87,1;%{BG1}|blue,1;%{BG2}|21,1;%{BG3}|39,1", P_("COLOR_UPGRADE_TEXT",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation texts."));

AddOption(STRING, "COLOR_DOWNGRADE_TEXT",
	"blue,1;%{BG0S}|135,1;%{BG1}|purple,1;%{BG2}|89,1;%{BG3}|135,1", P_("COLOR_DOWNGRADE_TEXT",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation texts."));

AddOption(STRING, "COLOR_UPGRADE",
	"%{COLOR_UPGRADE_TEXT;inverse}", P_("COLOR_UPGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing upgrade recommendation tags."));

AddOption(STRING, "COLOR_DOWNGRADE",
	"%{COLOR_DOWNGRADE_TEXT;inverse}", P_("COLOR_DOWNGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing downgrade recommendation tags."));

AddOption(STRING, "DIFF_COLOR_UNINST_STABILIZE",
	"%{COLOR_STABLE,1}", P_("DIFF_COLOR_UNINST_STABILIZE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for uninstalled packages\n"
	"which have gained a stable version."));

AddOption(STRING, "DIFF_COLOR_INST_STABILIZE",
	"%{DIFF_COLOR_UNINST_STABILIZE;inverse}", P_("DIFF_COLOR_INST_STABILIZE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing tags for installed packages\n"
	"which have gained a stable version."));

AddOption(STRING, "DIFF_COLOR_BETTER",
	"%{DIFF_COLOR_CHANGED}", P_("DIFF_COLOR_BETTER",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"better version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_WORSE",
	"%{DIFF_COLOR_DELETE}", P_("DIFF_COLOR_WORSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"worse version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_NEW_TAG",
	"%{DIFF_COLOR_NEW}", P_("DIFF_COLOR_NEW_TAG",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" tags (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_NEW",
	"%{COLOR_STABLE,1}", P_("DIFF_COLOR_NEW",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"new package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_DELETE",
	"red,1;%{BG0S}|197,1;%{BG1}|red,1;%{BG2}|197,1;%{BG3}", P_("DIFF_COLOR_DELETE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"deleted package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_COLOR_CHANGED",
	"yellow;%{BG0S}|226;%{BG1}|blue;%{BG2}|24;%{BG3}", P_("DIFF_COLOR_CHANGED",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for \"changed package\" separators (eix-diff)."));

AddOption(STRING, "COLOR_INST_TAG",
	"%{COLOR_STABLE,1;inverse}", P_("COLOR_INST_TAG",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging installed packages."));

AddOption(STRING, "COLOR_UNINST_TAG",
	"%{COLOR_STABLE}", P_("COLOR_UNINST_TAG",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for tagging uninstalled packages."));

AddOption(STRING, "COLOR_DATE",
	"purple,1;%{BG0S}|166;%{BG1}|purple,1;%{BG2}|130,1;%{BG3}", P_("COLOR_DATE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the date."));

AddOption(STRING, "COLOR_EAPI",
	"green,1;%{BG0S}|80;%{BG1}|green;%{BG2}|80;%{BG3}", P_("COLOR_EAPI",
	"This variable is only used for delayed substitution.\n"
	"It defines the color of the EAPI output."));

AddOption(STRING, "COLOR_DEPEND",
	"none|248;%{BG1}|none|241,1;%{BG3}", P_("COLOR_DEPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the color of the DEPEND output."));

AddOption(STRING, "COLOR_DEPEND_END",
	"none|;%{BG1}|none|;%{BG3}", P_("COLOR_DEPEND_END",
	"This variable is only used for delayed substitution.\n"
	"It defines the end of color of the DEPEND output."));

AddOption(STRING, "COLOR_NORMAL",
	";%{BG0S}|252;%{BG1}|,1;%{BG2}|237,1;%{BG3}", P_("COLOR_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing normal texts."));

AddOption(STRING, "COLOR_NORMAL_END",
	"none|;%{BG1}|none|;%{BG3}", P_("COLOR_NORMAL_END",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing end of normal texts."));

AddOption(STRING, "COLOR_MASKREASONS",
	"red;%{BG0S}|177;%{BG1}|red;%{BG2}|201;%{BG3}", P_("COLOR_MASKREASONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the color of the mask reasons output."));

AddOption(STRING, "COLOR_SET_USE",
	"red,1;%{BG0S}|125,1;%{BG1}|red,1;%{BG2}|125,1;%{BG3}", P_("COLOR_SET_USE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the set useflags."));

AddOption(STRING, "COLOR_UNSET_USE",
	"blue,1;%{BG0S}|33;%{BG1}|blue,1;%{BG2}|17;%{BG3}", P_("COLOR_UNSET_USE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the unset useflags."));

AddOption(STRING, "COLOR_VERSION_IUSE",
	";%{BG0S}|168;%{BG1}|,1;%{BG2}|168,1;%{BG3}|yellow,1|yellow,1", P_("COLOR_VERSION_IUSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing IUSE for available versions."));

AddOption(STRING, "COLOR_COLL_IUSE",
	";%{BG0S}|38;%{BG1}|,1;%{BG2}|61,1;%{BG3}", P_("COLOR_COLL_IUSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing collected IUSE for packages."));

AddOption(STRING, "COLOR_REQUIRED_USE",
	";%{BG0S}|165;%{BG1}|,1;%{BG2}|165,1;%{BG3}|yellow,1|yellow,1", P_("COLOR_REQUIRED_USE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing REQUIRED_USE."));

AddOption(STRING, "COLOR_USE_EXPAND_START",
	";%{BG0S}|115;%{BG1}|;%{BG2}|95,1;%{BG3}", P_("COLOR_USE_EXPAND_START",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing USE_EXPAND variables."));

AddOption(STRING, "COLOR_USE_EXPAND_END",
	"none|;%{BG1}|none|;%{BG3}", P_("COLOR_USE_EXPAND_END",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing end of USE_EXPAND variables."));

AddOption(STRING, "COLOR_USE_COLL",
	"yellow,1;%{BG0S}|252;%{BG1}|blue;%{BG2}|237,1;%{BG3}", P_("COLOR_USE_COLL",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the braces around collected USE."));

AddOption(STRING, "COLOR_INST_VERSION",
	"black;blue|33,1;%{BG1}|black;green|30,1;%{BG3}|33,1", P_("COLOR_INST_VERSION",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the version of installed packages."));

AddOption(STRING, "COLOR_TITLE",
	"green;%{BG0S}|34;%{BG1}|;%{BG2}|240,1;%{BG3}", P_("COLOR_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for packages."));

AddOption(STRING, "COLOR_INST_TITLE",
	"cyan;%{BG0S}|67;%{BG1}|cyan,1;%{BG2}|67,1;%{BG3}", P_("COLOR_INST_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for installed versions."));

AddOption(STRING, "COLOR_AVAILABLE_TITLE",
	"purple;%{BG0S}|70;%{BG1}|purple;%{BG2}|65,1;%{BG3}", P_("COLOR_AVAILABLE_TITLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for the title texts for available versions."));

AddOption(STRING, "COLOR_MARKED_VERSION",
	"%{COLOR_MARKED_NAME}", P_("COLOR_MARKED_VERSION",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked version of a packages."));

AddOption(STRING, "COLOR_PACKAGESETS",
	"%{COLOR_WORLD_SETS}", P_("COLOR_PACKAGESETS",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing the package sets."));

AddOption(STRING, "COLOR_MARKED_NAME",
	"red,1;%{BG0S};%{MARK_VERSIONS}|207,1;%{BG1};%{MARK_VERSIONS}|red,1;%{BG2};%{MARK_VERSIONS}|164,1;%{BG3};%{MARK_VERSIONS}", P_("COLOR_MARKED_NAME",
	"This variable is only used for delayed substitution.\n"
	"It defines the color used for printing a marked package name."));

AddOption(STRING, "COLOR_OVERLAYKEY",
	"cyan;%{BG0S}|87;%{BG1}|cyan,1;%{BG2}|26,1;%{BG3}|39", P_("COLOR_OVERLAYKEY",
	"Color for the overlaykey in version listings."));

AddOption(STRING, "COLOR_VIRTUALKEY",
	"purple;%{BG0S}|170;%{BG1}|purple,1;%{BG2}|92,1;%{BG3}|170", P_("COLOR_VIRTUALKEY",
	"Color for the overlaykey for virtual overlays in version listings."));

AddOption(STRING, "COLOR_KEYEND",
	"%{COLOR_RESET}", P_("COLOR_KEYEND",
	"Color after printing an overlay key."));

AddOption(STRING, "COLOR_OVERLAYNAME",
	"%{COLOR_NORMAL}", P_("COLOR_OVERLAYNAME",
	"Color for printing an overlay name."));

AddOption(STRING, "COLOR_OVERLAYNAMEEND",
	"", P_("COLOR_OVERLAYNAMEEND",
	"Color after printing an overlay name."));

AddOption(STRING, "COLOR_NUMBERTEXT",
	"%{COLOR_NORMAL}", P_("COLOR_NUMBERTEXT",
	"Color for printing the number of packages."));

AddOption(STRING, "COLOR_NUMBERTEXTEND",
	"", P_("COLOR_NUMBERTEXTEND",
	"Color after printing the number of packages."));

AddOption(STRING, "COLOR_SLOTS",
	"red,1;%{BG0S}|166,1;%{BG1}|red,1;%{BG2}|166,1;%{BG3}", P_("COLOR_SLOTS",
	"Color for slots. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_BINARY",
	"blue,1;%{BG0S}|39,1;%{BG1}|blue,1;%{BG2}|39,1;%{BG3}|39,1", P_("COLOR_BINARY",
	"Color for braces for binaries. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_TBZ",
	"%{COLOR_BINARY}", P_("COLOR_TBZ",
	"Color for tag for *.tbz2. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_PAK",
	"blue,1;%{BG0S}|59,1;%{BG1}|blue,1;%{BG2}|59,1;%{BG3}|59,1", P_("COLOR_PAK",
	"Color for tag for *.xpak. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_PAKCOUNT",
	"purple,1;%{BG0S}|97,1;%{BG1}|blue,1;%{BG2}|97,1;%{BG3}|97,1", P_("COLOR_PAKCOUNT",
	"Color for number of *.xpak. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_RESTRICT",
	"red;%{BG0S}|99;%{BG1}|red;%{BG2}|53;%{BG3}|99", P_("COLOR_RESTRICT",
	"Color for the restriction tags. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_PROPERTIES",
	"cyan;%{BG0S}|143;%{BG1}|cyan,1;%{BG2}|57;%{BG3}|214", P_("COLOR_PROPERTIES",
	"Color for the properties tags. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_KEYWORDS",
	"cyan;%{BG0S}|73;%{BG1}|cyan,1;%{BG2}|26;%{BG3}", P_("COLOR_KEYWORDS",
	"Color for keywords. This is only used for delayed substitution."));

AddOption(STRING, "COLOR_KEYWORDSS",
	"%{COLOR_KEYWORDS}", P_("COLOR_KEYWORDSS",
	"Color for keywords*. This is only used for delayed substitution."));

AddOption(STRING, "MARK_INSTALLED",
	"inverse", P_("MARK_INSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines how installed versions are marked."));

AddOption(STRING, "MARK_UPGRADE",
	"bold", P_("MARK_UPGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines how upgrade candidate versions are marked."));

AddOption(STRING, "MARK_VERSIONS",
	"underline", P_("MARK_VERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines how the package versions passed with --pipe are marked."));

AddOption(BOOLEAN, "NOCOLOR",
	"false", P_("NOCOLOR",
	"This variable is only used for delayed substitution.\n"
	"It is the default for NO{COLORS,STATUSLINE,PERCENTAGE}."));

AddOption(STRING, "CHAR_UPGRADE", P_("CHAR_UPGRADE",
	"U"), P_("CHAR_UPGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for upgrade recommendations."));

AddOption(STRING, "CHAR_DOWNGRADE", P_("CHAR_DOWNGRADE",
	"?"), P_("CHAR_DOWNGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for downgrade recommendations."));

AddOption(STRING, "CHAR_INSTALLED", P_("CHAR_INSTALLED",
	"I"), P_("CHAR_INSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for installed packages."));

AddOption(STRING, "CHAR_UNINSTALLED",
	"%{DIFF_CHAR_NEW}", P_("CHAR_UNINSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for uninstalled packages."));

AddOption(STRING, "DIFF_CHAR_UNINST_STABILIZE",
	"%{DIFF_CHAR_INST_STABILIZE}", P_("DIFF_CHAR_UNINST_STABILIZE",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for uninstalled packages which have gained\n"
	"a stable version."));

AddOption(STRING, "DIFF_CHAR_INST_STABILIZE",
	"*", P_("DIFF_CHAR_INST_STABILIZE",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for installed packages which have gained\n"
	"a stable version."));

AddOption(STRING, "DIFF_CHAR_NEW", P_("DIFF_CHAR_NEW",
	"N"), P_("DIFF_CHAR_NEW",
	"This variable is only used for delayed substitution.\n"
	"It defines the character printed for new packages (eix-diff)."));

AddOption(STRING, "DIFF_CHAR_BETTER",
	">", P_( "DIFF_CHAR_BETTER",
	"This variable is only used for delayed substitution.\n"
	"It defines the character used for \"better version\" tags (eix-diff)."));

AddOption(STRING, "DIFF_CHAR_WORSE",
	"\\<", P_("DIFF_CHAR_WORSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the character used for \"worse version\" tags (eix-diff)."));

AddOption(STRING, "TAG_UPGRADE",
	"(%{COLOR_UPGRADE})%{CHAR_UPGRADE}(%{COLOR_RESET})", P_("TAG_UPGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for upgrade recommendations."));

AddOption(STRING, "TAG_DOWNGRADE",
	"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}(%{COLOR_RESET})", P_("TAG_DOWNGRADE",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for downgrade recommendations."));

AddOption(STRING, "TAG_INSTALLED",
	"(%{COLOR_INST_TAG})%{CHAR_INSTALLED}(%{COLOR_RESET})", P_("TAG_INSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for installed packages."));

AddOption(STRING, "TAG_UNINSTALLED",
	"(%{COLOR_UNINST_TAG})%{CHAR_UNINSTALLED}(%{COLOR_RESET})", P_("TAG_UNINSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for uninstalled packages."));

AddOption(STRING, "TAG_STABILIZE",
	"{installed}"
		"(%{DIFF_COLOR_INST_STABILIZE})%{DIFF_CHAR_INST_STABILIZE}"
	"{else}"
		"(%{DIFF_COLOR_UNINST_STABILIZE})%{DIFF_CHAR_UNINST_STABILIZE}"
	"{}(%{COLOR_RESET})", P_("TAG_STABILIZE",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag for packages which have gained a stable version."));

AddOption(STRING, "TAG_NEW",
	"(%{DIFF_COLOR_NEW_TAG})%{DIFF_CHAR_NEW}(%{COLOR_RESET})", P_("TAG_NEW",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag printed for new packages (eix-diff)."));
#endif

#if (DEFAULT_PART == 4)

AddOption(STRING, "TAG_BETTER",
	"(%{DIFF_COLOR_BETTER})%{DIFF_CHAR_BETTER}(%{COLOR_RESET})", P_("TAG_BETTER",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"better version\" (eix-diff)."));

AddOption(STRING, "TAG_WORSE",
	"(%{DIFF_COLOR_WORSE})%{DIFF_CHAR_WORSE}(%{COLOR_RESET})", P_("TAG_WORSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the tag used for \"worse version\" (eix-diff)."));

AddOption(STRING, "STRING_PLAIN_INSTALLED",
	"%{STRING_PLAIN_UNINSTALLED}", P_("STRING_PLAIN_INSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"plain\" tagging installed packages."));

AddOption(STRING, "STRING_PLAIN_UNINSTALLED",
	"*", P_("STRING_PLAIN_UNINSTALLED",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"plain\" tagging uninstalled packages."));

AddOption(STRING, "DIFF_STRING_NEW",
	">>", P_("DIFF_STRING_NEW",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"new package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_STRING_DELETE",
	"\\<\\<", P_("DIFF_STRING_DELETE",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"deleted package\" separators (eix-diff)."));

AddOption(STRING, "DIFF_STRING_CHANGED",
	"==", P_("DIFF_STRING_CHANGED",
	"This variable is only used for delayed substitution.\n"
	"It defines the string used for \"changed package\" separators (eix-diff)."));

AddOption(STRING, "FORMAT_NOBEST",
	"%{FORMAT_COLOR_MASKED}"
	"--"
	"(%{COLOR_RESET})", P_("FORMAT_NOBEST",
	"This variable is only used for delayed substitution.\n"
	"It defines what to print if no version number is printed."));

AddOption(STRING, "FORMAT_NOBEST_CHANGE",
	"%{FORMAT_COLOR_MASKED}"
	"??"
	"(%{COLOR_RESET})", P_("FORMAT_NOBEST_CHANGE",
	"This variable is only used for delayed substitution.\n"
	"It defines what to print after \"->\" if there is no installable."));

AddOption(STRING, "TAG_TBZ",
	"tbz2", P_("TAG_TBZ",
	"Tag for *.tbz2. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PAK",
	"xpak", P_("TAG_PAK",
	"Tag for *.xpak. This is only used for delayed substitution."));

AddOption(STRING, "TAG_MULTIPAK",
	":", P_("TAG_MULTIPAK",
	"Tag for multiple *.xpak. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_FETCH",
	"f", P_("TAG_RESTRICT_FETCH",
	"Tag for RESTRICT=fetch. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_MIRROR",
	"m", P_("TAG_RESTRICT_MIRROR",
	"Tag for RESTRICT=mirror. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_PRIMARYURI",
	"p", P_("TAG_RESTRICT_PRIMARYURI",
	"Tag for RESTRICT=primaryuri. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_BINCHECKS",
	"b", P_("TAG_RESTRICT_BINCHECKS",
	"Tag for RESTRICT=binchecks. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_STRIP",
	"s", P_("TAG_RESTRICT_STRIP",
	"Tag for RESTRICT=strip. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_TEST",
	"t", P_("TAG_RESTRICT_TEST",
	"Tag for RESTRICT=test. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_USERPRIV",
	"u", P_("TAG_RESTRICT_USERPRIV",
	"Tag for RESTRICT=userpriv. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_INSTALLSOURCES",
	"i", P_("TAG_RESTRICT_INSTALLSOURCES",
	"Tag for RESTRICT=installsources. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_BINDIST",
	"d", P_("TAG_RESTRICT_BINDIST",
	"Tag for RESTRICT=bindist. This is only used for delayed substitution."));

AddOption(STRING, "TAG_RESTRICT_PARALLEL",
	"P", P_("TAG_RESTRICT_PARALLEL",
	"Tag for RESTRICT=parallel. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_INTERACTIVE",
	"i", P_("TAG_PROPERTIES_INTERACTIVE",
	"Tag for PROPERTIES=interactive. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_LIVE",
	"l", P_("TAG_PROPERTIES_LIVE",
	"Tag for PROPERTIES=live. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_VIRTUAL",
	"v", P_("TAG_PROPERTIES_VIRTUAL",
	"Tag for PROPERTIES=virtual. This is only used for delayed substitution."));

AddOption(STRING, "TAG_PROPERTIES_SET",
	"s", P_("TAG_PROPERTIES_SET",
	"Tag for PROPERTIES=set. This is only used for delayed substitution."));

AddOption(STRING, "TAG_FOR_PROFILE",
	"[P]", P_("TAG_FOR_PROFILE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"profile masked\" versions."));

AddOption(STRING, "TAG_FOR_MASKED",
	"[M]", P_("TAG_FOR_MASKED",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"package.masked\" versions."));

AddOption(STRING, "TAG_FOR_EX_PROFILE",
	"{P}", P_("TAG_FOR_EX_PROFILE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally profile masked but unmasked\" versions."));

AddOption(STRING, "TAG_FOR_EX_MASKED",
	"{M}", P_("TAG_FOR_EX_MASKED",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally package.masked but unmasked\" versions."));

AddOption(STRING, "TAG_FOR_LOCALLY_MASKED",
	"[m]", P_("TAG_FOR_LOCALLY_MASKED",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"only locally masked\" versions."));

AddOption(STRING, "TAG_FOR_STABLE",
	"", P_("TAG_FOR_STABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"stable\" versions."));

AddOption(STRING, "TAG_FOR_UNSTABLE",
	"~", P_("TAG_FOR_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"unstable\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_ASTERISK",
	"-*", P_("TAG_FOR_MINUS_ASTERISK",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"-*\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_UNSTABLE",
	"-~*", P_("TAG_FOR_MINUS_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"-~*\" versions."));

AddOption(STRING, "TAG_FOR_MINUS_KEYWORD",
	"-", P_("TAG_FOR_MINUS_KEYWORD",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"-ARCH\" versions."));

AddOption(STRING, "TAG_FOR_ALIEN_STABLE",
	"*", P_("TAG_FOR_ALIEN_STABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"ALIENARCH\" versions."));

AddOption(STRING, "TAG_FOR_ALIEN_UNSTABLE",
	"~*", P_("TAG_FOR_ALIEN_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"~ALIENARCH\" versions."));

AddOption(STRING, "TAG_FOR_MISSING_KEYWORD",
	"**", P_("TAG_FOR_MISSING_KEYWORD",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"no keyword\" versions."));

AddOption(STRING, "TAG_FOR_EX_UNSTABLE",
	"(~)", P_("TAG_FOR_EX_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally unstable but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_ASTERISK",
	"(-*)", P_("TAG_FOR_EX_MINUS_ASTERISK",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally -* but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_UNSTABLE",
	"(-~*)", P_("TAG_FOR_EX_MINUS_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally -~* but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MINUS_KEYWORD",
	"(-)", P_("TAG_FOR_EX_MINUS_KEYWORD",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally -ARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_ALIEN_STABLE",
	"(*)", P_("TAG_FOR_EX_ALIEN_STABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally ALIENARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_ALIEN_UNSTABLE",
	"(~*)", P_("TAG_FOR_EX_ALIEN_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally ~ALIENARCH but now stable\" versions."));

AddOption(STRING, "TAG_FOR_EX_MISSING_KEYWORD",
	"(**)", P_("TAG_FOR_EX_MISSING_KEYWORD",
	"This variable is only used for delayed substitution.\n"
	"It is the tag for \"originally no keyword but now stable\" versions."));

AddOption(STRING, "VERSION_NEWLINE",
	"\\n", P_("VERSION_NEWLINE",
	"This variable is used by delayed substitution in version formatters.\n"
	"It prints a newline at the end of a version."));

AddOption(STRING, "NAMEVERSION",
	"<category>/<name>-<version>"
	"%{VERSION_NEWLINE}", P_("NAMEVERSION",
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <bestslotupgradeversions:NAMEVERSION>\n"
	"or <installedversions:NAMEVERION> or <availableversions:NAMEVERSION>."));

AddOption(STRING, "EQNAMEVERSION",
	"=<category>/<name>-<version>"
	"%{VERSION_NEWLINE}", P_("EQNAMEVERSION",
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <bestslotupgradeversions:EQNAMEVERSION>\n"
	"or <installedversions:EQNAMEVERION> or <availableversions:EQNAMEVERSION>."));

AddOption(STRING, "ANAMESLOT",
	"{slotlast}<category>/<name>{slots}:<slot>{}"
	"%{VERSION_NEWLINE}", P_("ANAMESLOT",
	"This variable is used as a version formatter.\n"
	"It is an example for usage as <availableversion:ANAMESLOT:ANAMESLOT>."));

AddOption(STRING, "ANAMEASLOT",
	"{slotlast}<category>/<name>:<slot>{!last}\\n{}{}"
	"%{VERSION_NEWLINE}", P_("ANAMEASLOT",
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <availableversion:ANAMEASLOT:ANAMEASLOT>."));

AddOption(STRING, "NAMESLOT",
	"<category>/<name>{slots}:<slot>{}%{VERSION_NEWLINE}", P_("NAMESLOT",
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <installedversions:NAMESLOT>."));

AddOption(STRING, "NAMEASLOT",
	"<category>/<name>:<slot>"
	"%{VERSION_NEWLINE}", P_("NAMEASLOT",
	"This variable is used as a version formatter.\n"
	"It is an example for usage with <installedversions:NAMEASLOT>."));

AddOption(STRING, "DATESORT_DATE",
	"%s	%x %X", P_("DATESORT_DATE",
	"strftime() format for printing the installation date in DATESORT"));

AddOption(STRING, "DATESORT",
	"<date:DATESORT_DATE>\\t%{NAMESLOT}", P_("DATESORT",
	"This variable is used as a version formatter.\n"
	"It is an example for usage as <installedversions:DATESORT>. Typical usage:\n"
	"eix -'*I' --format '<installedversions:DATESORT>' | sort | cut -f2-3"));

AddOption(STRING, "FORMAT_EAPI",
	"(%{COLOR_EAPI})<eapi>(%{COLOR_RESET})", P_("FORMAT_EAPI",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the EAPI output."));

AddOption(STRING, "FORMAT_DEPEND",
	"(%{COLOR_DEPEND})<depend*>(%{COLOR_DEPEND_END})", P_("FORMAT_DEPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the DEPEND output."));

AddOption(STRING, "FORMAT_RDEPEND",
	"(%{COLOR_DEPEND})<rdepend*>(%{COLOR_DEPEND_END})", P_("FORMAT_RDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the RDEPEND output."));

AddOption(STRING, "FORMAT_PDEPEND",
	"(%{COLOR_DEPEND})<pdepend*>(%{COLOR_DEPEND_END})", P_("FORMAT_PDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PDEPEND output."));

AddOption(STRING, "FORMAT_HDEPEND",
	"(%{COLOR_DEPEND})<hdepend*>(%{COLOR_DEPEND_END})", P_("FORMAT_HDEPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the HDEPEND output."));

AddOption(STRING, "FORMAT_DEPEND_VERBOSE",
	"%{FORMAT_DEPEND}", P_("FORMAT_DEPEND_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the DEPEND output with --verbose."));

AddOption(STRING, "FORMAT_RDEPEND_VERBOSE",
	"%{FORMAT_RDEPEND}", P_("FORMAT_RDEPEND_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the RDEPEND output with --verbose."));

AddOption(STRING, "FORMAT_PDEPEND_VERBOSE",
	"%{FORMAT_PDEPEND}", P_("FORMAT_PDEPEND_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PDEPEND output with --verbose."));

AddOption(STRING, "FORMAT_HDEPEND_VERBOSE",
	"%{FORMAT_HDEPEND}", P_("FORMAT_HDEPEND_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the HDEPEND output with --verbose."));

AddOption(STRING, "FORMAT_VERSION_IUSE",
	"(%{COLOR_VERSION_IUSE})"
		"%{?OMIT_EXPAND}"
			"<use0>"
		"%{else}"
			"%{?EIX_USE_EXPAND}<use*>%{else}<use>%{}"
		"%{}", P_("FORMAT_VERSION_IUSE",
	"This variable is only used for delayed substitution.\n"
	"It is used for colored <use> in available versions; color is not reset."));

AddOption(STRING, "FORMAT_COLLIUSE",
	"(%{COLOR_COLL_IUSE})"
		"%{?OMIT_EXPAND}"
			"<colliuse0>"
		"%{else}"
			"%{?EIX_USE_EXPAND}<colliuse*>%{else}<colliuse>%{}"
		"%{}", P_("FORMAT_COLLIUSE",
	"This variable is only used for delayed substitution.\n"
	"It is used for colored <colliuse>; color is not reset."));

AddOption(STRING, "FORMAT_VERSION_REQUIRED_USE",
	"(%{COLOR_REQUIRED_USE})<requireduse>", P_("FORMAT_VERSION_REQUIRED_USE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for REQUIRED_USE for an available version."));

AddOption(STRING, "FORMAT_KEYWORDS",
	"(%{COLOR_KEYWORDS})<versionkeywords>(%{COLOR_RESET})", P_("FORMAT_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"It is used for printing colored <keywords>; color is not reset."));

AddOption(STRING, "FORMAT_KEYWORDSS",
	"(%{COLOR_KEYWORDSS})<versionkeywords*>(%{COLOR_RESET})", P_("FORMAT_KEYWORDSS",
	"This variable is only used for delayed substitution.\n"
	"It is used for printing colored <keywords*>; color is not reset."));

AddOption(STRING, "FORMAT_KEYWORDS_EQUAL",
	"{versionkeywords}(%{COLOR_KEYWORDSS})$\\{KEYWORDS\\}(%{COLOR_RESET}){}", P_("FORMAT_KEYWORDS_EQUAL",
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
	"{}", P_("FORMAT_MASK_TAG",
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
	"{}{}{}{}{}{}{}", P_("FORMAT_STABILITY_TAG",
	"This variable is only used for delayed substitution.\n"
	"It sets the runtime variable $stable to the stability tag, and unless the\n"
	"runtime variable $color is set, it outputs the color and sets $color."));

AddOption(STRING, "FORMAT_STABILITY",
	"{!*color}%{FORMAT_MASK_TAG}%{FORMAT_STABILITY_TAG}<$mask><$stable>", P_("FORMAT_STABILITY",
	"This variable is only used for delayed substitution.\n"
	"It outputs the stability tag, changing the color appropriately.\n"
	"It sets the runtime variable $color depending on whether color was changed."));

AddOption(STRING, "FORMAT_PROPERTIESSEPARATOR",
	"<$sep>{!*sep}{*color}(%{COLOR_PROPERTIES})*", P_("FORMAT_PROPERTIESSEPARATOR",
	"This variable is only used for delayed substitution.\n"
	"It sets the initial string and $color for PROPERTIES output."));

AddOption(STRING, "FORMAT_RESTRICTSEPARATOR",
	"<$sep>{!*sep}{*color}(%{COLOR_RESTRICT})^", P_("FORMAT_RESTRICTSEPARATOR",
	"This variable is only used for delayed substitution.\n"
	"It sets the initial string and $color for RESTRICT output."));

AddOption(STRING, "FORMAT_PROPERTIES",
	"{properties}%{FORMAT_PROPERTIESSEPARATOR}"
		"{propertiesinteractive}%{TAG_PROPERTIES_INTERACTIVE}{}"
		"{propertieslive}%{TAG_PROPERTIES_LIVE}{}"
		"{propertiesset}%{TAG_PROPERTIES_SET}{}"
	"{}", P_("FORMAT_PROPERTIES",
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
	"{}", P_("FORMAT_RESTRICT",
	"This variable is only used for delayed substitution.\n"
	"It outputs the RESTRICT tag, changing the color appropriately.\n"
	"It sets the runtime variable $color if color was changed."));

AddOption(STRING, "FORMAT_PROPRESTRICT",
	"%{!NO_RESTRICTIONS}"
		"{!*color}%{FORMAT_PROPERTIES}%{FORMAT_RESTRICT}"
		"{$color}(%{COLOR_RESET}){}"
	"%{}", P_("FORMAT_PROPRESTRICT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PROPERTIES and RESTRICT of a version\n"
	"and resets the color."));

AddOption(STRING, "FORMAT_BINARY",
	"%{!NO_BINARY}{isbinary}"
		"<$sep>{!*sep}(%{COLOR_BINARY})\\{"
		"{istbz}"
			"%{FORMAT_TBZ}"
			"{ispak}(%{COLOR_BINARY}),{}"
		"{}"
		"(%{COLOR_RESET})"
		"{ispak}"
			"%{FORMAT_PAK}"
		"{}"
		"(%{COLOR_BINARY})}(%{COLOR_RESET})"
	"{}%{}", P_("FORMAT_BINARY",
	"This variable is only used for delayed substitution.\n"
	"It defines the format of the PROPERTIES and RESTRICT of a version\n"
	"and resets the color."));

AddOption(STRING, "FORMAT_TBZ",
	"(%{COLOR_TBZ})%{TAG_TBZ}(%{COLOR_RESET})", P_("FORMAT_TBZ",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for versions with *.tbz2."));

AddOption(STRING, "FORMAT_PAK",
	"(%{COLOR_PAK})%{TAG_PAK}"
	"{ismultipak}"
		"%{TAG_MULTIPAK}"
		"(%{COLOR_PAKCOUNT})<pakcount>"
	"{}"
	"(%{COLOR_RESET})", P_("FORMAT_PAK",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for versions with *.xpak."));

AddOption(STRING, "FORMAT_SLOT",
	"(%{COLOR_SLOTS})\\(<slot>\\)(%{COLOR_RESET})", P_("FORMAT_SLOT",
	"This variable is only used for delayed substitution.\n"
	"It defines the slot format printed for slotsorted versions."));

AddOption(STRING, "MARKEDVERSION",
	"(%{COLOR_MARKED_VERSION})<version>(%{COLOR_RESET}){!last} {}", P_("MARKEDVERSION",
	"This variable is used for <markedversions:MARKEDVERSION>."));

AddOption(STRING, "FORMAT_PVERSION",
	"{color}"
		"{installedversion}"
			"{*color}(none;%{MARK_INSTALLED})"
		"{else}"
			"{isbestupgradeslot}{*color}(%{MARK_UPGRADE}){}"
		"{}"
		"{markedversion}{*color}(%{MARK_VERSIONS}){}"
	"{}"
	"<version>", P_("FORMAT_PVERSION",
	"This variable is only used for delayed substitution.\n"
	"It outputs an available version with various marks and sets the runtime\n"
	"variable $color if a mark was printed.\n"
	"It should be follows by FORMAT_VERSION_END or FORMAT_VERSIONS_END."));

AddOption(STRING, "FORMAT_VERSION_END",
	"{$color}(%{COLOR_RESET}){}", P_("FORMAT_VERSION_END",
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
	"%{FORMAT_VERSION_END}", P_("FORMAT_VERSIONO_END",
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
	"%{FORMAT_VERSION_END}", P_("FORMAT_VERSIONS_END",
	"This variable is only used for delayed substitution.\n"
	"It outputs an optional slot, caring about the runtime variable $color,\n"
	"and then invokes FORMAT_VERSION_END."));

AddOption(STRING, "PVERSION",
	"%{FORMAT_PVERSION}%{FORMAT_VERSION_END}", P_("PVERSION",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version without slot."));

AddOption(STRING, "PVERSIONO",
	"%{FORMAT_PVERSION}%{FORMAT_VERSIONO_END}", P_("PVERSIONO",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing a plain version with its slot."));

AddOption(STRING, "PVERSIONS",
	"%{FORMAT_PVERSION}%{FORMAT_VERSIONS_END}", P_("PVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing a plain version with its slot."));

AddOption(STRING, "AVERSION",
	"%{FORMAT_STABILITY}%{FORMAT_PVERSION}%{FORMAT_VERSION_END}", P_("AVERSION",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version without slot."));

AddOption(STRING, "AVERSIONO",
	"%{FORMAT_STABILITY}%{FORMAT_PVERSION}%{FORMAT_VERSIONO_END}", P_("AVERSIONO",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing an available version\n"
	"with its optional subslot."));

AddOption(STRING, "AVERSIONS",
	"%{FORMAT_STABILITY}%{FORMAT_PVERSION}%{FORMAT_VERSIONS_END}", P_("AVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing an available version with its slot."));

AddOption(STRING, "IVERSIONS",
	"(%{COLOR_INST_VERSION}){*color}<version>%{FORMAT_VERSIONS_END}", P_("IVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing an installed version with its slot."));

AddOption(STRING, "OVERLAYVER",
	"{overlayver}<$sep>{!*sep}<overlayver>{}", P_("OVERLAYVER",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for printing the overlay in versions."));

AddOption(STRING, "PVERSIONS_VERBOSE",
	"%{PVERSIONS}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("PVERSIONS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with most data and slot."));

AddOption(STRING, "AVERSIONS_VERBOSE",
	"%{AVERSIONS}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("AVERSIONS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with most data and slot."));

AddOption(STRING, "IVERSIONS_VERBOSE",
	"%{IVERSIONS}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("IVERSIONS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an installed version with most data and slot."));
#endif

#if (DEFAULT_PART == 5)

AddOption(STRING, "PVERSIONO_VERBOSE",
	"%{PVERSIONO}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("PVERSIONO_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with most data, optional subslot."));

AddOption(STRING, "AVERSIONO_VERBOSE",
	"%{AVERSIONO}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("AVERSIONO_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with most data,\n"
	"optional subslot."));

AddOption(STRING, "PVERSION_VERBOSE",
	"%{PVERSION}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("PVERSION_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with most data, no slot."));

AddOption(STRING, "AVERSION_VERBOSE",
	"%{AVERSION}%{FORMAT_PROPRESTRICT}%{FORMAT_BINARY}%{OVERLAYVER}", P_("AVERSION_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with most data, no slot."));

AddOption(STRING, "PVERSIONS_COMPACT",
	"%{PVERSIONS}%{FORMAT_BINARY}%{OVERLAYVER}", P_("PVERSIONS_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a plain version with important data and slot."));

AddOption(STRING, "AVERSIONS_COMPACT",
	"%{AVERSIONS}%{FORMAT_BINARY}%{OVERLAYVER}", P_("AVERSIONS_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an available version with important data and slot."));

AddOption(STRING, "IVERSIONS_COMPACT",
	"%{IVERSIONS}%{FORMAT_BINARY}%{OVERLAYVER}", P_("IVERSIONS_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for an installed version with important data and slot."));

AddOption(STRING, "FORMAT_BEFORE_KEYWORDS",
	"(%{COLOR_NORMAL}) \\C<%{COLUMN_KEYWORDS}>\"", P_("FORMAT_BEFORE_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"This string is printed before KEYWORDS string for a version is output\n"
	"(with --versionlines and nonverbose)"));

AddOption(STRING, "FORMAT_AFTER_KEYWORDS",
	"(%{COLOR_NORMAL})\"(%{COLOR_NORMAL_END})", P_("FORMAT_AFTER_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"This string is printed after KEYWORDS string for a version is output.\n"
	"(with --versionlines and nonverbose)"));

AddOption(STRING, "FORMAT_NEWLINE",
	"%{?RESET_ALL_LINES}()\\n(%{COLOR_RESET_BG})%{else}\\n%{}", P_("FORMAT_NEWLINE",
	"This variable only used for delayed substitution.\n"
	"It prints a newline, optionally handling background colors for some terminals."));

AddOption(STRING, "COLUMN_KEYWORDS",
	"22", P_("COLUMN_KEYWORDS",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column after the version."));

AddOption(STRING, "COLUMN_SLOT",
	"%{C_COLUMN_TITLE}", P_("COLUMN_SLOT",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column for the slot without versionlines."));

AddOption(STRING, "COLUMN_SLOT_AFTER",
	"12", P_("COLUMN_SLOT_AFTER",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column after the slot without versionlines."));

AddOption(STRING, "COLUMN_STABILITY",
	"7", P_("COLUMN_STABILITY",
	"This variable is only used for delayed substitution.\n"
	"It defines the first stability column with versionlines."));

AddOption(STRING, "COLUMN_STABILITY_AFTER",
	"12", P_("COLUMN_STABILITY_AFTER",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column after stability with versionlines."));

AddOption(STRING, "COLUMN_SLOT_APPEND",
	"22", P_("COLUMN_SLOT_APPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column of the slot with versionsort."));

AddOption(STRING, "COLUMN_VER_APPEND",
	"%{COLUMN_SLOT_APPEND}", P_("COLUMN_VER_APPEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column after the version without versionsort."));

AddOption(STRING, "COLUMN_USE",
	"7", P_("COLUMN_USE",
	"This variable is only used for delayed substitution.\n"
	"It defines the first column of the collected USE."));

AddOption(STRING, "DIFF_COLUMNA",
	"6", P_("DIFF_COLUMNA",
	"This variable is only used for delayed substitution.\n"
	"It defines the column for the main tag in eix-diff."));

AddOption(STRING, "DIFF_COLUMNB",
	"9", P_("DIFF_COLUMNB",
	"This variable is only used for delayed substitution.\n"
	"It defines the column after the tags in eix-diff."));

AddOption(STRING, "FORMAT_VER_LINESKIP",
	"%{FORMAT_NEWLINE}\\C<%{I18N_COLUMN_AVAILABLE_TITLE}>", P_("FORMAT_VER_LINESKIP",
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
	"%{}", P_("FORMAT_VERSION_KEYWORDS_NORMAL",
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
	"%{}", P_("FORMAT_IUSE_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines the normal format for IUSE for an available version."));

AddOption(STRING, "FORMAT_REQUIRED_USE_NORMAL",
	"%{?PRINT_IUSE}"
		"%{?VERSION_IUSE_NORMAL}"
			"{haverequireduse}"
				"%{FORMAT_BEFORE_REQUIRED_USE}"
				"%{FORMAT_VERSION_REQUIRED_USE}"
				"%{FORMAT_AFTER_REQUIRED_USE}"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_REQUIRED_USE_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines the normal format for REQUIRED_USE for an available version."));

AddOption(STRING, "FORMAT_DEPS_NORMAL",
	"%{?DEP}"
		"%{?VERSION_DEPS_NORMAL}"
			"{havedepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_DEPEND}(%{COLOR_RESET})"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_DEPEND}"
			"{}"
			"{haverdepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_RDEPEND}(%{COLOR_RESET})"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_RDEPEND}"
			"{}"
			"{havepdepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_PDEPEND}(%{COLOR_RESET})"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_PDEPEND}"
			"{}"
			"{havehdepend}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_HDEPEND}(%{COLOR_RESET})"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_HDEPEND}"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_DEPS_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for DEPs for an available version."));

AddOption(STRING, "FORMAT_MASKREASONS_NORMAL",
	"%{?PRINT_MASKREASONS}"
		"%{?MASKREASONS_NORMAL}"
			"{havemaskreasons}"
				"%{?WIDETERM} %{else}%{FORMAT_NEWLINE}%{}"
				"(%{COLOR_MASKREASONS})<maskreasons>(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_MASKREASONS_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines the normal format for mask reasons for an available version."));

AddOption(STRING, "FORMAT_VERSION_KEYWORDS_VERBOSE",
	"%{?PRINT_KEYWORDS}"
		"%{?VERSION_KEYWORDS_VERBOSE}"
			"%{!PRINT_ALWAYS}{versionkeywords}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_KEYWORDS}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{versionkeywords}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_KEYWORDS}(%{COLOR_RESET})"
			"{}"
			"%{!PRINT_ALWAYS}{versionekeywords}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_KEYWORDSS}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}"
					"{!versionekeywords}"
						"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
						"%{FORMAT_KEYWORDS_EQUAL}"
					"{else}"
				"%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_KEYWORDSS}(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_VERSION_KEYWORDS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for KEYWORDS for an available version."));

AddOption(STRING, "FORMAT_IUSE_VERBOSE",
	"%{?PRINT_IUSE}"
		"%{?VERSION_IUSE_VERBOSE}"
			"%{!PRINT_ALWAYS}{haveuse}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_IUSE}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{haveuse}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_VERSION_IUSE}(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_IUSE_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for IUSE for an available version."));

AddOption(STRING, "FORMAT_REQUIRED_USE_VERBOSE",
	"%{?PRINT_IUSE}"
		"%{?VERSION_IUSE_VERBOSE}"
			"%{!PRINT_ALWAYS}{haverequireduse}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_REQUIRED_USE}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{haverequireduse}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_VERSION_REQUIRED_USE}(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_REQUIRED_USE_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for REQUIRED_USE for an available version."));

AddOption(STRING, "FORMAT_DEPS_VERBOSE",
	"%{?DEP}"
		"%{?VERSION_DEPS_VERBOSE}"
			"%{!PRINT_ALWAYS}{havedepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_DEPEND}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{depend}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_DEPEND_VERBOSE}"
			"{}"
			"%{!PRINT_ALWAYS}{haverdepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_RDEPEND}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{haverdepend}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_RDEPEND_VERBOSE}"
			"{}"
			"%{!PRINT_ALWAYS}{havepdepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_PDEPEND}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{havepdepend}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_PDEPEND_VERBOSE}"
			"{}"
			"%{!PRINT_ALWAYS}{havehdepend}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_HDEPEND}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{havehdepend}%{}"
				"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{FORMAT_HDEPEND_VERBOSE}"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_DEPS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for DEPs for an available version."));

AddOption(STRING, "FORMAT_EAPI_VERBOSE",
	"%{?PRINT_EAPI}"
		"%{FORMAT_VER_LINESKIP}"
		"(%{COLOR_AVAILABLE_TITLE})%{I18N_EAPI}(%{COLOR_RESET})"
		"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
		"%{FORMAT_EAPI}"
	"%{}", P_("FORMAT_EAPI_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for the EAPI of an available version."));

AddOption(STRING, "FORMAT_MASKREASONS_VERBOSE",
	"%{?PRINT_MASKREASONS}"
		"%{?MASKREASONS_VERBOSE}"
			"%{!PRINT_ALWAYS}{havemaskreasons}%{}"
				"%{FORMAT_VER_LINESKIP}"
				"(%{COLOR_AVAILABLE_TITLE})%{I18N_MASK}(%{COLOR_RESET})"
				"%{?PRINT_ALWAYS}{havemaskreasons}%{}"
				"%{?WIDETERM}"
					"\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
				"%{else}"
					"%{FORMAT_NEWLINE}"
				"%{}"
				"(%{COLOR_MASKREASONS})<maskreasons*>(%{COLOR_RESET})"
			"{}"
		"%{}"
	"%{}", P_("FORMAT_MASKREASONS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for mask reasons for an available version."));

AddOption(STRING, "FORMAT_VERSION_APPENDIX_NORMAL",
	"%{FORMAT_VERSION_KEYWORDS_NORMAL}"
	"%{FORMAT_IUSE_NORMAL}"
	"%{FORMAT_REQUIRED_USE_NORMAL}"
	"%{FORMAT_DEPS_NORMAL}"
	"%{FORMAT_MASKREASONS_NORMAL}", P_("FORMAT_VERSION_APPENDIX_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines normal data appended to available versions with --versionlines"));

AddOption(STRING, "FORMAT_VERSION_APPENDIX_VERBOSE",
	"%{FORMAT_VERSION_KEYWORDS_VERBOSE}"
	"%{FORMAT_IUSE_VERBOSE}"
	"%{FORMAT_REQUIRED_USE_VERBOSE}"
	"%{FORMAT_DEPS_VERBOSE}"
	"%{FORMAT_EAPI_VERBOSE}"
	"%{FORMAT_MASKREASONS_VERBOSE}", P_("FORMAT_VERSION_APPENDIX_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines verbose data appended to available versions with --versionlines"));

AddOption(STRING, "FORMAT_VERSION_APPENDIX",
	"{$modus=verbose}"
		"%{FORMAT_VERSION_APPENDIX_VERBOSE}"
	"{else}"
		"%{FORMAT_VERSION_APPENDIX_NORMAL}"
	"{}", P_("FORMAT_VERSION_APPENDIX",
	"This variable is only used for delayed substitution.\n"
	"It defines the data appended to available versions with --versionlines"));

AddOption(STRING, "FORMAT_SLOTLINESKIP_VERSIONLINES",
	"%{FORMAT_NEWLINE}\\C<%{COLUMN_SLOT}>%{FORMAT_SLOT}", P_("FORMAT_SLOTLINESKIP_VERSIONLINES",
	"This variable is only used for delayed substitution.\n"
	"It defines lineskip + slot if slotsorted versionlines are used."));

AddOption(STRING, "FORMAT_SLOTLINESKIP",
	"%{FORMAT_NEWLINE}\\C<%{COLUMN_SLOT}>%{FORMAT_SLOT} \\C<%{COLUMN_SLOT_AFTER}>", P_("FORMAT_SLOTLINESKIP",
	"This variable is only used for delayed substitution.\n"
	"It defines lineskip + slot if slotsort but no versionlines are used."));

AddOption(STRING, "FORMAT_VERSLINESKIP",
	"%{FORMAT_NEWLINE}\\C<%{COLUMN_STABILITY}>%{FORMAT_STABILITY} \\C<%{COLUMN_STABILITY_AFTER}>", P_("FORMAT_VERSLINESKIP",
	"This variable is only used for delayed substitution.\n"
	"It defines lineskip + stability if lineskip is used."));

AddOption(STRING, "FORMAT_INST_LINESKIP",
	"%{FORMAT_NEWLINE}\\C<%{I18N_COLUMN_INST_TITLE}>", P_("FORMAT_INST_LINESKIP",
	"This variable is only used for delayed substitution.\n"
	"It defines the lineskip for installed versions."));

AddOption(STRING, "VSORTL",
	"%{FORMAT_VERSLINESKIP}{*sep=\\ \\C<%{COLUMN_SLOT_APPEND}>}%{PVERSIONS_VERBOSE}{!*sep}"
	"%{FORMAT_VERSION_APPENDIX}{last}{*sorted=version}{}", P_("VSORTL",
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; versionsorted with versionlines."));

AddOption(STRING, "VSORT",
	"%{AVERSIONS_VERBOSE}{last}{*sorted=version}{else} {}", P_("VSORT",
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; versionsorted without versionlines."));

AddOption(STRING, "SSORTL",
	"{slotfirst}%{FORMAT_SLOTLINESKIP_VERSIONLINES}{}"
	"%{FORMAT_VERSLINESKIP}{*sep=\\ \\C<%{COLUMN_VER_APPEND}>}%{PVERSIONO_VERBOSE}{!*sep}"
	"%{FORMAT_VERSION_APPENDIX}{last}{*sorted=slot}{}", P_("SSORTL",
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; slotsorted with versionlines."));

AddOption(STRING, "SSORT",
	"{slotfirst}"
		"{oneslot}%{FORMAT_SLOT} "
		"{else}%{FORMAT_SLOTLINESKIP}{}"
	"{else} {}"
	"%{AVERSIONO_VERBOSE}{last}{*sorted=slot}{}", P_("SSORT",
	"This variable is used as a version formatter.\n"
	"It defines the format for a version; slotsorted without versionlines."));

AddOption(STRING, "FORMAT_COLL",
	"%{FORMAT_BEFORE_COLL}%{FORMAT_COLLIUSE}%{FORMAT_AFTER_COLL}", P_("FORMAT_COLL",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data in one line."));

AddOption(STRING, "FORMAT_COLL_SEP",
	"%{FORMAT_BEFORE_COLL_SEP}%{FORMAT_COLLIUSE}%{FORMAT_AFTER_COLL_SEP}", P_("FORMAT_COLL_SEP",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data in a separate line."));

AddOption(STRING, "FORMAT_COLL_VERBOSE",
	"%{!PRINT_ALWAYS}{havecolliuse}%{}"
		"%{FORMAT_NEWLINE}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_IUSEALLVERSIONS}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{havecolliuse}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"%{FORMAT_COLLIUSE}(%{COLOR_RESET})"
	"{}",  P_("FORMAT_COLL_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It is the format for package IUSE in single-line verbose mode."));

AddOption(STRING, "FORMAT_COLL_VERBOSE_SEP",
	"%{FORMAT_COLL_VERBOSE}", P_("FORMAT_COLL_VERBOSE_SEP",
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
	"%{}", P_("FORMAT_COLL_IUSE",
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
	"%{}", P_("FORMAT_COLL_IUSE_LIST",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for collected IUSE data with --versionlines."));

AddOption(STRING, "INORMAL",
	"%{IVERSIONS_VERBOSE}"
	"(%{COLOR_DATE})\\(<date:FORMAT_INSTALLATION_DATE>\\)(%{COLOR_RESET})"
	"%{FORMAT_INST_USEFLAGS}"
	"{!last}"
		"{versionlines}%{FORMAT_INST_LINESKIP}"
		"{else} {}"
	"{}", P_("INORMAL",
	"This variable is used as a version formatter.\n"
	"It defines the normal format of installed versions."));

AddOption(STRING, "ICOMPACT",
	"%{IVERSIONS_COMPACT}"
	"(%{COLOR_NORMAL})@(%{COLOR_DATE})<date:FORMAT_SHORT_INSTALLATION_DATE>(%{COLOR_RESET})"
	"{!last} {}", P_("ICOMPACT",
	"This variable is used as a version formatter.\n"
	"It defines the compact format of installed versions."));

AddOption(STRING, "IVERBOSE",
	"(%{COLOR_INST_TITLE})%{I18N_VERSION}(%{COLOR_RESET})"
	"\\C<%{I18N_COLUMN_INST_CONTENT}>"
	"%{IVERSIONS_VERBOSE}(%{COLOR_RESET})"
	"%{FORMAT_INST_LINESKIP}"
	"(%{COLOR_INST_TITLE})%{I18N_DATE}(%{COLOR_RESET})"
	"\\C<%{I18N_COLUMN_INST_CONTENT}>"
	"(%{COLOR_DATE})<date:FORMAT_INSTALLATION_DATE>(%{COLOR_RESET})"
	"{haveuse}"
		"%{FORMAT_INST_LINESKIP}"
		"(%{COLOR_INST_TITLE})%{I18N_USE}(%{COLOR_RESET})"
		"\\C<%{I18N_COLUMN_INST_CONTENT}>"
		"%{?OMIT_EXPAND}"
			"<use0>"
		"%{else}"
			"%{?EIX_USE_EXPAND}<use*>%{else}<use>%{}"
		"%{}"
	"{}"
	"%{?DEP}"
		"{havedepend}"
			"%{FORMAT_INST_LINESKIP}"
			"(%{COLOR_INST_TITLE})%{I18N_DEPEND}(%{COLOR_RESET})"
			"\\C<%{I18N_COLUMN_INST_CONTENT}>"
			"%{FORMAT_DEPEND_VERBOSE}"
		"{}"
		"{haverdepend}"
			"%{FORMAT_INST_LINESKIP}"
			"(%{COLOR_INST_TITLE})%{I18N_RDEPEND}(%{COLOR_RESET})"
			"\\C<%{I18N_COLUMN_INST_CONTENT}>"
			"%{FORMAT_RDEPEND_VERBOSE}"
		"{}"
		"{havepdepend}"
			"%{FORMAT_INST_LINESKIP}"
			"(%{COLOR_INST_TITLE})%{I18N_PDEPEND}(%{COLOR_RESET})"
			"\\C<%{I18N_COLUMN_INST_CONTENT}>"
			"%{FORMAT_PDEPEND_VERBOSE}"
		"{}"
		"{havehdepend}"
			"%{FORMAT_INST_LINESKIP}"
			"(%{COLOR_INST_TITLE})%{I18N_HDEPEND}(%{COLOR_RESET})"
			"\\C<%{I18N_COLUMN_INST_CONTENT}>"
			"%{FORMAT_HDEPEND_VERBOSE}"
		"{}"
	"%{}"
	"%{?PRINT_EAPI}"
		"%{FORMAT_INST_LINESKIP}"
		"(%{COLOR_INST_TITLE})%{I18N_EAPI}(%{COLOR_RESET})"
		"\\C<%{I18N_COLUMN_INST_CONTENT}>"
		"%{FORMAT_EAPI}"
	"%{}"
	"{!last}%{FORMAT_INST_LINESKIP}{}", P_("IVERBOSE",
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
	"{}", P_("FORMAT_AVAILABLEVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the verbose format for printing available versions."));

AddOption(STRING, "FORMAT_INST_USEFLAGS",
	"{haveuse}"
		"(%{COLOR_NORMAL})\\("
		"%{?OMIT_EXPAND}"
			"<use0>"
		"%{else}"
			"%{?EIX_USE_EXPAND}<use*>%{else}<use>%{}"
		"%{}"
		"(%{COLOR_NORMAL})\\)(%{COLOR_NORMAL_END})"
	"{}", P_("FORMAT_INST_USEFLAGS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for useflags in installed versions."));

AddOption(STRING, "INSTALLEDVERSIONS_NORMAL",
	"<installedversions:INORMAL>", P_("INSTALLEDVERSIONS_NORMAL",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions normally."));

AddOption(STRING, "INSTALLEDVERSIONS_COMPACT",
	"<installedversions:ICOMPACT>", P_("INSTALLEDVERSIONS_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions compactly."));

AddOption(STRING, "INSTALLEDVERSIONS_VERBOSE",
	"<installedversions:IVERBOSE>", P_("INSTALLEDVERSIONS_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions verbosely."));

AddOption(STRING, "INSTALLEDVERSIONS",
	"{$modus=verbose}"
		"%{INSTALLEDVERSIONS_VERBOSE}"
	"{else}"
		"%{INSTALLEDVERSIONS_NORMAL}"
	"{}", P_("INSTALLEDVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format used for printing installed versions."));

AddOption(STRING, "FORMATLINE_INSTALLEDVERSIONS",
	"%{!PRINT_ALWAYS}{installed}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_INSTALLEDVERSIONS}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{installed}%{}"
			"\\C<%{I18N_COLUMN_CONTENT}>"
			"%{INSTALLEDVERSIONS}"
		"%{?PRINT_ALWAYS}{else}"
			"\\C<%{I18N_COLUMN_CONTENT}>"
			"%{I18N_NONE}{}%{FORMAT_NEWLINE}"
		"%{else}%{FORMAT_NEWLINE}{}"
	"%{}", P_("FORMATLINE_INSTALLEDVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with installed versions."));

AddOption(STRING, "DIFF_FORMATLINE_INSTALLEDVERSIONS",
	"{installed}%{INSTALLEDVERSIONS_COMPACT}"
	"(%{COLOR_NORMAL});(%{COLOR_NORMAL_END}) {}", P_("DIFF_FORMATLINE_INSTALLEDVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for eix-diff for installed versions."));

AddOption(STRING, "FORMAT_FINISH",
	"()\\n", P_("FORMAT_FINISH",
	"This variable is only used for delayed substitution.\n"
	"It prints a newline at the end of a package."));

AddOption(STRING, "FORMAT_NAME",
	"{system}(%{COLOR_CATEGORY_SYSTEM})"
	"{else}"
		"{profile}(%{COLOR_CATEGORY_PROFILE})"
		"{else}"
			"{world}(%{COLOR_CATEGORY_WORLD})"
			"{else}"
				"{world_sets}(%{COLOR_CATEGORY_WORLD_SETS})"
				"{else}(%{COLOR_CATEGORY}){}"
			"{}"
		"{}"
	"{}<category>(%{COLOR_NORMAL})/"
	"{marked}(%{COLOR_MARKED_NAME})"
	"{else}"
		"{world}(%{COLOR_WORLD})"
		"{else}"
			"{world_sets}(%{COLOR_WORLD_SETS})"
			"{else}(%{COLOR_NAME}){}"
		"{}"
	"{}<name>(%{COLOR_RESET})", P_("FORMAT_NAME",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the printing the package name."));

AddOption(STRING, "FORMAT_HEADER",
	"{installed}"
		"(%{COLOR_NORMAL})[{!*updn}"
		"{upgrade}{*updn}%{TAG_UPGRADE}{}"
		"{downgrade}{*updn}%{TAG_DOWNGRADE}{}"
		"{!$updn}%{TAG_INSTALLED}{}"
		"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})"
	"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}", P_("FORMAT_HEADER",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the normal header symbols."));

AddOption(STRING, "FORMAT_HEADER_VERBOSE",
	"{installed}(%{COLOR_INST_TAG})%{STRING_PLAIN_INSTALLED}"
	"{else}(%{COLOR_UNINST_TAG})%{STRING_PLAIN_UNINSTALLED}{}"
	"(%{COLOR_RESET})", P_("FORMAT_HEADER_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the verbose header symbols."));

AddOption(STRING, "FORMAT_HEADER_COMPACT",
	"(%{COLOR_NORMAL})["
	"{installed}"
		"{upgrade}{*upgrade}%{TAG_UPGRADE}{else}{!*upgrade}{}"
		"{downgrade}%{TAG_DOWNGRADE}"
		"{else}{!$upgrade}%{TAG_INSTALLED}{}{}"
	"{else}%{TAG_UNINSTALLED}{}"
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})", P_("FORMAT_HEADER_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the compact header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_NEW",
	"(%{COLOR_NORMAL})["
	"{havebest}%{TAG_STABILIZE}{}"
	"{upgrade}{*upgrade}%{TAG_UPGRADE}{else}{!*upgrade}{}"
	"{downgrade}%{TAG_DOWNGRADE}"
	"{else}{!$upgrade}%{TAG_NEW}{}{}"
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})"
	"\\C<%{DIFF_COLUMNA}>"
	"(%{DIFF_COLOR_NEW})%{DIFF_STRING_NEW}(%{COLOR_RESET})"
	"\\C<%{DIFF_COLUMNB}>", P_("DIFF_FORMAT_HEADER_NEW",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-new header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_DELETE",
	"{installed}"
		"(%{COLOR_NORMAL})["
		"(%{COLOR_DOWNGRADE})%{CHAR_DOWNGRADE}"
		"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})"
	"{}\\C<%{DIFF_COLUMNA}>"
	"(%{DIFF_COLOR_DELETE})%{DIFF_STRING_DELETE}(%{COLOR_RESET})"
	"\\C<%{DIFF_COLUMNB}>", P_("DIFF_FORMAT_HEADER_DELETE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-delete header symbols."));

AddOption(STRING, "DIFF_FORMAT_HEADER_CHANGED",
	"(%{COLOR_NORMAL})["
	"{havebest}{!oldhavebest}%{TAG_STABILIZE}{}{}"
	"{!*better}{!*worse}"
	"{upgrade}%{TAG_UPGRADE}{else}{better}{*better}{}{}"
	"{downgrade}%{TAG_DOWNGRADE}{else}{worse}{*worse}{}{}"
	"{$better}%{TAG_BETTER}{}"
	"{$worse}%{TAG_WORSE}{}"
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})"
	"\\C<%{DIFF_COLUMNA}>"
	"(%{DIFF_COLOR_CHANGED})%{DIFF_STRING_CHANGED}(%{COLOR_RESET})"
	"\\C<%{DIFF_COLUMNB}>", P_("DIFF_FORMAT_HEADER_CHANGED",
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
	"{}", P_("FORMAT_BEST_COMPACT",
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
	"{}", P_("FORMAT_BEST_CHANGE",
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
	"{}", P_("DIFF_FORMAT_BEST",
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
	"{}", P_("DIFF_FORMAT_BEST_CHANGE",
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
	"{}", P_("DIFF_FORMAT_OLDBEST_CHANGE",
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff format for the old best versions in case of changes."));

AddOption(STRING, "DIFF_FORMAT_CHANGED_VERSIONS",
	"%{?DIFF_PRINT_INSTALLED}"
		"%{DIFF_FORMATLINE_INSTALLEDVERSIONS}"
	"%{}"
	"%{DIFF_FORMAT_OLDBEST_CHANGE}"
	" (%{COLOR_NORMAL})->(%{COLOR_NORMAL_END}) "
	"%{DIFF_FORMAT_BEST_CHANGE}", P_("DIFF_FORMAT_CHANGED_VERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff format for changed versions."));

AddOption(STRING, "FORMAT_OVERLAYKEY",
	"{overlaykey} <overlaykey>{}", P_("FORMAT_OVERLAYKEY",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the printing the optional overlay key."));

AddOption(STRING, "FORMATLINE_NAME",
	"%{FORMAT_HEADER} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}%{FORMAT_NEWLINE}", P_("FORMATLINE_NAME",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the normal header line."));

AddOption(STRING, "FORMATLINE_NAME_VERBOSE",
	"%{FORMAT_HEADER_VERBOSE} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}%{FORMAT_NEWLINE}", P_("FORMATLINE_NAME_VERBOSE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the verbose header line."));

AddOption(STRING, "FORMATLINE_NAME_COMPACT",
	"%{FORMAT_HEADER_COMPACT} %{FORMAT_NAME}%{FORMAT_OVERLAYKEY}", P_("FORMATLINE_NAME_COMPACT",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the compact header line."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_NEW",
	"%{DIFF_FORMAT_HEADER_NEW}%{FORMAT_NAME} ", P_("DIFF_FORMATLINE_NAME_NEW",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-new header."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_DELETE",
	"%{DIFF_FORMAT_HEADER_DELETE}%{FORMAT_NAME} ", P_("DIFF_FORMATLINE_NAME_DELETE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-delete header."));

AddOption(STRING, "DIFF_FORMATLINE_NAME_CHANGED",
	"%{DIFF_FORMAT_HEADER_CHANGED}%{FORMAT_NAME} ", P_("DIFF_FORMATLINE_NAME_CHANGED",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for the diff-changed header."));

AddOption(STRING, "FORMATLINE_AVAILABLEVERSIONS",
	"\\C<%{I18N_COLUMN_TITLE}>"
	"(%{COLOR_TITLE})%{I18N_AVAILABLEVERSIONS}(%{COLOR_RESET})"
	"\\C<%{I18N_COLUMN_CONTENT}>"
	"%{FORMAT_AVAILABLEVERSIONS}%{FORMAT_NEWLINE}", P_("FORMATLINE_AVAILABLEVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with installed versions."));

AddOption(STRING, "DIFF_FORMATLINE_BEST",
	"(%{COLOR_NORMAL})\\(%{DIFF_FORMAT_BEST}"
	"(%{COLOR_NORMAL}))(%{COLOR_NORMAL_END})", P_("DIFF_FORMATLINE_BEST",
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff line for the best versions/slots."));

AddOption(STRING, "DIFF_FORMATLINE_CHANGED_VERSIONS",
	"(%{COLOR_NORMAL})\\(%{DIFF_FORMAT_CHANGED_VERSIONS}"
		"(%{COLOR_NORMAL}))(%{COLOR_NORMAL_END})", P_("DIFF_FORMATLINE_CHANGED_VERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the eix-diff line for changed versions."));

AddOption(STRING, "FORMATLINE_MARKEDVERSIONS",
	"%{!PRINT_ALWAYS}{havemarkedversion}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_MARKED}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{havemarkedversion}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"<markedversions:MARKEDVERSION>"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_MARKEDVERSIONS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with marked versions."));

AddOption(STRING, "FORMATLINE_PACKAGESETS",
	"%{!PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_PACKAGESETS}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{%{PRINT_SETNAMES}}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"(%{COLOR_PACKAGESETS})<%{PRINT_SETNAMES}>"
		"%{!RESET_ALL_LINES}(%{COLOR_RESET})%{}"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_PACKAGESETS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with package sets."));

AddOption(STRING, "FORMATLINE_HOMEPAGE",
	"%{!PRINT_ALWAYS}{homepage}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_HOMEPAGE}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{homepage}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"(%{COLOR_NORMAL})<homepage>"
		"%{!RESET_ALL_LINES}(%{COLOR_NORMAL_END})%{}"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_HOMEPAGE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package homepage."));

AddOption(STRING, "FORMATLINE_BUGS",
	"%{?PRINT_BUGS}%{!PRINT_ALWAYS}{mainrepo}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_FINDOPENBUGS}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{mainrepo}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"(%{COLOR_NORMAL})"
		"https://bugs.gentoo.org/buglist.cgi?quicksearch="
		"<category>%2F<name>"
		"%{!RESET_ALL_LINES}(%{COLOR_NORMAL_END})%{}"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}%{}", P_("FORMATLINE_BUGS",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package bug-reference."));

AddOption(STRING, "FORMATLINE_DESCRIPTION",
	"%{!PRINT_ALWAYS}{description}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_DESCRIPTION}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{description}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"(%{COLOR_NORMAL})<description>"
		"%{!RESET_ALL_LINES}(%{COLOR_NORMAL_END})%{}"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_DESCRIPTION",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package description."));

AddOption(STRING, "FORMATLINE_BEST",
	"%{!PRINT_ALWAYS}{havebest}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_BESTVERIONSSLOT}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{havebest}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"<bestslotversions:VSORT>"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_BEST",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the best versions/slots."));

AddOption(STRING, "FORMATLINE_RECOMMEND",
	"%{!PRINT_ALWAYS}{recommend}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_RECOMMENDATION}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{recommend}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"{upgrade}(%{COLOR_UPGRADE_TEXT})%{I18N_UPGRADE}(%{COLOR_RESET})"
			"{downgrade}"
				"%{I18N_AND}"
			"{}"
		"{}"
		"{downgrade}"
			"(%{COLOR_DOWNGRADE_TEXT})%{I18N_DOWNGRADE}"
			"%{!RESET_ALL_LINES}(%{COLOR_RESET})%{}"
		"{}"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_RECOMMEND",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the up-/downgrade recommendations."));

AddOption(STRING, "FORMATLINE_LICENSES",
	"%{!PRINT_ALWAYS}{licenses}%{}"
		"\\C<%{I18N_COLUMN_TITLE}>"
		"(%{COLOR_TITLE})%{I18N_LICENSE}(%{COLOR_RESET})"
		"%{?PRINT_ALWAYS}{licenses}%{}"
		"\\C<%{I18N_COLUMN_CONTENT}>"
		"(%{COLOR_NORMAL})<licenses>"
		"%{!RESET_ALL_LINES}(%{COLOR_NORMAL_END})%{}"
	"%{?PRINT_ALWAYS}{}%{FORMAT_NEWLINE}%{else}%{FORMAT_NEWLINE}{}%{}", P_("FORMATLINE_LICENSES",
	"This variable is only used for delayed substitution.\n"
	"It defines the format for a line with the package licenses."));
#endif

#if (DEFAULT_PART == 6)

AddOption(STRING, "DIFF_FORMATLINE",
	"%{FORMAT_OVERLAYKEY}"
	"(%{COLOR_NORMAL}): <description>"
	"(%{COLOR_NORMAL_END})"
	"%{FORMAT_FINISH}", P_("DIFF_FORMATLINE",
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
	"%{FORMAT_FINISH}", P_("FORMAT_ALL",
	"This format is only used for delayed substitution in FORMAT.\n"
	"It defines the format of the normal output of eix."));

AddOption(STRING, "FORMAT_ALL_COMPACT",
	"{*modus=compact}"
	"%{FORMATLINE_NAME_COMPACT}"
	" (%{COLOR_NORMAL})\\((%{COLOR_NORMAL_END})"
	"{havemarkedversion}<markedversions:MARKEDVERSION>; {}"
	"{installed}"
		"%{INSTALLEDVERSIONS_COMPACT}"
		"{recommend} -> %{FORMAT_BEST_CHANGE}{}"
	"{else}"
		"%{FORMAT_BEST_COMPACT}"
	"{}"
	"(%{COLOR_NORMAL})\\): <description>"
	"(%{COLOR_NORMAL_END})"
	"%{FORMAT_FINISH}", P_("FORMAT_ALL_COMPACT",
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
	"%{FORMAT_FINISH}", P_("FORMAT_ALL_VERBOSE",
	"This format is only used for delayed substitution in FORMAT_VERBOSE.\n"
	"It defines the format of the verbose output of eix (option -v)."));

AddOption(STRING, "FORMAT_VERSION_ETCAT",
	"\\C<8>["
	"{!*mask}{*color=(green,1)}"
	"{ismasked}"
		"{*mask}"
	"{else}"
		"{!isstable}{!isunstable}{*mask}{}{}"
	"{}"
	"{$mask}{*color=(red,1)}<$color>M(){else} {}"
	"{isunstable}"
		"{!$mask}{*color=(yellow,1)}{}"
		"(yellow,1)~()"
	"{else}"
		" "
	"{}"
	"{installedversion}{*color=(cyan,1)}<$color>I(){else} {}"
	"] <$color><version>()"
	" \\(<$color><fullslot>()\\)"
	"{overlayplainname} [<$color><overlayplainname>()]{}"
	"\\n", P_("VERSION_ETCAT",
	"This format outputs a version in the style of the etcat script."));

AddOption(STRING, "FORMAT_ETCAT",
	"(green,1)*()  (white,1)<category>/<name>() :\\n"
	"<availableversions:FORMAT_VERSION_ETCAT>\\n", P_("FORMAT_ETCAT",
	"This format outputs a package in the style of the etcat script."));

AddOption(STRING, "FORMAT",
	"%{FORMAT_ALL}", P_("FORMAT",
	"The format of the normal output of eix.\n"
	"Do not modify it in a config file; modify FORMAT_ALL instead."));

AddOption(STRING, "FORMAT_COMPACT",
	"%{FORMAT_ALL_COMPACT}", P_("FORMAT_COMPACT",
	"The format of the compact output of eix (option -c).\n"
	"Do not modify it in a config file; modify FORMAT_ALL_COMPACT instead."));

AddOption(STRING, "FORMAT_VERBOSE",
	"%{FORMAT_ALL_VERBOSE}", P_("FORMAT_VERBOSE",
	"The format of the verbose output of eix (option -v).\n"
	"Do not modify it in a config file; modify FORMAT_ALL_VERBOSE instead."));

AddOption(STRING, "FORMAT_TEST_OBSOLETE",
	"%{FORMAT_ALL_COMPACT}", P_("FORMAT_TEST_OBSOLETE",
	"The format used for output by the eix-test-obsolete script.\n"
	"The value %{FORMAT} is not allowed here; use %{FORMAT_ALL} instead."));

AddOption(STRING, "DIFF_FORMAT_ALL_NEW",
	"%{DIFF_FORMATLINE_NAME_NEW}"
	"%{DIFF_FORMATLINE_BEST}"
	"%{DIFF_FORMATLINE}", P_("DIFF_FORMAT_ALL_NEW",
	"This format is only used for delayed substitution in DIFF_FORMAT_NEW.\n"
	"It defines the format used for new packages (eix-diff)."));

AddOption(STRING, "DIFF_FORMAT_ALL_DELETE",
	"%{DIFF_FORMATLINE_NAME_DELETE}"
	"%{DIFF_FORMATLINE_BEST}"
	"%{DIFF_FORMATLINE}", P_("DIFF_FORMAT_ALL_DELETE",
	"This format is only used for delayed substitution in DIFF_FORMAT_DELETE.\n"
	"It defines the format used for packages that were deleted (eix-diff)."));

AddOption(STRING, "DIFF_FORMAT_ALL_CHANGED",
	"%{DIFF_FORMATLINE_NAME_CHANGED}"
	"%{DIFF_FORMATLINE_CHANGED_VERSIONS}"
	"%{DIFF_FORMATLINE}", P_("DIFF_FORMAT_ALL_CHANGED",
	"This format is only used for delayed substitution in DIFF_FORMAT_CHANGED.\n"
	"It defines the format used for packages that were changed (eix-diff)."));

AddOption(STRING, "DIFF_FORMAT_NEW",
	"%{DIFF_FORMAT_ALL_NEW}", P_("DIFF_FORMAT_NEW",
	"The format used for new packages (eix-diff).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_NEW instead."));

AddOption(STRING, "DIFF_FORMAT_DELETE",
	"%{DIFF_FORMAT_ALL_DELETE}", P_("DIFF_FORMAT_DELETE",
	"The format used for packages that were deleted (eix-diff).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_DELETE instead."));

AddOption(STRING, "DIFF_FORMAT_CHANGED",
	"%{DIFF_FORMAT_ALL_CHANGED}", P_("DIFF_FORMAT_CHANGED",
	"The format used for packages that were changed (eix-diff).\n"
	"Do not modify it in a config file; modify DIFF_FORMAT_ALL_CHANGED instead."));

AddOption(STRING, "FORMAT_INSTALLATION_DATE",
	"%X %x", P_("FORMAT_INSTALLATION_DATE",
	"strftime() format for printing the installation date in long form"));

AddOption(STRING, "FORMAT_SHORT_INSTALLATION_DATE",
	"%x", P_("FORMAT_SHORT_INSTALLATION_DATE",
	"strftime() format for printing the installation date in short form"));

AddOption(STRING, "XML_KEYWORDS",
	"none", P_("XML_KEYWORDS",
	"Can be full/effective/both/full*/effective*/none.\n"
	"Depending on the value, with --xml the full/effective (or both types)\n"
	"KEYWORDS string is output for each version.\n"
	"With full*/effective* also both types are output if they differ."));

AddOption(STRING, "XML_OVERLAY",
	"false", P_("XML_OVERLAY",
	"If false, the overlay is not output with --xml.\n"
	"For overlays without label (repository name) the overlay is output anyway."));

AddOption(STRING, "XML_DATE",
	"%s", P_("XML_DATE",
	"strftime() format for printing the installation date with --xml."));

AddOption(STRING, "FORMAT_MASKREASONS_LINESKIP",
	"%{?WIDETERM}"
		" "
	"%{else}"
		"%{FORMAT_NEWLINE}(%{COLOR_MASKREASONS})"
	"%{}", P_("FORMAT_MASKREASONS_LINESKIP",
	"This string is printed as line separator in <maskreasons>."));

AddOption(STRING, "FORMAT_MASKREASONS_SEP",
	"%{?WIDETERM}"
		" - "
	"%{else}"
		"%{FORMAT_NEWLINE}%{FORMAT_NEWLINE}(%{COLOR_MASKREASONS})"
	"%{}", P_("FORMAT_MASKREASONS_SEP",
	"This string is printed as separator for different <maskreasons>."));

AddOption(STRING, "FORMAT_MASKREASONSS_LINESKIP",
	"%{?WIDETERM}"
		"%{FORMAT_VER_LINESKIP}\\C<%{I18N_COLUMN_AVAILABLE_CONTENT}>"
	"%{else}"
		"%{FORMAT_NEWLINE}"
	"%{}"
	"(%{COLOR_MASKREASONS})", P_("FORMAT_MASKREASONSS_LINESKIP",
	"This string is printed as line separator in <maskreasons*>."));

AddOption(STRING, "FORMAT_MASKREASONSS_SEP",
	"%{FORMAT_NEWLINE}%{FORMAT_MASKREASONSS_LINESKIP}", P_("FORMAT_MASKREASONSS_SEP",
	"This string is printed as separator for different <maskreasons*>."));

AddOption(STRING, "FORMAT_BEFORE_SET_USE",
	"(%{COLOR_SET_USE})", P_("FORMAT_BEFORE_SET_USE",
	"This string is printed before each set USE flag of an installed version."));

AddOption(STRING, "FORMAT_AFTER_SET_USE",
	"(%{COLOR_RESET})", P_("FORMAT_AFTER_SET_USE",
	"This string is printed after each set USE flag of an installed version."));

AddOption(STRING, "FORMAT_BEFORE_UNSET_USE",
	"(%{COLOR_UNSET_USE})-", P_("FORMAT_BEFORE_UNSET_USE",
	"This string is printed before each unset USE flag of an installed version."));

AddOption(STRING, "FORMAT_AFTER_UNSET_USE",
	"(%{COLOR_RESET})", P_("FORMAT_AFTER_UNSET_USE",
	"This string is printed after each unset USE flag of an installed version."));

AddOption(STRING, "FORMAT_BEFORE_USE_EXPAND_START",
	"(%{COLOR_USE_EXPAND_START})", P_("FORMAT_BEFORE_USE_EXPAND_START",
	"This string is printed before the variable name of an USE_EXPAND use value."));

AddOption(STRING, "FORMAT_BEFORE_USE_EXPAND_END",
	"=\"", P_("FORMAT_BEFORE_USE_EXPAND_END",
	"This string is printed after the variable name of an USE_EXPAND use value."));

AddOption(STRING, "FORMAT_AFTER_USE_EXPAND",
	"(%{COLOR_USE_EXPAND_START})\"(%{COLOR_USE_EXPAND_END})", P_("FORMAT_AFTER_USE_EXPAND",
	"This string is printed at the end of a USE_EXPAND use value."));

AddOption(STRING, "FORMAT_BEFORE_IUSE_EXPAND_START",
	"(%{COLOR_USE_EXPAND_START})", P_("FORMAT_BEFORE_IUSE_EXPAND_START",
	"This string is printed before the variable name of an USE_EXPAND iuse value."));

AddOption(STRING, "FORMAT_BEFORE_IUSE_EXPAND_END",
	"=\"(%{COLOR_VERSION_IUSE})", P_("FORMAT_BEFORE_IUSE_EXPAND_END",
	"This string is printed after the variable name of an USE_EXPAND iuse value."));

AddOption(STRING, "FORMAT_AFTER_IUSE_EXPAND",
	"(%{COLOR_USE_EXPAND_START})\"(%{COLOR_USE_EXPAND_END})", P_("FORMAT_AFTER_IUSE_EXPAND",
	"This string is printed at the end of a USE_EXPAND iuse value."));

AddOption(STRING, "FORMAT_BEFORE_COLL_EXPAND_START",
	"(%{COLOR_USE_EXPAND_START})", P_("FORMAT_BEFORE_COLL_EXPAND_START",
	"This string is printed before the variable name of an USE_EXPAND collected\n"
	"iuse value."));

AddOption(STRING, "FORMAT_BEFORE_COLL_EXPAND_END",
	"=\"(%{COLOR_COLL_IUSE})", P_("FORMAT_BEFORE_COLL_EXPAND_END",
	"This string is printed after the variable name of an USE_EXPAND collected\n"
	"iuse value."));

AddOption(STRING, "FORMAT_AFTER_COLL_EXPAND",
	"(%{COLOR_USE_EXPAND_START})\"(%{COLOR_USE_EXPAND_END})", P_("FORMAT_AFTER_COLL_EXPAND",
	"This string is printed at the end of a USE_EXPAND collected iuse value."));

AddOption(STRING, "FORMAT_BEFORE_IUSE",
	"{!*colliuse}\\t(%{COLOR_NORMAL})[(%{COLOR_NORMAL_END})", P_("FORMAT_BEFORE_IUSE",
	"This variable is only used for delayed substitution.\n"
	"This string is printed before IUSE data for a version is output.\n"
	"(Normally, this is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_AFTER_IUSE",
	"(%{COLOR_NORMAL})](%{COLOR_NORMAL_END})", P_("FORMAT_AFTER_IUSE",
	"This variable is only used for delayed substitution.\n"
	"This string is printed after IUSE data for a version is output.\n"
	"(Normally, this is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_BEFORE_REQUIRED_USE",
	"\\t(%{COLOR_NORMAL})[\"(%{COLOR_NORMAL_END})", P_("FORMAT_BEFORE_REQUIRED_USE",
	"This variable is only used for delayed substitution.\n"
	"This string is printed before REQUIRED_USE for a version is output.\n"
	"(Normally, this is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_AFTER_REQUIRED_USE",
	"(%{COLOR_NORMAL})\"](%{COLOR_NORMAL_END})", P_("FORMAT_AFTER_REQUIRED_USE",
	"This variable is only used for delayed substitution.\n"
	"This string is printed after REQUIRED_USE for a version is output.\n"
	"(Normally, this is only used when --versionlines is active)"));

AddOption(STRING, "FORMAT_BEFORE_COLL",
	"{*colliuse} (%{COLOR_USE_COLL})\\{", P_("FORMAT_BEFORE_COLL",
	"This variable is only used for delayed substitution.\n"
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is meant for printing after all versions in a line)"));

AddOption(STRING, "FORMAT_AFTER_COLL",
	"(%{COLOR_USE_COLL})\\}(%{COLOR_RESET})", P_("FORMAT_AFTER_COLL",
	"This variable is only used for delayed substitution.\n"
	"This string is printed after IUSE data for all versions is output.\n"
	"(This is meant for printing after all versions in a line)"));

AddOption(STRING, "FORMAT_BEFORE_COLL_SEP",
	"%{FORMAT_NEWLINE}\\C<%{COLUMN_USE}>(%{COLOR_USE_COLL})\\{", P_("FORMAT_BEFORE_COLL_SEP",
	"This variable is only used for delayed substitution.\n"
	"This string is printed before IUSE data for all versions is output.\n"
	"(This is meant for printing in a separate line)"));

AddOption(STRING, "FORMAT_AFTER_COLL_SEP",
	"%{FORMAT_AFTER_COLL}", P_("FORMAT_AFTER_COLL_SEP",
	"This variable is only used for delayed substitution.\n"
	"This string is printed after IUSE data for all versions is output.\n"
	"(This is meant for printing in a separate line)"));

AddOption(STRING, "COLOR_MASKED",
	"red;%{BG0S}|196;%{BG1}|red;%{BG2}|196;%{BG3}", P_("COLOR_MASKED",
	"This variable is only used for delayed substitution.\n"
	"It defines the color for masked versions."));

AddOption(STRING, "COLOR_UNSTABLE",
	"yellow;%{BG0S}|190;%{BG1}|blue;%{BG2}|21;%{BG3}", P_("COLOR_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color for unstable versions."));

AddOption(STRING, "COLOR_STABLE",
	"green;%{BG0S}|46;%{BG1}|;%{BG2}|58;%{BG3}", P_("COLOR_STABLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the color for stable versions."));

AddOption(STRING, "FORMAT_COLOR_MASKED",
	"(%{COLOR_MASKED})", P_("FORMAT_COLOR_MASKED",
	"This variable is only used for delayed substitution.\n"
	"It defines the format to change the color for masked versions."));

AddOption(STRING, "FORMAT_COLOR_UNSTABLE",
	"(%{COLOR_UNSTABLE})", P_("FORMAT_COLOR_UNSTABLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format to change the color for unstable versions."));

AddOption(STRING, "FORMAT_COLOR_STABLE",
	"(%{COLOR_STABLE})", P_("FORMAT_COLOR_STABLE",
	"This variable is only used for delayed substitution.\n"
	"It defines the format to change the color for stable versions."));

AddOption(STRING, "DUMMY",
	"", P_("DUMMY",
	"This variable is ignored. You can use it to collect delayed references to\n"
	"locally added (unused) variables so that they are printed with --dump."));

#endif
