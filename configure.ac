# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#  Wolfgang Frisch <xororand@users.sourceforge.net>
#  Emil Beinroth <emilbeinroth@gmx.net>
#  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

dnl The formatting convention used in this file is the following:
dnl
dnl After/before braces no space/newline is added (exception is for C++-code
dnl which always starts and ends with a newline and indent level 0;
dnl also an exception is of course where the space is needed, e.g. in
dnl AC_DEFINE* to get proper comments).
dnl Indents are done by tabs: Each indent level is one tab.
dnl
dnl Macro calls always start with an opening brace; possibly we just use ().
dnl All macro arguments are quoted, no matter whether it is necessary or not.
dnl (Local)/global sh variables are (non-)/capital and used with {} braces.
dnl Unless we use them as a command (like true/false) they are always quoted,
dnl no matter whether it is necessary or not.
dnl All strings are quoted in " " (even if ' ' could be used or is easier).
dnl
dnl We use M4SH instead of sh, i.e. no "if", "case", "&&", "||" from the sh
dnl but only the M4SH equivalents - this avoids some unexpected troubles.
dnl
dnl After the first macro arg, we start a new line and increase the
dnl indent level; all further args also start in a new line after the comma.
dnl Exception is the AS_CASE macro where value and action are on the same line,
dnl separated only with a space (and the indent level is increased for the
dnl action if it needs more than one line).
dnl Similar exceptions are macros where the arguments are closely connected
dnl (like calls of most MV_* macros defined later).
dnl
dnl Commas and closing braces are appended without leading space or newlines;
dnl in particular, no extra space is used even if several indent levels are
dnl closed. Of course, closing braces influence the indent level appropriately.
dnl In other words: For closing braces, we use lisp-type conventions.
dnl
dnl There are two exceptions of these rules:
dnl 1. If an argument has many items (like AC_CONFIG_FILES),
dnl    each item is listed in a separate line with indent level increased;
dnl    in such a case the opening/closing braces are isolated.
dnl 2. The AC_INIT macro must be in one line since it is parsed by
dnl    primitive scripts.
AC_INIT([eix], [0.18.3], [http://eix.sourceforge.net])
AC_PREREQ([2.61])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([
		Makefile
		manpage/Makefile
		src/Makefile
		doc/Makefile
		po/Makefile.in
	])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET()

AM_MAINTAINER_MODE()

# The following must appear after option parsing and before automake's
# initialization.
# Its purpose is to ignore user-specified --disable-dependency-tracking
# unless the user also explicitly specified --disable-silent-rules.
# It is not nice that we do this, because it is only documented here
# (not even shown with ./configure --help).
# However, it seems necessary, because --disable-dependency-tracking
# is planned to be default in Gentoo's EAPI=3, and to my big regret
# this option does no good, because it makes silent rules worthless to
# some extent. Since I do not know a way to exclude this option completely
# (or only eliminate its negative effects),this hack ignores this option.
# Although this hack uses undocumented behaviour, it is not that hackish:
# It only sets a variable, so I suppose that the worst thing which can
# happen is that it is ignored...
AS_IF([test x"${enable_silent_rules}" != x"no"],
	[AS_CASE(["${enable_dependency_tracking}"],
		[yes], [],
		[no], [AC_MSG_WARN([Ignoring --disable-dependency-tracking as it interferes with silent rules])],
		[AC_MSG_NOTICE([Trying to enable dependency tracking for consistency with silent rules])])
	enable_dependency_tracking=yes])

AM_INIT_AUTOMAKE()
m4_ifdef([AM_SILENT_RULES],
	[AM_SILENT_RULES([yes])])

AC_LANG_CPLUSPLUS()
AC_PROG_CXX()

# Now our flag mangling options:

AC_ARG_ENABLE([debugging],
	[AS_HELP_STRING([--enable-debugging],
		[set flags appropriate for debugging])],
	[MV_ENABLE([debugging], [debugging])],
	[debugging=false])

AC_ARG_ENABLE([optimization],
	[AS_HELP_STRING([--disable-optimization],
		[do not append optimization flags])],
	[MV_ENABLE([optimization], [optimization])],
	[optimization=true])

AC_ARG_ENABLE([strong_optimization],
	[AS_HELP_STRING([--enable-strong-optimization],
		[use also optimization flags which might not work on broken compilers])],
	[MV_ENABLE([strong_optimization], [strong_optimization])],
	[strong_optimization=false])

AC_ARG_ENABLE([warnings],
	[AS_HELP_STRING([--enable-warnings],
		[append warning/testing flags; might produce worse code])],
	[MV_ENABLE([warnings], [warnings])],
	[warnings=false])

AC_ARG_ENABLE([strong_warnings],
	[AS_HELP_STRING([--enable-strong-warnings],
		[append flags to get lots of mostly unimportant warnings])],
	[MV_ENABLE([strong_warnings], [strong_warnings])],
	[strong_warnings=false])

AC_ARG_ENABLE([security],
	[AS_HELP_STRING([--disable-security],
		[do not append security relevant options to flags])],
	[MV_ENABLE([security], [security])],
	[security=true])

AC_ARG_ENABLE([quickcheck],
	[AS_HELP_STRING([--enable-quickcheck],
		[do not check whether flags work when adding them])],
	[MV_ENABLE([quickcheck], [quickcheck])],
	[quickcheck=false])

# Some implicit dependencies of these options.
dnl We could mix this with the cases later on, but for clarity we do it here:
AS_IF([${strong_optimization}],
	[optimization=true])
AS_IF([${strong_warnings}],
	[warnings=true])
AS_IF([${debugging}],
	[optimization=false
	strong_optimization=false])
AS_IF([${warnings}],
	[security=true])

# Now start the flag mangling:
AS_IF([${debugging}],
	[CFLAGS=""
	CXXFLAGS=""
	LDFLAGS=""])
mycxxflagstext="${CXXFLAGS}"
myldflagstext="${LDFLAGS}"

# Note that the (potential) resetting of the flags was necessary *before*
# (potentially) modifying flags for system extensions.
# On the other hand, we must check for system extensions before we compile
# test programs to find out the other flags.
AC_USE_SYSTEM_EXTENSIONS()

# Only now the flag mangling can continue:
#
# At first, we must calculate our [fatal-flags] and [mode] arguments
mycxxfatal=""
myldfatal=""
AS_IF([${quickcheck}],
	[quicktrue=true
	quickfalse=false],
	[quicktrue=""
	quickfalse=""
	MV_ADDFLAGS([mycxxfatal], [CXXFLAGS], [COMPILE], [ \
			-Werror \
		])
	MV_ADDFLAGS([myldfatal], [LDFLAGS], [LINK], [ \
			-Wl,--fatal-warnings \
		])])

# Now the actual testing:
mycxxadd=""
myldadd=""
AS_IF([${debugging}],
	[MV_ADDFLAGS([mycxxadd], [CXXFLAGS], [COMPILE], [ \
			-g \
			-ggdb3 \
		], [${mycxxfatal}], [${quicktrue}])])
AS_IF([${optimization}],
	[MV_ADDFLAGS([mycxxadd], [CXXFLAGS], [COMPILE], [ \
			-fomit-frame-pointer \
			-frename-registers \
			-fstrict-aliasing \
			-fmerge-all-constants \
			-funsafe-loop-optimizations \
			-finline-functions \
			-ffast-math \
			-fgcse-sm \
			-fgcse-las \
			-fgcse-after-reload \
			-fpredictive-commoning \
			-ftree-switch-conversion \
			-fno-ident \
			-fvisibility=hidden \
			-fvisibility-inlines-hidden \
			-fno-enforce-eh-specs \
			-DNDEBUG \
			-DNO_DEBUG \
			-DG_DISABLE_ASSERT \
		], [${mycxxfatal}], [${quicktrue}])
	MV_ADDFLAGS([myldadd], [LDFLAGS], [LINK], [ \
			-Wl,-O1 \
			-Wl,--relax \
			-Wl,--hash-style=gnu \
			-Wl,--enable-new-dtags \
			-Wl,--as-needed \
			-Wl,--sort-common \
			-Wl,-z,combreloc \
		], [${myldfatal}], [${quicktrue}])])
AS_IF([${strong_optimization}],
	[MV_ADDFLAGS([mycxxadd], [CXXFLAGS], [COMPILE], [ \
			-fweb \
			-ftree-vectorize \
			-fgraphite \
			-floop-interchange \
			-floop-strip-mine \
			-floop-block \
		], [${mycxxfatal}], [${quicktrue}])
	MV_ADDFLAGS([myldadd], [LDFLAGS], [LINK], [ \
			-Wl,-O9 \
		], [${myldfatal}], [${quicktrue}])])
AS_IF([${warnings}],
	[security=true
	MV_ADDFLAGS([mycxxadd], [CXXFLAGS], [COMPILE], [ \
			-fdirectives-only \
			-funsigned-char \
			-fPIC \
			-ftracer \
			-pedantic \
			-D_FILE_OFFSET_BITS=64 \
			-D_LARGEFILE_SOURCE \
			-D_LARGEFILE64_SOURCE \
			-D_GLIBCXX_DEBUG \
			-Wall \
			-Wextra \
			-Wformat=2 \
			-Winit-self \
			-Wmissing-include-dirs \
			-Wswitch-default \
			-Wunused \
			-Wunused-parameter \
			-Wstrict-aliasing=1 \
			-Wstrict-overflow=5 \
			-Wfloat-equal \
			-Wundef \
			-Wshadow \
			-Wpointer-arith \
			-Wcast-qual \
			-Wcast-align \
			-Wwrite-strings \
			-Wsign-compare \
			-Wmissing-field-initializers \
			-Wnormalized=nfkc \
			-Wpacked \
			-Wredundant-decls \
			-Winvalid-pch \
			-Wlong-long \
			-Wvolatile-register-var \
			-Wdisabled-optimization \
			-Wlogical-op \
			-Wvla \
			-Wabi \
			-Wctor-dtor-privacy \
			-Wstrict-null-sentinel \
			-Woverloaded-virtual \
			-Wsign-promo \
			-Wmissing-declarations \
			-Wmissing-format-attribute \
		], [${mycxxfatal}], [${quicktrue}])
dnl			-Wtraditional \
dnl			-Wc++-compat \
dnl			-Wstrict-prototypes \
dnl			-Wold-style-definition \
dnl			-Wmissing-prototypes \
dnl			-Wnested-externs \
dnl			-Wpointer-sign \
	MV_ADDFLAGS([myldadd], [LDFLAGS], [LINK], [ \
			-Wl,-z,defs \
			-Wl,--no-undefined \
			-Wl,--no-allow-shlib-undefined \
			-Wl,--no-undefined-version \
			-Wl,--warn-common \
			-Wl,--fatal-warnings \
		], [${myldfatal}], [${quicktrue}])])
AS_IF([${strong_warnings}],
	[MV_ADDFLAGS([mycxxadd], [CXXFLAGS], [COMPILE], [ \
			-Wsign-conversion \
			-Wold-style-cast \
			-Wmissing-noreturn \
		], [${mycxxfatal}], [${quicktrue}])])
dnl			-Wconversion \
dnl			-Wswitch-enum \
dnl			-Wunsafe-loop-optimizations \
dnl			-Wpadded \
dnl			-Wunreachable-code \
dnl			-Waggregate-return \
dnl			-Winline \
dnl			-Weffc++ \
AS_IF([${security}],
	[MV_ADDFLAGS([mycxxadd], [CXXFLAGS], [COMPILE], [ \
			-D_FORTIFY_SOURCE=2 \
			-fstack-protector \
		], [${mycxxfatal}], [${quicktrue}])
	MV_ADDFLAGS([myldadd], [LDFLAGS], [LINK], [ \
			-Wl,-z,now \
			-Wl,-z,relro \
		], [${myldfatal}], [${quicktrue}])])

# We are done with testing, the real flag mangling takes place:
MV_PREPEND([CXXFLAGS], [${mycxxadd}])
MV_PREPEND([LDFLAGS], [${myldadd}])
MV_APPEND([mycxxflagstext], [${mycxxadd}], [
CXXFLAGS_PREPEND: ])
MV_APPEND([myldflagstext], [${myldadd}], [
LDFLAGS_PREPEND: ])

# For --enable-quickcheck, we do at least one (collected) safety check;
# otherwise, the error messages may be too confusing for the user:
AS_IF([${quickcheck}],
	[AC_MSG_CHECKING([whether quickcheck mode was appropriate])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])
		echo "export CXXFLAGS='${CXXFLAGS}'"
		echo "export LDFLAGS='${LDFLAGS}'"
		AC_MSG_ERROR([Compiling/linking a simple program failed. Try --disable-quickcheck])])])

AC_PROG_LN_S()

# Unfortunately, gettext requires libtool on some systems:
AC_LIBTOOL_DLOPEN()
AC_PROG_LIBTOOL()
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

# Provide basic types:
AC_TYPE_UINT8_T()
AC_TYPE_UINT16_T()
AC_TYPE_UINT32_T()

# This must be an early test, since we need the null device later in this file:
AC_MSG_CHECKING([for null-device])
AC_ARG_WITH([dev-null],
	[AS_HELP_STRING([--with-null-device=STR],
		[null device of the system, usually /dev/null])],
	[DEV_NULL="${withval}"],
	[DEV_NULL=""])
MV_IF_EMPTY([${DEV_NULL}],
	[DEV_NULL="/dev/null"])
AC_MSG_RESULT([${DEV_NULL}])
AC_DEFINE_UNQUOTED([DEV_NULL],
	["${DEV_NULL}"],
	[ Null device of the system])
AC_SUBST([DEV_NULL])

# Separate binaries?
AC_ARG_ENABLE([separate_binaries],
	[AS_HELP_STRING([--enable-separate-binaries],
		[create a separate binary for each program])],
	[MV_ENABLE([separate_binaries], [separate-binaries])],
	[separate_binaries=false])
AM_CONDITIONAL([SEPARATE_BINARIES],
	[${separate_binaries}])

# Separate eix-update?
AC_ARG_ENABLE([separate_update],
	[AS_HELP_STRING([--enable-separate-update],
		[create a separate eix-update binary])],
	[MV_ENABLE([separate_update], [separate-update])],
	[separate_update=false])
AS_IF([${separate_binaries}],
	[separate_update=true])
AM_CONDITIONAL([SEPARATE_UPDATE],
	[${separate_update}])

# Separate tools?
AC_ARG_ENABLE([separate_tools],
	[AS_HELP_STRING([--enable-separate-tools],
		[create separate binaries for tools (versionsort)])],
	[MV_ENABLE([separate_tools], [separate-tools])],
	[separate_tools=false])
AM_CONDITIONAL([SEPARATE_TOOLS],
	[${separate_tools}])

# Symlinks for obsolete names?
AC_ARG_ENABLE([obsolete_symlinks],
	[AS_HELP_STRING([--enable-obsolete-symlinks],
		[create a separate binary for each program])],
	[MV_ENABLE([obsolete_symlinks], [obsolete-symlinks])],
	[obsolete_symlinks=false])
AM_CONDITIONAL(OBSOLETE_SYMLINKS,
	[${obsolete_symlinks}])

# Wrappers for obsolete names?
AC_ARG_ENABLE([obsolete_reminder],
	[AS_HELP_STRING([--enable-obsolete-reminder],
		[create a separate binary for each program])],
	[MV_ENABLE([obsolete_reminder], [obsolete-reminder])],
	[obsolete_reminder=false])
AM_CONDITIONAL([OBSOLETE_REMINDER],
	[${obsolete_reminder}])

# Look for basic functions:
AC_FUNC_FSEEKO()
AC_CHECK_FUNCS([regcomp strchr strdup strerror strrchr fnmatch memset],
	[],
	[AC_MSG_ERROR([We really need this function ..])])

# We need at least one of the following, the former the better
AC_CHECK_FUNCS([strtoul strtol atoi],
	[break],
	[AC_MSG_ERROR([At least one of strtoul/strtoll/atoi is needed ..])])

# Well define the following if we don't have one:
AC_CHECK_FUNCS([strndup])

# We use these optionally:
AC_CHECK_FUNCS([canonicalize_file_name realpath vfork setenv])

# Optional header files:
AC_CHECK_HEADERS([sys/param.h])

# Not sure whether this is optional; however it probably exists on all systems
AC_CHECK_HEADERS([unistd.h])

# If climits exists, we need not check for limits.h
AC_CHECK_HEADERS([climits limits.h],
	[break])

# If tr1/cstdint exists we need not check for stdint.h.
AC_CHECK_HEADERS([tr1/cstdint cstdint stdint.h],
	[break])

# Standard programs:
AC_CHECK_PROGS([regex_cmd],
	[sed],
	[no])
AS_CASE(["${regex_cmd}"],
	[no], [AC_MSG_ERROR([sed is required])])

# Use bzip2?
AC_MSG_CHECKING([whether libbz2 should be used])
AC_ARG_WITH([bzip2],
	[AS_HELP_STRING([--with-bzip2],
		[Use libbz2 (required for detecting installed overlays)])],
	[AS_CASE(["${withval}"],
		[no], [use_bzlib=false
			MV_MSG_RESULT([no], [on request])],
		[use_bzlib=true])],
	[use_bzlib=""])
AS_IF([test x"${use_bzlib}" != x"false"],
	[BZLIB_LIBS="-lbz2"
	myoldlibs="${LIBS}"
	LIBS="${LIBS} ${BZLIB_LIBS}"
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <bzlib.h>
		]],
		[[
BZ2_bzopen("dummy","rb");
		]])],
		[MV_IF_EMPTY([${use_bzlib}],
			[MV_MSG_RESULT([yes], [autodetected])],
			[MV_MSG_RESULT([yes], [on request])])
		use_bzlib=true],
		[MV_MSG_RESULT([no], [autodetected])
		MV_IF_NONEMPTY([${use_bzlib}],
			[AC_MSG_ERROR([bzip2 requested but libbz2 is unusable])])
		use_bzlib=false
		BZLIB_LIBS=""])
	LIBS="${myoldlibs}"])
AS_IF([${use_bzlib}],
	[AC_DEFINE([USE_BZLIB],
		[1],
		[ Define to 1 if bzlib can be used])])
AC_SUBST(BZLIB_LIBS)

# Use rst?
AC_ARG_WITH([rst],
	[AS_HELP_STRING([--with-rst],
		[Use rst2html.py to document cache file format in html])],
	[AS_CASE(["${withval}"],
		[no], [rst=false],
		[rst=true])],
	[rst=false])
AM_CONDITIONAL([USE_RST], [${rst}])

# Use obsoleted cache method?
AC_ARG_ENABLE([extra_cache],
	[AS_HELP_STRING([--enable-extra-cache],
		[enable unsupported cache method portage-2.1])],
	[MV_ENABLE([extra_cache], [extra-cache])],
	[extra_cache=false])
AS_IF([${extra_cache}],
	[AC_MSG_WARN([Unsupported cache method portage-2.1 enabled on request])])
AM_CONDITIONAL([EXTRA_CACHE],
	[${extra_cache}])

# Check if std::unique works
AC_MSG_CHECKING([whether std::unique seems to work])
AC_ARG_ENABLE([unique],
	[AS_HELP_STRING([--enable-unique],
		[assume that unique works])],
	[MV_ENABLE([unique_works], [unique-works])
	MV_MSG_RESULT([${enableval}], [skipped test])],
	[AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>
		]],
		[[
const char *a[] = {
	"1", "2", "2", "3", "3", "4", "4", "5", NULL,
	"1", "1", "2", "3", "4", "5", "5", "5", NULL,
	"1", "2", "3", "4", "5", NULL,
	NULL };
const char *b[]= {
	"1", "2", "3", "4", "5", NULL };
typedef std::vector<std::string>::size_type ind;
for(ind i = 0; a[i]; i++) {
	std::vector<std::string> t;
	while(a[i])
		t.push_back(a[i++]);
	t.erase(unique(t.begin(), t.end()), t.end());
	for(ind j = 0; ; j++) {
		if(j == t.size()) {
			if(b[j])
				return 1;
			break;
		}
		if(!b[j])
			return 1;
		if(t[j] != b[j])
			return 1;
	}
}
		]])],
		[AC_MSG_RESULT([yes])
		unique_works=true],
		[AC_MSG_RESULT([no])
		unique_works=false],
		[MV_MSG_RESULT([assuming yes], [crosscompile guess])
		unique_works=true])])
AS_IF([${unique_works}],
	[AC_DEFINE([UNIQUE_WORKS],
		[1],
		[ Define to 1 if std::unique() works reliable])])

# What about sqlite?
AC_MSG_CHECKING([whether sqlite should be used])
SQLITE_LIBS=""
AC_ARG_WITH([sqlite],
	[AS_HELP_STRING([--with-sqlite],
		[Compile in support for cache method sqlite])],
	[AS_CASE(["${withval}"],
		[no], [MV_MSG_RESULT([no], [on request])],
		[MV_MSG_RESULT([yes], [on request])
		AS_IF([eval command -v pkg-config >${DEV_NULL} 2>&1],
			[SQLITE_LIBS="`pkg-config --libs sqlite3`"])
		# If pkg-config is not available or failed, we use a default
		MV_IF_EMPTY([${SQLITE_LIBS}],
			[AC_MSG_WARN([ignoring pkg-config and using -lsqlite3])
			SQLITE_LIBS="-lsqlite3"])])],
	[# If autodetection of sqlite is necessary, we rely on pkg-config
	AS_IF([eval command -v pkg-config >${DEV_NULL} 2>&1],
		[SQLITE_LIBS="`eval pkg-config --libs sqlite3 2>${DEV_NULL}`"])
	MV_IF_EMPTY([${SQLITE_LIBS}],
		[MV_MSG_RESULT([no], [autodetected])],
		[MV_MSG_RESULT([yes], [autodetected])])])
MV_IF_NONEMPTY([${SQLITE_LIBS}],
	[AC_DEFINE([WITH_SQLITE],
		[1],
		[ Define to 1 if cache method sqlite is wanted])])
AC_SUBST([SQLITE_LIBS])

AC_MSG_CHECKING([PORTDIR_CACHE_METHOD default])
AC_ARG_WITH([portdir-cache-method],
	[AS_HELP_STRING([--with-portdir-cache-method=STR],
		[default PORTDIR_CACHE_METHOD])],
	[PORTDIR_CACHE_METHOD="${withval}"
	description="on request"],
	[PORTDIR_CACHE_METHOD="metadata-flat"
	description="default"])
MV_MSG_RESULT([${PORTDIR_CACHE_METHOD}], [${description}])
AC_DEFINE_UNQUOTED([PORTDIR_CACHE_METHOD],
	["${PORTDIR_CACHE_METHOD}"],
	[ Default PORTDIR_CACHE_METHOD])
AC_SUBST([PORTDIR_CACHE_METHOD])

AC_MSG_CHECKING([EPREFIX default])
AC_ARG_WITH([eprefix-default],
	[AS_HELP_STRING([--with-eprefix-default=STR],
		[default EPREFIX])],
	[EPREFIX_DEFAULT="${withval}"],
	[EPREFIX_DEFAULT="`eval portageq envvar EPREFIX 2>${DEV_NULL}`"])
MV_PREFIX(EPREFIX_DEFAULT)
MV_MSG_RESULTVAR([${EPREFIX_DEFAULT}])
AC_DEFINE_UNQUOTED([EPREFIX_DEFAULT],
	["${EPREFIX_DEFAULT}"],
	[ Default EPREFIX])
AC_SUBST([EPREFIX_DEFAULT])

AC_MSG_CHECKING([EIX_PREFIX default])
AC_ARG_WITH([eix-prefix-default],
	[AS_HELP_STRING([--with-eix-prefix-default=STR],
		[default EIX_PREFIX])],
	[EIX_PREFIX_DEFAULT="${withval}"
		AS_IF([test x"${EIX_PREFIX_DEFAULT}" = x"/"],
			[EIX_PREFIX_DEFAULT=""])],
	[EIX_PREFIX_DEFAULT=""])
MV_PREFIX(EPREFIX_DEFAULT)
MV_MSG_RESULTVAR([${EIX_PREFIX_DEFAULT}])
AC_DEFINE_UNQUOTED([EIX_PREFIX_DEFAULT],
	["${EIX_PREFIX_DEFAULT}"],
	[ Default EIX_PREFIX])
AC_SUBST([EIX_PREFIX_DEFAULT])

AC_MSG_CHECKING([ROOT default])
AC_ARG_WITH([root-default],
	[AS_HELP_STRING([--with-root-default=STR],
		[default ROOT])],
	[ROOT_DEFAULT="${withval}"
		AS_IF([test x"${ROOT_DEFAULT}" = x"/"],
			[ROOT_DEFAULT=""])],
	[ROOT_DEFAULT=""])
MV_PREFIX(ROOT_DEFAULT)
MV_MSG_RESULTVAR([${ROOT_DEFAULT}])
AC_DEFINE_UNQUOTED([ROOT_DEFAULT],
	["${ROOT_DEFAULT}"],
	[ Default ROOT])
AC_SUBST([ROOT_DEFAULT])

AC_MSG_CHECKING([EXEC_EBUILD_SH default])
AC_ARG_WITH([ebuild-sh-default],
	[AS_HELP_STRING([--with-ebuild-sh-default=STR],
		[default EXEC_EBUILD_SH])],
	[EBUILD_SH_DEFAULT="${withval}"
		description="on request"],
	[EBUILD_SH_DEFAULT="/usr/lib/portage/bin/ebuild.sh"
	AS_IF([test -d "${EPREFIX_DEFAULT}/usr/lib"],
		[],
		AS_IF([test -d "${EPREFIX_DEFAULT}/usr/lib64"],
			[EBUILD_SH_DEFAULT="/usr/lib64/portage/bin/ebuild.sh"]))
	AS_IF([test -x "${EPREFIX_DEFAULT}${EBUILD_SH_DEFAULT}"],
		[description="autodetected"],
		[description="guessed"])])
MV_PREFIX(EBUILD_SH_DEFAULT)
MV_MSG_RESULTVAR([${EBUILD_SH_DEFAULT}], [${description}])
AC_DEFINE_UNQUOTED([EBUILD_SH_DEFAULT],
	["${EBUILD_SH_DEFAULT}"],
	[ Default EXEC_EBUILD_SH (without prefix)])
AC_SUBST([EBUILD_SH_DEFAULT])

AC_MSG_CHECKING([PORTAGE_ROOTPATH default])
AC_ARG_WITH([portage-rootpath],
	[AS_HELP_STRING([--with-portage-rootpath=STR],
		[default PORTAGE_ROOTPATH])],
	[PORTAGE_ROOTPATH_DEFAULT="${withval}"],
	[PORTAGE_ROOTPATH_DEFAULT=""])
MV_MSG_RESULTVAR([${PORTAGE_ROOTPATH_DEFAULT}])
AC_DEFINE_UNQUOTED([PORTAGE_ROOTPATH_DEFAULT],
	["${PORTAGE_ROOTPATH_DEFAULT}"],
	[ Default PORTAGE_ROOTPATH])
AC_SUBST([PORTAGE_ROOTPATH_DEFAULT])

AC_MSG_CHECKING([PORTAGE_BIN_PATH default])
AC_ARG_WITH([portage-bin-path],
	[AS_HELP_STRING([--with-portage-bin-path=STR],
		[default PORTAGE_BIN_PATH])],
	[PORTAGE_BIN_PATH_DEFAULT="${withval}"],
	[PORTAGE_BIN_PATH_DEFAULT=""])
MV_MSG_RESULTVAR([${PORTAGE_BIN_PATH_DEFAULT}])
AC_DEFINE_UNQUOTED([PORTAGE_BIN_PATH_DEFAULT],
	["${PORTAGE_BIN_PATH_DEFAULT}"],
	[ Default PORTAGE_BIN_PATH])
AC_SUBST([PORTAGE_BIN_PATH_DEFAULT])

AC_MSG_CHECKING([default remote path])
AC_ARG_WITH([remote-file],
	[AS_HELP_STRING([--with-remote-file=STR],
		[default remote file for eix-remote])],
	[DEFAULT_REMOTE_PATH="${withval}"],
	[DEFAULT_REMOTE_PATH="http://dev.gentooexperimental.org/eix_cache/eix-caches.tbz2"])
MV_MSG_RESULTVAR([${DEFAULT_REMOTE_PATH}])
AC_SUBST([DEFAULT_REMOTE_PATH])

# Check endianess
AC_C_BIGENDIAN()

EIX_CACHEFILE="/var/cache/eix"
AC_DEFINE_UNQUOTED([EIX_CACHEFILE],
	["${EIX_CACHEFILE}"],
	[ Location of the cachefile])
AC_SUBST([EIX_CACHEFILE])

AC_DEFINE_UNQUOTED([LEVENSHTEIN_DISTANCE_DEFAULT],
	["2"],
	[ Default Levenshtein distance for matches (as string)])
AC_SUBST([LEVENSHTEIN_DISTANCE])

AC_DEFINE_UNQUOTED([TARGET],
	["${target}"],
	[ Target system])

MV_IF_NONEMPTY([${GCC}],
	[AC_MSG_CHECKING([gcc version])
	gcc_v=`${CC} -dumpversion`
	AC_MSG_RESULT([${gcc_v}])
	AC_DEFINE_UNQUOTED([GCC_VERSION],
		["${gcc_v}"],
		[ Version of gcc])])

AC_SUBST([PACKAGE_STRING])

# Done!
AC_OUTPUT()

echo "

${PACKAGE}-${VERSION} configured successfully.

CXXFLAGS: ${mycxxflagstext}

LDFLAGS:  ${myldflagstext}

"
