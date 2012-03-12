# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#  Wolfgang Frisch <xororand@users.sourceforge.net>
#  Emil Beinroth <emilbeinroth@gmx.net>
#  Martin V\"ath <vaeth@mathematik.uni-wuerzburg.de>

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
dnl Unless we use them as a command (like :/false) they are always quoted,
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
AC_INIT([eix], [0.25.3], [http://developer.berlios.de/projects/eix/], [eix], [http://eix.berlios.de/])
AC_PREREQ([2.64])

AC_CONFIG_AUX_DIR([config])
AC_CONFIG_FILES([
		Makefile
		manpage/Makefile
		src/Makefile
		doc/Makefile
		po/Makefile.in
		zsh/Makefile
	])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET()

AM_MAINTAINER_MODE()
AM_INIT_AUTOMAKE([no-dependencies])
AM_SILENT_RULES([yes])

AC_LANG([C++])
AC_PROG_CXX()

AC_DEFINE_UNQUOTED([TARGET],
	["${target}"],
	[Target system])

AS_VAR_SET([my_gcc_v], [])
AS_VAR_IF([GXX], [yes],
	[AC_MSG_CHECKING([gcc version])
	AS_VAR_SET([my_gcc_v], [`"${CXX}" -dumpversion`])
	MV_IF_EMPTY([my_gcc_v],
		[AS_VAR_SET([my_gcc_v], ["unknown"])])
	MV_MSG_RESULT_VAR([my_gcc_v])
	AC_DEFINE_UNQUOTED([GCC_VERSION],
		["${my_gcc_v}"],
		[Version of gcc])])

# Now our flag mangling options:

AC_ARG_ENABLE([debugging],
	[AS_HELP_STRING([--enable-debugging],
		[set flags appropriate for debugging])],
	[MV_ENABLE([debugging])
	AS_VAR_SET([cmt_debugging], ["on request"])],
	[AS_VAR_SET([debugging], [false])
	AS_VAR_SET([cmt_debugging], ["default"])])

AC_ARG_ENABLE([security],
	[AS_HELP_STRING([--enable-security],
		[append security relevant options to flags])],
	[MV_ENABLE([security])
	AS_VAR_SET([cmt_security], ["on request"])],
	[AS_VAR_SET([security], [false])
	AS_VAR_SET([cmt_security], ["default"])])

AC_ARG_ENABLE([optimization],
	[AS_HELP_STRING([--enable-optimization],
		[append optimization flags])],
	[MV_ENABLE([optimization])
	AS_VAR_SET([cmt_optimization], ["on request"])],
	[AS_VAR_SET([optimization], [false])
	AS_VAR_SET([cmt_optimization], ["default"])])

AC_ARG_ENABLE([strong_optimization],
	[AS_HELP_STRING([--enable-strong-optimization],
		[use also optimization flags which might not work on broken compilers])],
	[MV_ENABLE([strong-optimization])
	AS_VAR_SET([cmt_strong_optimization], ["on request"])],
	[AS_VAR_SET([strong_optimization], [false])
	AS_VAR_SET([cmt_strong_optimization], ["default"])])

AC_ARG_ENABLE([warnings],
	[AS_HELP_STRING([--enable-warnings],
		[append warning/testing flags; might produce worse code])],
	[MV_ENABLE([warnings])
	AS_VAR_SET([cmt_warnings], ["on request"])],
	[AS_VAR_SET([warnings], [false])
	AS_VAR_SET([cmt_warnings], ["default"])])

AC_ARG_ENABLE([strong_warnings],
	[AS_HELP_STRING([--enable-strong-warnings],
		[append flags to get lots of mostly unimportant warnings])],
	[MV_ENABLE([strong-warnings])
	AS_VAR_SET([cmt_strong_warnings], ["on request"])],
	[AS_VAR_SET([strong_warnings], [false])
	AS_VAR_SET([cmt_strong_warnings], ["default"])])

AC_ARG_ENABLE([quickcheck],
	[AS_HELP_STRING([--enable-quickcheck],
		[do not check whether flags work when adding them])],
	[MV_ENABLE([quickcheck])
	AS_VAR_SET([cmt_quickcheck], ["on request"])],
	[AS_VAR_SET([quickcheck], [false])
	AS_VAR_SET([cmt_quickcheck], ["default"])])

# Some implicit dependencies of these options.
# We do this here explicitly and output the results

AC_MSG_CHECKING([whether debugging options should be used])
MV_MSG_RESULT_BIN([${debugging}], [${cmt_debugging}])

AC_MSG_CHECKING([whether strong optimization options should be used])
AS_IF([${debugging}],
	[AS_VAR_SET([strong_optimization], [false])
	AS_VAR_SET([cmt_strong_optimization], ["disabled by debugging"])])
MV_MSG_RESULT_BIN([${strong_optimization}], [${cmt_strong_optimization}])

AC_MSG_CHECKING([whether optimization options should be used])
AS_IF([${strong_optimization}],
	[AS_VAR_SET([optimization], [:])
	AS_VAR_SET([cmt_optimization], ["forced by strong optimization"])])
AS_IF([${debugging}],
	[AS_VAR_SET([optimization], [false])
	AS_VAR_SET([cmt_optimization], ["disabled by debugging"])])
MV_MSG_RESULT_BIN([${optimization}], [${cmt_optimization}])

AC_MSG_CHECKING([whether strong warning options should be used])
MV_MSG_RESULT_BIN([${strong_warnings}], [${cmt_strong_warnings}])

AC_MSG_CHECKING([whether warning options should be used])
AS_IF([${strong_warnings}],
	[AS_VAR_SET([warnings], [:])
	AS_VAR_SET([cmt_warnings], ["forced by strong warnings"])])
MV_MSG_RESULT_BIN([${warnings}], [${cmt_warnings}])

AC_MSG_CHECKING([whether security options should be used])
MV_MSG_RESULT_BIN([${security}], [${cmt_security}])

AC_MSG_CHECKING([whether options should not be tested individually])
MV_MSG_RESULT_BIN([${quickcheck}], [${cmt_quickcheck}])

# Now start the flag mangling:
AS_IF([${debugging}],
	[AS_VAR_SET([CFLAGS], [])
	AS_VAR_SET([CXXFLAGS], [])
	AS_VAR_SET([LDFLAGS], [])])
AS_VAR_COPY([oricxxflags], [CXXFLAGS])
AS_VAR_COPY([orildflags], [LDFLAGS])

# Note that the (potential) resetting of the flags was necessary *before*
# (potentially) modifying flags for system extensions.
# On the other hand, we must check for system extensions before we compile
# test programs to find out the other flags.
AC_USE_SYSTEM_EXTENSIONS()

# Only now the flag mangling can continue:
#
# At first, we must calculate our [fatal-flags] and [mode] arguments
AS_IF([${quickcheck}],
	[AS_VAR_SET([quicktrue], [:])
	AS_VAR_SET([quickfalse], [false])],
	[AS_VAR_SET([quicktrue], [])
	AS_VAR_SET([quickfalse], [])])

AS_VAR_SET([my_cxxfatal], [])
AS_VAR_SET([my_ldfatal], [])
MV_ADDFLAGS([my_cxxfatal], [CXXFLAGS], [COMPILE], [ \
		-Werror \
	], [], [], [${quicktrue}])
MV_ADDFLAGS([my_ldfatal], [LDFLAGS], [LINK], [ \
		-Wl,--fatal-warnings \
	], [], [], [${quicktrue}])

# Now the actual testing:
AS_VAR_SET([my_cxxadd], [])
AS_VAR_SET([my_ldadd], [])
AS_IF([${debugging}],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [RUN], [ \
			-g \
			-ggdb3 \
		], [${my_cxxfatal}], [], [${quicktrue}])])
AS_IF([${optimization}],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [RUN], [ \
			-O2 \
			-fomit-frame-pointer \
			-fno-common \
			-fnothrow-opt \
			-frename-registers \
			-fstrict-aliasing \
			-fmerge-all-constants \
			-funsafe-loop-optimizations \
			-finline-functions \
			-fgcse-sm \
			-fgcse-las \
			-fgcse-after-reload \
			-fpredictive-commoning \
			-ftree-switch-conversion \
			-fno-ident \
			-fvisibility=hidden \
			-fvisibility-inlines-hidden \
		], [${my_cxxfatal}], [], [${quicktrue}])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [RUN], [ \
			-Wl,-O1 \
			-Wl,--relax \
			-Wl,--as-needed \
			-Wl,--sort-common \
			-Wl,-z,combreloc \
		], [${my_ldfatal}], [], [${quicktrue}])])
AS_IF([${strong_optimization}],
	AS_VAR_SET([flto], ["-flto -flto-partition=none"])
	AS_CASE(["${my_gcc_v}"],
		["4.6.0"],
			[AC_MSG_WARN([Not forcing -flto: It seems broken in gcc-${my_gcc_v}])
			AS_VAR_SET([flto], [])])
# We first modify the LDFLAGS: This is important because of -flto:
# Without -flto in LDFLAGS, -flto in CXXFLAGS might not fail even if it
# should produce broken code.
	[MV_ADDFLAGS([my_ldadd], [LDFLAGS], [RUN], [ \
			-Wl,-O9 \
			${flto} \
			-fwhole-program \
			-Wl,-z,noexecstack \
		], [${my_ldfatal}], [], [${quicktrue}])
dnl The following is never tried, since the run can succeed but the result
dnl may fail for some unknown reason on non-GNU systems, see
dnl http://bugs.gentoo.org/show_bug.cgi?id=293263
dnl			-Wl,--hash-style=gnu \
dnl			-Wl,--enable-new-dtags \
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [RUN], [ \
			${flto} \
		], [${my_cxxfatal}], [], [${quicktrue}])])
# If -flto is used during compilation, we can also use -fwhole-program during
# compilation and should specify -flto during linking.
# However, -Wl,--warn-common will then give lots of warnings, so we
# do not add the latter unless we use strong_warnings.
AS_VAR_SET([fwhole_program], [])
AS_VAR_SET([warn_common], [-Wl,--warn-common])
AS_CASE([" ${CXXFLAGS} ${my_cxxadd} "],
	[*" -flto "*],
		[AS_VAR_SET([fwhole_program], [-fwhole-program])
		AS_IF([${strong_warnings}],
			[],
			[AS_VAR_SET([warn_common], [])])])
AS_IF([${strong_optimization}],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [RUN], [ \
			-fweb \
			-ftree-vectorize \
			-fgraphite \
			-fgraphite-identity \
			-floop-interchange \
			-floop-strip-mine \
			-floop-block \
			-fno-enforce-eh-specs \
			${fwhole_program} \
			-DNDEBUG \
			-DNO_DEBUG \
			-DG_DISABLE_ASSERT \
		], [${my_cxxfatal}], [${CPPFLAGS}], [${quicktrue}])])
dnl We do not use (although we could, but it would change nothing):
dnl			-ffast-math \
AS_IF([${warnings}],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [COMPILE], [ \
			-fno-common \
			-fdirectives-only \
			-funsigned-char \
			-fPIC \
			-ftracer \
			-pedantic \
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
			-Wnoexcept \
			-Wmissing-declarations \
			-Wmissing-format-attribute \
			-Wunused-but-set-variable \
			-Wunused-but-set-parameter \
			-Wsuggest-attribute=pure \
			-Wsuggest-attribute=const \
			-Wsuggest-attribute=noreturn \
			-Wdelete-non-virtual-dtor \
			-Wzero-as-null-pointer-constant \
		], [], [], [${quicktrue}])
		dnl No [${my_cxxfatal}] for warning flags
dnl We do not use (since it gives too many useless warnings or is only for C):
dnl			-Wtraditional \
dnl			-Wc++-compat \
dnl			-Wstrict-prototypes \
dnl			-Wold-style-definition \
dnl			-Wmissing-prototypes \
dnl			-Wnested-externs \
dnl			-Wpointer-sign \
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [RUN], [ \
			-Wl,-z,defs \
			-Wl,--no-undefined \
			-Wl,--no-allow-shlib-undefined \
			-Wl,--no-undefined-version \
			${warn_common} \
		], [], [], [${quicktrue}])])
		dnl No [${my_ldfatal}] for warning flags
AS_IF([${strong_warnings}],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [COMPILE], [ \
			-Wsign-conversion \
			-Wold-style-cast \
			-Wmissing-noreturn \
		], [${my_cxxfatal}], [], [${quicktrue}])])
dnl We do not use (since it gives too many useless warnings or is only for C):
dnl			-Wconversion \
dnl			-Wswitch-enum \
dnl			-Wunsafe-loop-optimizations \
dnl			-Wpadded \
dnl			-Wunreachable-code \
dnl			-Waggregate-return \
dnl			-Winline \
dnl			-Weffc++ \
AS_IF([${security}],
	# We try to add -fstack-protector to LDFLAGS before adding to CXXFLAGS
	# since on some systems it requires also linking with a library.
	[MV_ADDFLAGS([my_ldadd], [LDFLAGS], [RUN], [ \
			-fstack-protector \
			-Wl,-z,now \
			-Wl,-z,relro \
			-Wl,-z,noexecstack \
		], [${my_ldfatal}], [], [${quicktrue}])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [RUN], [ \
			-D_FORTIFY_SOURCE=2 \
			-fstack-protector \
		], [${my_cxxfatal}], [${CPPFLAGS}], [${quicktrue}])])

# We are done with testing, the real flag mangling takes place:
MV_PREPEND([CXXFLAGS], [${my_cxxadd}])
AS_CASE([" ${LDFLAGS} ${my_ldadd} "],
	[*" -flto "*], [MV_APPEND([my_ldadd], [${CXXFLAGS}])])
MV_PREPEND([LDFLAGS], [${my_ldadd}])

AS_VAR_COPY([my_cxxflags], [CXXFLAGS])
AS_VAR_COPY([my_ldflags], [LDFLAGS])

# For --enable-quickcheck, we do at least one (collected) safety check;
# otherwise, the error messages may be too confusing for the user:
AS_IF([${quickcheck}],
	[AC_MSG_CHECKING([whether non-checking of options was appropriate])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
		[MV_MSG_RESULT([yes])],
		[MV_MSG_RESULT([no])
		AS_ECHO(["export CXXFLAGS='${CXXFLAGS}'"])
		AS_ECHO(["export LDFLAGS='${LDFLAGS}'"])
		AC_MSG_ERROR([Compiling/linking a simple program failed. Try --disable-quickcheck])])])

# Provide external tools
AC_PROG_LN_S()
AC_PROG_MKDIR_P()
AC_PROG_SED()

# Add the libintl support if needed.
dnl LT_INIT()
AM_GNU_GETTEXT_VERSION([0.17])
AM_GNU_GETTEXT([external])

# This must be an early test, since we need the null device later in this file:
AC_MSG_CHECKING([for null-device])
AC_ARG_WITH([dev-null],
	[AS_HELP_STRING([--with-null-device=STR],
		[null device of the system, usually /dev/null])],
	[AS_VAR_COPY([DEV_NULL], [withval])],
	[AS_VAR_SET([DEV_NULL], [])])
AS_VAR_IF([DEV_NULL], [],
	[AS_VAR_SET([DEV_NULL], ["/dev/null"])])
MV_MSG_RESULT_VAR([DEV_NULL])
AC_DEFINE_UNQUOTED([DEV_NULL],
	["${DEV_NULL}"],
	[Null device of the system])
AC_SUBST([DEV_NULL])

# Separate binaries?
AC_ARG_ENABLE([separate_binaries],
	[AS_HELP_STRING([--enable-separate-binaries],
		[create a separate binary for each program])],
	[MV_ENABLE([separate_binaries])],
	[AS_VAR_SET([separate_binaries], [false])])
AM_CONDITIONAL([SEPARATE_BINARIES],
	[${separate_binaries}])

# Separate eix-update?
AC_ARG_ENABLE([separate_update],
	[AS_HELP_STRING([--enable-separate-update],
		[create a separate eix-update binary])],
	[MV_ENABLE([separate-update])],
	[AS_VAR_SET([separate_update], [false])])
AS_IF([${separate_binaries}],
	[AS_VAR_SET([separate_update], [:])])
AM_CONDITIONAL([SEPARATE_UPDATE],
	[${separate_update}])

# Separate tools?
AC_ARG_ENABLE([separate_tools],
	[AS_HELP_STRING([--enable-separate-tools],
		[create separate binaries for tools (versionsort)])],
	[MV_ENABLE([separate-tools])],
	[AS_VAR_SET([separate_tools], [false])])
AM_CONDITIONAL([SEPARATE_TOOLS],
	[${separate_tools}])

# Look for basic functions:
AC_SYS_LARGEFILE()
AC_FUNC_FSEEKO()
AC_CHECK_FUNCS([regcomp strchr strdup strerror strrchr fnmatch memset],
	[],
	[AC_MSG_ERROR([We really need this function ..])])

# We need at least one of the following, the former the better
AC_CHECK_FUNCS([strtoul strtol atoi],
	[break],
	[AC_MSG_ERROR([At least one of strtoul/strtoll/atoi is needed ..])])

# Optional header files:
AC_CHECK_HEADERS_ONCE([sys/param.h grp.h])

# We use these optionally:
# Concerning strndup, we have a home-brewed replacement,
# for the others some less convenient workarounds.
AC_CHECK_FUNCS_ONCE([strndup \
	sigaction \
	canonicalize_file_name
	realpath \
	vfork \
	setenv \
	setuid \
	setgid \
	seteuid \
	setegid \
	setgroups \
	initgroups \
	])

# If climits exists, we need not check for limits.h
AC_CHECK_HEADERS([climits limits.h],
	[break])

# If tr1/cstdint exists we need not check for stdint.h.
AC_CHECK_HEADERS([tr1/cstdint cstdint stdint.h],
	[break])

# Use extra-doc?
AC_ARG_WITH([extra-doc],
	[AS_HELP_STRING([--with-extra-doc],
		[install developer documentation. Might need rst2html.py from docutils])],
	[AS_CASE(["${withval}"],
		[no], [AS_VAR_SET([extra_doc], [false])],
		[AS_VAR_SET([extra_doc], [:])])],
	[AS_VAR_SET([extra_doc], [false])])
AM_CONDITIONAL([EXTRA_DOC], [${extra_doc}])

# Check if __builtin_expect works
AC_MSG_CHECKING([whether __builtin_expect can be used])
AC_DEFUN([BUILTIN_EXPECT_PROGRAM], [AC_LANG_PROGRAM([[]], [[
if(!((__builtin_expect(1, 0)) && (__builtin_expect(1, 1)) &&
	(!__builtin_expect(0, 0)) && (!__builtin_expect(0, 1))))
	return 1;
		]])])
AC_RUN_IFELSE([BUILTIN_EXPECT_PROGRAM],
	[MV_MSG_RESULT([yes])
	AS_VAR_SET([bi_expect_works], [:])],
	[MV_MSG_RESULT([no])
	AS_VAR_SET([bi_expect_works], [false])],
	[AC_LINK_IFELSE([BUILTIN_EXPECT_PROGRAM],
		[MV_MSG_RESULT([yes], [crosscompile guess])
		AS_VAR_SET([bi_expect_works], [:])],
		[MV_MSG_RESULT([no], [crosscompile guess])
		AS_VAR_SET([bi_expect_works], [false])])])
AS_IF([${bi_expect_works}],
	[AC_DEFINE([HAVE___BUILTIN_EXPECT],
		[1],
		[Define to 1 if __builtin_expect can be used])])

# The __attribute__ tests are special: They should even break on warnings:
AS_VAR_COPY([CXXFLAGS], [oricxxflags])
AS_VAR_COPY([LDFLAGS], [orildflags])
MV_APPEND([CXXFLAGS], ["${my_cxxfatal}"])
MV_APPEND([LDFLAGS], ["${my_ldfatal}"])

# We even switch on explicitly -Wattribute although this should be default:
MV_ADDFLAGS([CXXFLAGS], [CXXFLAGS], [COMPILE], [ \
		-Wattributes \
	], [], [], [${quicktrue}])

# Check if __attribute__ ((__unused__)) works
AC_MSG_CHECKING([whether __attribute__ ((__unused__)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
static int dummy(int i);
static int
dummy(int i __attribute__ ((__unused__)))
{ return 0; }
		]], [[
return dummy(1);
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attribute_unused], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attribute_unused], [false])])
AS_IF([${attribute_unused}],
	[AC_DEFINE([HAVE_ATTRIBUTE_UNUSED],
		[1],
		[Define to 1 if __attribute__ ((__unused__)) can be used])])

# Check if __attribute__ ((__noreturn__)) works
AC_MSG_CHECKING([whether __attribute__ ((__noreturn__)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstdlib>
static void dummy() __attribute__ ((__noreturn__));
static void
dummy()
{ exit(0); }
		]], [[
dummy()
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attribute_noreturn], ["__attribute__ ((__noreturn__))"])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attribute_noreturn], [])])
AC_DEFINE_UNQUOTED([ATTRIBUTE_NORETURN],
	[${attribute_noreturn}],
	[Define to __attribute__ ((__noreturn__)) if it can be used])

# Check if __attribute__ ((__signal__)) works
AC_MSG_CHECKING([whether __attribute__ ((__signal__)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <csignal>
void my_sig_handler(int sig) __attribute__ ((__signal__));
class my_int {
friend void my_sig_handler(int sig) __attribute__ ((__signal__));
private:	static int i;
		static void set(int s) { i=s; }
};
int my_int::i;
void my_sig_handler(int sig)
{ my_int::set(sig); }
		]], [[
	signal(SIGTERM, my_sig_handler);
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attribute_signal], ["__attribute__ ((__signal__))"])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attribute_signal], [])])
AC_DEFINE_UNQUOTED([ATTRIBUTE_SIGNAL],
	[${attribute_signal}],
	[Define to  __attribute__ ((__signal__)) if it can be used])

# Check if __attribute__ ((__const__)) works
AC_MSG_CHECKING([whether __attribute__ ((__const__)) can be used])
AC_RUN_IFELSE([AC_LANG_PROGRAM([[
class b {
public:
	static int c(int i) __attribute__ ((__const__)) { return i; }
};
int a(int i) __attribute__ ((__const__));
int a(int i) { b x; return x.c(i); }
		]], [[
return !((a(4) == 4) && (a(5) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attribute_const], ["__attribute__ ((__const__))"])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attribute_const], [])])
AC_DEFINE_UNQUOTED([ATTRIBUTE_CONST],
	[${attribute_const}],
	[Define to  __attribute__ ((__const__)) if it can be used])

# Check if __attribute__ ((__pure__)) works
AC_MSG_CHECKING([whether __attribute__ ((__pure__)) can be used])
AC_RUN_IFELSE([AC_LANG_PROGRAM([[
class b {
public:
	static int c(int *i) __attribute__ ((__pure__)) { return *i; }
};
int a(int i) __attribute__ ((__pure__));
int a(int i) { b x; return x.c(&i); }
		]], [[
return !((a(4) == 4) && (a(5) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attribute_pure], ["__attribute__ ((__pure__))"])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attribute_pure], [])])
AC_DEFINE_UNQUOTED([ATTRIBUTE_PURE],
	[${attribute_pure}],
	[Define to  __attribute__ ((__pure__)) if it can be used])

# I know no reliable way to check whether __attribute__ ((__const__))
# can be used for const virtuals. So better use __attribute__ ((__pure__)).
AC_DEFINE_UNQUOTED([ATTRIBUTE_CONST_VIRTUAL],
	[${attribute_pure}],
	[Define to  __attribute__ ((__pure__)) or ((__const__)) or empty])

# reset the CXXFLAGS, LDFLAGS for the normal tests
AS_VAR_COPY([CXXFLAGS], [my_cxxflags])
AS_VAR_COPY([LDFLAGS], [my_ldflags])


# What about sqlite?
AC_MSG_CHECKING([whether sqlite should be used])
AS_VAR_SET([support_sqlite], [false])
AS_VAR_SET([manual_sqlite], [false])
AS_VAR_SET([pkgcfg_check_sqlite], [false])
AC_ARG_WITH([sqlite],
	[AS_HELP_STRING([--with-sqlite],
		[Compile in support for cache method sqlite])],
	[AS_CASE(["${withval}"],
		[no], [MV_MSG_RESULT([no], [on request])],
		[yes], [MV_MSG_RESULT([yes], [on request])
			AS_VAR_SET([support_sqlite], [:])
			m4_ifdef([PKG_CHECK_MODULES],
				[AS_VAR_SET([pkgcfg_check_sqlite], [:])],
				[AS_VAR_SET([manual_sqlite], [:])])])],
	[m4_ifdef([PKG_CHECK_MODULES],
		[MV_MSG_RESULT([trying autodetect])
		AS_VAR_SET([pkgcfg_check_sqlite], [:])],
		[MV_MSG_RESULT([no], [autodetection needs pkg-config])])])
AS_IF([${pkgcfg_check_sqlite}],
	[PKG_CHECK_MODULES([SQLITE], [sqlite3],
		[AS_VAR_SET([support_sqlite], [:])],
		[AS_IF([${support_sqlite}],
			[MV_MSG_RESULT([yes], [although pkg-config failed])
			AS_VAR_SET([manual_sqlite], [:])],
			[MV_MSG_RESULT([no], [autodetected])])])])
AS_IF([${manual_sqlite}],
	[AS_VAR_SET([SQLITE_LIBS], ["-lsqlite3"])
	AS_VAR_SET([SQLITE_CFLAGS], [])])
AS_IF([${support_sqlite}],
	[AC_DEFINE([WITH_SQLITE],
		[1],
		[Define to 1 if cache method sqlite is wanted])],
	[AS_VAR_SET([SQLITE_LIBS], [])
	AS_VAR_SET([SQLITE_CFLAGS], [])])
AC_SUBST([SQLITE_LIBS])
AC_SUBST([SQLITE_CFLAGS])

AC_MSG_CHECKING([PORTDIR_CACHE_METHOD default])
AC_ARG_WITH([portdir-cache-method],
	[AS_HELP_STRING([--with-portdir-cache-method=STR],
		[default PORTDIR_CACHE_METHOD])],
	[AS_VAR_COPY([PORTDIR_CACHE_METHOD], [withval])
	AS_VAR_SET([description], ["on request"])],
	[AS_VAR_SET([PORTDIR_CACHE_METHOD], ["metadata-md5-or-flat"])
	AS_VAR_SET([description], ["default"])])
MV_MSG_RESULT_VAR([PORTDIR_CACHE_METHOD], [${description}])
AC_DEFINE_UNQUOTED([PORTDIR_CACHE_METHOD],
	["${PORTDIR_CACHE_METHOD}"],
	[Default PORTDIR_CACHE_METHOD])
AC_SUBST([PORTDIR_CACHE_METHOD])

AC_MSG_CHECKING([EPREFIX default])
AC_ARG_WITH([eprefix-default],
	[AS_HELP_STRING([--with-eprefix-default=STR],
		[default EPREFIX])],
	[AS_VAR_COPY([EPREFIX_DEFAULT], [withval])],
	[AS_VAR_SET([EPREFIX_DEFAULT],
		["`eval portageq envvar EPREFIX 2>${DEV_NULL}`"])])
MV_PREFIX(EPREFIX_DEFAULT)
MV_MSG_RESULT_VAR([EPREFIX_DEFAULT])
AC_DEFINE_UNQUOTED([EPREFIX_DEFAULT],
	["${EPREFIX_DEFAULT}"],
	[Default EPREFIX])
AC_SUBST([EPREFIX_DEFAULT])

AC_MSG_CHECKING([EIX_PREFIX default])
AC_ARG_WITH([eix-prefix-default],
	[AS_HELP_STRING([--with-eix-prefix-default=STR],
		[default EIX_PREFIX])],
	[AS_VAR_COPY([EIX_PREFIX_DEFAULT], [withval])],
	[AS_VAR_SET([EIX_PREFIX_DEFAULT], [])])
MV_PREFIX(EPREFIX_DEFAULT)
MV_MSG_RESULT_VAR([EIX_PREFIX_DEFAULT])
AC_DEFINE_UNQUOTED([EIX_PREFIX_DEFAULT],
	["${EIX_PREFIX_DEFAULT}"],
	[Default EIX_PREFIX])
AC_SUBST([EIX_PREFIX_DEFAULT])

AC_MSG_CHECKING([ROOT default])
AC_ARG_WITH([root-default],
	[AS_HELP_STRING([--with-root-default=STR],
		[default ROOT])],
	[AS_VAR_COPY([ROOT_DEFAULT], [withval])],
	[AS_VAR_SET([ROOT_DEFAULT], [])])
MV_PREFIX(ROOT_DEFAULT)
MV_MSG_RESULT_VAR([ROOT_DEFAULT])
AC_DEFINE_UNQUOTED([ROOT_DEFAULT],
	["${ROOT_DEFAULT}"],
	[Default ROOT])
AC_SUBST([ROOT_DEFAULT])

AC_MSG_CHECKING([EXEC_EBUILD_SH default])
AC_ARG_WITH([ebuild-sh-default],
	[AS_HELP_STRING([--with-ebuild-sh-default=STR],
		[default EXEC_EBUILD_SH])],
	[AS_VAR_COPY([EBUILD_SH_DEFAULT], [withval])
	AS_VAR_SET([description], ["on request"])],
	[AS_VAR_SET([EBUILD_SH_DEFAULT], ["/usr/lib/portage/bin/ebuild.sh"])
	AS_IF([test -d "${EPREFIX_DEFAULT}/usr/lib"],
		[],
		[AS_IF([test -d "${EPREFIX_DEFAULT}/usr/lib64"],
			[AS_VAR_SET([EBUILD_SH_DEFAULT],
				["/usr/lib64/portage/bin/ebuild.sh"])])])
	AS_IF([test -x "${EPREFIX_DEFAULT}${EBUILD_SH_DEFAULT}"],
		[AS_VAR_SET([description], ["autodetected"])],
		[AS_VAR_SET([description], ["guessed"])])])
MV_PREFIX(EBUILD_SH_DEFAULT)
MV_MSG_RESULT_VAR([EBUILD_SH_DEFAULT], [${description}])
AC_DEFINE_UNQUOTED([EBUILD_SH_DEFAULT],
	["${EBUILD_SH_DEFAULT}"],
	[Default EXEC_EBUILD_SH (without prefix)])
AC_SUBST([EBUILD_SH_DEFAULT])

AC_MSG_CHECKING([PORTAGE_ROOTPATH default])
AC_ARG_WITH([portage-rootpath],
	[AS_HELP_STRING([--with-portage-rootpath=STR],
		[default PORTAGE_ROOTPATH])],
	[AS_VAR_COPY([PORTAGE_ROOTPATH_DEFAULT], [withval])],
	[AS_VAR_SET([PORTAGE_ROOTPATH_DEFAULT], [])])
MV_MSG_RESULT_VAR([PORTAGE_ROOTPATH_DEFAULT])
AC_DEFINE_UNQUOTED([PORTAGE_ROOTPATH_DEFAULT],
	["${PORTAGE_ROOTPATH_DEFAULT}"],
	[Default PORTAGE_ROOTPATH])
AC_SUBST([PORTAGE_ROOTPATH_DEFAULT])

AC_MSG_CHECKING([PORTAGE_BIN_PATH default])
AC_ARG_WITH([portage-bin-path],
	[AS_HELP_STRING([--with-portage-bin-path=STR],
		[default PORTAGE_BIN_PATH])],
	[AS_VAR_COPY([PORTAGE_BIN_PATH_DEFAULT], [withval])],
	[AS_VAR_SET([PORTAGE_BIN_PATH_DEFAULT], [])])
MV_MSG_RESULT_VAR([PORTAGE_BIN_PATH_DEFAULT])
AC_DEFINE_UNQUOTED([PORTAGE_BIN_PATH_DEFAULT],
	["${PORTAGE_BIN_PATH_DEFAULT}"],
	[Default PORTAGE_BIN_PATH])
AC_SUBST([PORTAGE_BIN_PATH_DEFAULT])

AC_MSG_CHECKING([default remote path])
AC_ARG_WITH([remote-file],
	[AS_HELP_STRING([--with-remote-file=STR],
		[default remote file for eix-remote])],
	[AS_VAR_COPY([DEFAULT_REMOTE_PATH], [withval])],
	[AS_VAR_SET([DEFAULT_REMOTE_PATH],
		["http://dev.gentooexperimental.org/eix_cache/eix-caches.tbz2"])])
MV_MSG_RESULT_VAR([DEFAULT_REMOTE_PATH])
AC_SUBST([DEFAULT_REMOTE_PATH])

AC_MSG_CHECKING([ALWAYS_ACCEPT_KEYWORDS default])
AC_ARG_WITH([always-accept-keywords],
	[AS_HELP_STRING([--with-always-accept-keywords],
		[default always-accept-keywords])],
	[AS_CASE(["${withval}"],
		[no], [AS_VAR_SET([ALWAYS_ACCEPT_KEYWORDS_DEFAULT], ["false"])],
		[AS_VAR_SET([ALWAYS_ACCEPT_KEYWORDS_DEFAULT], ["true"])])],
	[AS_VAR_SET([ALWAYS_ACCEPT_KEYWORDS_DEFAULT], ["false"])])
MV_MSG_RESULT_VAR([ALWAYS_ACCEPT_KEYWORDS_DEFAULT])
AC_DEFINE_UNQUOTED([ALWAYS_ACCEPT_KEYWORDS_DEFAULT],
	["${ALWAYS_ACCEPT_KEYWORDS_DEFAULT}"],
	[Default ALWAYS_ACCEPT_KEYWORDS])
AC_SUBST([ALWAYS_ACCEPT_KEYWORDS_DEFAULT])

AC_MSG_CHECKING([DEP default])
AC_ARG_WITH([dep-default],
	[AS_HELP_STRING([--with-dep-default],
		[default to DEP=true])],
	[AS_CASE(["${withval}"],
		[no], [AS_VAR_SET([DEP_DEFAULT], ["false"])],
		[AS_VAR_SET([DEP_DEFAULT], ["true"])])],
	[AS_VAR_SET([DEP_DEFAULT], ["false"])])
MV_MSG_RESULT_VAR([DEP_DEFAULT])
AC_DEFINE_UNQUOTED([DEP_DEFAULT],
	["${DEP_DEFAULT}"],
	[Default DEP])
AC_SUBST([DEP_DEFAULT])

AC_MSG_CHECKING([zsh completion])
AS_VAR_SET([ZSH_COMPLETION], ["${datadir}/zsh/site-functions"])
AC_ARG_WITH([zsh-completion],
	[AS_HELP_STRING([--with-zsh-completion=STR],
		[install zsh-completion file in directory STR])],
	[AS_CASE([${withval}],
		['/'*], [AS_VAR_COPY([ZSH_COMPLETION], [withval])],
		[no], [AS_VAR_SET([ZSH_COMPLETION], [])])])
MV_MSG_RESULT_VAR([ZSH_COMPLETION])
AC_SUBST([ZSH_COMPLETION])
MV_IF_EMPTY([${ZSH_COMPLETION}],
	[AS_VAR_SET([install_zsh_completion], [false])],
	[AS_VAR_SET([install_zsh_completion], [:])])
AM_CONDITIONAL([INSTALL_ZSH_COMPLETION],
	[${install_zsh_completion}])

# Check endianess
AC_C_BIGENDIAN()

# Provide basic types:
AC_TYPE_UINT8_T()
AC_TYPE_UINT16_T()
AC_TYPE_UINT32_T()

EIX_CACHEFILE="/var/cache/eix/portage.eix"
AC_DEFINE_UNQUOTED([EIX_CACHEFILE],
	["${EIX_CACHEFILE}"],
	[Location of the cachefile])
AC_SUBST([EIX_CACHEFILE])

EIX_PREVIOUS="/var/cache/eix/previous.eix"
AC_DEFINE_UNQUOTED([EIX_PREVIOUS],
	["${EIX_PREVIOUS}"],
	[Location of the previous cachefile])
AC_SUBST([EIX_PREVIOUS])

EIX_REMOTEARCHIVE="/var/cache/eix/remote.tar.bz2"
AC_DEFINE_UNQUOTED([EIX_REMOTEARCHIVE],
	["${EIX_REMOTEARCHIVE}"],
	[Location of the local remote cache archive])
AC_SUBST([EIX_REMOTEARCHIVE])

AC_DEFINE_UNQUOTED([LEVENSHTEIN_DISTANCE_DEFAULT],
	["2"],
	[Default Levenshtein distance for matches (as string)])
AC_SUBST([LEVENSHTEIN_DISTANCE])

# Done!
AC_OUTPUT()

AS_ECHO(["

${PACKAGE}-${VERSION} configured successfully.

"])
AS_ECHO(["CXXFLAGS: ${oricxxflags}"])
MV_IF_NONEMPTY([${my_cxxadd}],
	[AS_ECHO(["PREPEND_CXXFLAGS: ${my_cxxadd}
"])])
AS_ECHO(["LDFLAGS: ${orildflags}"])
MV_IF_NONEMPTY([${my_ldadd}],
	[AS_ECHO(["PREPEND_LDFLAGS: ${my_ldadd}"])])
AS_ECHO([])
