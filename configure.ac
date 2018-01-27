# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#  Wolfgang Frisch <xororand@users.sourceforge.net>
#  Emil Beinroth <emilbeinroth@gmx.net>
#  Martin V\"ath <martin@mvath.de>

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
dnl (Local)/global sh variables are (non-)/capital.
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
dnl (like calls of most MV_* macros).
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
AC_INIT([eix], [0.33.2], [https://github.com/vaeth/eix/issues/], [eix], [https://github.com/vaeth/eix/])
AC_PREREQ([2.64])

m4_ifdef([AC_CONFIG_MACRO_DIR],
	[AC_CONFIG_MACRO_DIR([m4])])
m4_ifdef([AC_CONFIG_MACRO_DIRS],
	[AC_CONFIG_MACRO_DIRS([m4 martinm4])])
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
# disable --disable-silent-rules to omit useless output with Gentoo's EAPI=5:
# AS_VAR_SET([enable_silent_rules], [yes])
AM_SILENT_RULES([yes])

AC_LANG([C++])
AC_PROG_CXX()

# Now our flag mangling options:

AC_ARG_ENABLE([debugging],
	[AS_HELP_STRING([--enable-debugging],
		[activate paranoic asserts and flags for debugging])],
	[MV_ENABLE([debugging])
	AS_VAR_SET([cmt_debugging], ["on request"])],
	[AS_VAR_SET([debugging], [false])
	AS_VAR_SET([cmt_debugging], ["default"])])

AC_ARG_ENABLE([paranoic-asserts],
	[AS_HELP_STRING([--enable-paranoic-asserts],
		[activate paranoic asserts])],
	[MV_ENABLE([paranoic_asserts])],
	[AS_VAR_SET([paranoic_asserts], [false])])
AS_IF([$debugging],
	[AS_VAR_SET([paranoic_asserts], [true])])

AC_ARG_ENABLE([debug-format],
	[AS_HELP_STRING([--enable-debug-format],
		[activate string format runtime tests])],
	[MV_ENABLE([debug_format])
	AS_VAR_SET([cmt_debug_format], ["on request"])],
	[AS_IF([$debugging],
		[AS_VAR_SET([debug_format], [true])
		AS_VAR_SET([cmt_debug_format], ["default due to debugging"])],
		[AS_VAR_SET([debug_format], [true])
		AS_VAR_SET([cmt_debug_format], ["default"])])])

AC_ARG_ENABLE([new-dialect],
	[AS_HELP_STRING([--enable-new-dialect],
		[enable most current C++ dialect; provides optimizations])],
	[MV_ENABLE([new_dialect])
	AS_VAR_SET([cmt_new_dialect], ["on request"])],
	[AS_VAR_SET([new_dialect], [:])
	AS_VAR_SET([cmt_new_dialect], ["default"])])

AC_ARG_ENABLE([dead-code],
	[AS_HELP_STRING([--disable-dead-code-eliminitation],
		[disable dead code elimination options; disable this only if compilation fails])],
	[MV_ENABLE([dead_code])
	AS_VAR_SET([cmt_dead_code], ["on request"])],
	[AS_VAR_SET([dead_code], [:])
	AS_VAR_SET([cmt_dead_code], ["default"])])

AC_ARG_ENABLE([security],
	[AS_HELP_STRING([--enable-security],
		[append security relevant options to flags])],
	[MV_ENABLE([security])
	AS_VAR_SET([cmt_security], ["on request"])],
	[AS_VAR_SET([security], [false])
	AS_VAR_SET([cmt_security], ["default"])])

AC_ARG_ENABLE([strong-security],
	[AS_HELP_STRING([--enable-strong-security],
		[append paranoic security options to flags; slowdown])],
	[MV_ENABLE([strong-security])
	AS_VAR_SET([cmt_strong_security], ["on request"])],
	[AS_VAR_SET([strong_security], [false])
	AS_VAR_SET([cmt_strong_security], ["default"])])

AC_ARG_ENABLE([nopie-security],
	[AS_HELP_STRING([--enable-nopie-security],
		[as --enable-strong-security but avoid pie to make sanitizer work])],
	[MV_ENABLE([nopie-security])
	AS_VAR_SET([cmt_nopie_security], ["on request"])],
	[AS_VAR_SET([nopie_security], [false])
	AS_VAR_SET([cmt_nopie_security], ["default"])])

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

# Some implicit dependencies of these options.
# We do this here explicitly and output the results

AC_MSG_CHECKING([whether debugging asserts/flags should be used])
MV_MSG_RESULT_BIN([$debugging], [$cmt_debugging])

AC_MSG_CHECKING([whether strong optimization options should be used])
AS_IF([$paranoic_asserts],
	[AS_VAR_SET([strong_optimization], [false])
	AS_VAR_SET([cmt_strong_optimization], ["disabled by paranoic asserts"])])
MV_MSG_RESULT_BIN([$strong_optimization], [$cmt_strong_optimization])

AC_MSG_CHECKING([whether optimization options should be used])
AS_IF([$strong_optimization],
	[AS_VAR_SET([optimization], [:])
	AS_VAR_SET([cmt_optimization], ["forced by strong optimization"])])
AS_IF([$debugging],
	[AS_VAR_SET([optimization], [false])
	AS_VAR_SET([cmt_optimization], ["disabled by debugging"])])
MV_MSG_RESULT_BIN([$optimization], [$cmt_optimization])

AC_MSG_CHECKING([whether dead code elimination options should be used])
AS_IF([$debugging],
	[AS_VAR_SET([dead_code], [false])
	AS_VAR_SET([cmt_dead_code], ["disabled by debugging"])])
MV_MSG_RESULT_BIN([$dead_code], [$cmt_dead_code])

AC_MSG_CHECKING([whether most current C++ dialect options should be used])
AS_IF([$optimization],
	[AS_VAR_SET([new_dialect], [:])
	AS_VAR_SET([cmt_new_dialect], ["forced by optimization"])])
MV_MSG_RESULT_BIN([$new_dialect], [$cmt_new_dialect])

AC_MSG_CHECKING([whether strong warning options should be used])
MV_MSG_RESULT_BIN([$strong_warnings], [$cmt_strong_warnings])

AC_MSG_CHECKING([whether warning options should be used])
AS_IF([$strong_warnings],
	[AS_VAR_SET([warnings], [:])
	AS_VAR_SET([cmt_warnings], ["forced by strong warnings"])])
MV_MSG_RESULT_BIN([$warnings], [$cmt_warnings])

AC_MSG_CHECKING([whether nopie security options should be used])
MV_MSG_RESULT_BIN([$nopie_security], [$cmt_nopie_security])

AC_MSG_CHECKING([whether strong security options should be used])
AS_IF([$nopie_security],
	[AS_VAR_SET([strong_security], [:])
	AS_VAR_SET([cmt_strong_security], ["forced by nopie security"])])
MV_MSG_RESULT_BIN([$strong_security], [$cmt_strong_security])

AC_MSG_CHECKING([whether security options should be used])
AS_IF([$strong_security],
	[AS_VAR_SET([security], [:])
	AS_VAR_SET([cmt_security], ["forced by strong security"])])
MV_MSG_RESULT_BIN([$security], [$cmt_security])

# Now start the flag mangling:
AS_IF([$debugging],
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
AS_VAR_SET([my_cxxfatal], [])
AS_VAR_SET([my_ldfatal], [])
MV_ADDFLAGS([my_cxxfatal], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
		-Werror \
		-Werror=unknown-warning-option \
		-Wunknown-warning-option \
	])
MV_ADDFLAGS([my_ldfatal], [LDFLAGS], [AC_LINK_IFELSE], [ \
		$my_cxxfatal \
		-Wl,--fatal-warnings \
	])

# Now the actual testing:
AS_VAR_SET([my_cxxadd], [])
AS_VAR_SET([my_ldadd], [])

# Prefer the newest available most fancy C++ dialect to detect problems early
AS_IF([$new_dialect],
	[AS_CASE([" $CXXFLAGS $my_cxxadd"],
		[*" -std="*], [],
		[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
				-std=gnu++17 \
				-std=c++17 \
				-std=gnu++1z \
				-std=c++1z \
				-std=gnu++14 \
				-std=c++14 \
				-std=gnu++1y \
				-std=c++1y \
				-std=gnu++11 \
				-std=c++11 \
			], [$my_cxxfatal], [], [break])])])
AS_VAR_COPY([my_cxxdialect], [my_cxxadd])

m4_if([dnl The following turned out not to be useful currently, so it is
dnl commented out. Note that this needs a second part at the end of this file
dnl (also commented out) which modifies the CXXFLAGS correspodingly.
# Use -include config.h if possible to get more hits with ccache
AS_VAR_SET([add_incopt], ["-include config.h"])
AC_MSG_CHECKING([whether CXXFLAGS=$add_incopt can be used])
AS_VAR_COPY([my_cxxflags], [CXXFLAGS])
MV_APPEND([CXXFLAGS], [$my_cxxfatal])
MV_APPEND([CXXFLAGS], [$add_incopt.in])
export CXXFLAGS
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	AS_VAR_SET([add_incopt], [])])
AS_VAR_COPY([CXXFLAGS], [my_cxxflags])])

AS_IF([$dead_code],
	[AS_VAR_SET([dead_code], [false])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-Wl,--gc-sections \
		], [$my_ldfatal], [], [AS_VAR_SET([dead_code], [:])])])
AS_IF([$dead_code],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
		-fdata-sections \
		-ffunction-sections \
		], [$my_cxxfatal], [], [:])])

AS_IF([$debugging],
	[MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-g \
			-ggdb3 \
		], [$my_ldfatal], [], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-g \
			-ggdb3 \
		], [$my_cxxfatal], [], [:])])
AS_IF([$optimization],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-O2 \
			-fomit-frame-pointer \
			-fno-common \
			-fstrict-aliasing \
			-fstrict-enums \
			-fmerge-all-constants \
			-finline-functions \
			-fvisibility=hidden \
			-fvisibility-inlines-hidden \
			-ftree-pre \
			-ftree-partial-pre \
			-fnothrow-opt \
			-fno-unwind-tables \
			-fno-asynchronous-unwind-tables \
			-ffast-math \
			-fno-math-errno \
			-frename-registers \
			-funsafe-loop-optimizations \
			-fgcse-sm \
			-fgcse-las \
			-fgcse-after-reload \
			-fpredictive-commoning \
			-ftree-switch-conversion \
			-fno-ident \
			-freorder-functions \
			-fdevirtualize-speculatively \
			-fdevirtualize-at-ltrans \
			-fno-semantic-interposition \
			-fira-loop-pressure \
			-ftree-loop-distribution \
			-ftree-loop-distribute-patterns \
			-ftree-loop-im \
			-fivopts \
			-fdelete-null-pointer-checks \
		], [$my_cxxfatal], [], [:])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-O1 \
			-Wl,--relax \
			-Wl,--as-needed \
			-Wl,--sort-common \
			-Wl,-z,combreloc \
			-Wl,--build-id=none \
		], [$my_ldfatal], [], [:])])
AS_IF([$strong_optimization],
	[AS_VAR_SET([flto], ["
		-flto \
		-flto-partition=none \
		-flto-odr-type-merging \
		"])
#		-use-linker-plugin \
#		-emit-llvm \
# We first modify the LDFLAGS: This is important because of -flto:
# Without -flto in LDFLAGS, -flto in CXXFLAGS might not fail even if it
# should produce broken code.
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-O9 \
			$flto \
			-fwhole-program \
			-Wl,-z,noexecstack \
		], [$my_ldfatal], [], [:])
dnl The following is never tried, since the run can succeed but the result
dnl may fail for some unknown reason on non-GNU systems, see
dnl https://bugs.gentoo.org/show_bug.cgi?id=293263
dnl			-Wl,--hash-style=gnu \
dnl			-Wl,--enable-new-dtags \
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			$flto \
		], [$my_cxxfatal], [], [:])])
# If -flto is used during compilation, we can also use -fwhole-program and
# -fno-fat-lto-objects during compilation and should specify -flto during
# linking.
# However, -Wl,--warn-common will then give lots of warnings, so we
# do not add the latter unless we use strong_warnings.
AS_VAR_SET([fwhole_program], [])
AS_VAR_SET([warn_common], [-Wl,--warn-common])
AS_CASE([" $CXXFLAGS $my_cxxadd "],
	[*" -flto "*],
		[AS_VAR_SET([fwhole_program], ["
			-fwhole-program
			-fno-fat-lto-objects
			"])
		AS_IF([$strong_warnings],
			[],
			[AS_VAR_SET([warn_common], [])])])
AS_IF([$strong_optimization],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-fweb \
			-fno-enforce-eh-specs \
			$fwhole_program \
			-ftree-vectorize \
			-fvect-cost-model \
			-fno-rtti \
			-fno-threadsafe-statics \
		], [$my_cxxfatal], [$CPPFLAGS], [:])
dnl We do not use graphite flags since they breaks in too many compilers:
dnl			-fgraphite \
dnl			-fgraphite-identity \
dnl			-floop-interchange \
dnl			-floop-strip-mine \
dnl			-floop-block \
	AS_IF([$strong_security],
		[],
		[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-DNDEBUG \
			-DNO_DEBUG \
			-DG_DISABLE_ASSERT \
		], [$my_cxxfatal], [$CPPFLAGS], [:])])])
dnl We do not use:
dnl Too much memory/time for compilation:
dnl			-fipa-pta
dnl Default (e.g. with -O2):
dnl			-fipa-icf
dnl			-fipa-ra
AS_IF([$warnings],
dnl -Wzero-as-null-pointer-constant makes sense only with new dialects
	[AS_VAR_SET([zero_as_null_pointer], [-Wzero-as-null-pointer-constant])
	AS_CASE([" $CXXFLAGS "],
		[*" -std=c++"??" "*], [],
		[*" -std=gnu++"??" "*], [],
		[AS_VAR_SET([zero_as_null_pointer], [])])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
			-fno-common \
			-funsigned-char \
			-fdirectives-only \
			-ftracer \
			-fconcepts \
		], [$my_cxxfatal], [$CPPFLAGS], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
			-Wpedantic \
			-pedantic \
			-Wall \
			-Wextra \
			-Wformat=2 \
			-Wunused \
			-Wunused-variable \
			-Wunused-parameter \
			-Winit-self \
			-Wmissing-include-dirs \
			-Wswitch-default \
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
			-Wvolatile-register-var \
			-Wdisabled-optimization \
			-Wlogical-op \
			-Wvla \
			-Wabi \
			-Wctor-dtor-privacy \
			-Wstrict-null-sentinel \
			-Wstring-plus-int \
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
			-Wsuggest-final-type \
			-Wsuggest-final-methods \
			-Wsuggest-override \
			-Wdelete-non-virtual-dtor \
			$zero_as_null_pointer \
			-Wint-to-pointer-cast \
			-Wcomment \
			-WNSObject-attribute \
			-fdiagnostic-color=always \
			-Wswitch-bool \
			-Wlogical-not-parantheses \
			-Wsizeof-array-argument \
			-Walloc-zero \
			-Wbool-compare \
			-Wbool-operation \
			-Wodr-type-mismatch \
			-Wshift-negative-value \
			-Wshift-overflow \
			-Wshift-overflow=2 \
			-Wnull-dereference \
			-Wduplicated-branches \
			-Wduplicated-cond \
			-Wno-c++1z-extensions \
		], [$my_cxxfatal], [], [:])
dnl We do not use:
dnl Not needed anymore with current clang:
dnl			-Wno-unknown-warning-option \
dnl Output is too confusing:
dnl			-fmem-report \
dnl Too many useless warnings or only for C:
dnl			-Weverything \
dnl			-Wtraditional \
dnl			-Wc++-compat \
dnl			-Wstrict-prototypes \
dnl			-Wold-style-definition \
dnl			-Wmissing-prototypes \
dnl			-Wnested-externs \
dnl			-Wpointer-sign \
dnl Included in -Wall (in gcc-6; not available earlier, anyway)
dnl			-Wtautological-compare \
dnl			-Wmisleading-indentation \
dnl
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_FALSE], [ \
			-Wl,-z,defs \
			-Wl,--no-undefined \
			-Wl,--no-allow-shlib-undefined \
			-Wl,--no-undefined-version \
			$warn_common \
		], [$my_cxxfatal], [], [:])])
AS_IF([$strong_warnings],
	[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
			-Wsign-conversion \
			-Wold-style-cast \
			-Wmissing-noreturn \
		], [], [], [:])])
dnl We do not use (since it gives too many useless warnings or is only for C):
dnl			-Wconversion \
dnl			-Wswitch-enum \
dnl			-Wunsafe-loop-optimizations \
dnl			-Wpadded \
dnl			-Wunreachable-code \
dnl			-Waggregate-return \
dnl			-Winline \
dnl			-Weffc++ \
AS_IF([$strong_security],
	[AS_VAR_SET([my_stackprotector], ["
		-fstack-protector-all
		-fstack-protector-strong
		-fstack-protector
		-fstack-check
		-fstack-check=specific
	"])],
	[AS_VAR_SET([my_stackprotector], ["
		-fstack-protector-strong
		-fstack-protector
		-fstack-check
		-fstack-check=specific
	"])])
AS_IF([$security],
	# We try to add -fstack-protector to LDFLAGS before adding to CXXFLAGS
	# since on some systems it requires also linking with a library.
	[AS_IF([$nopie_security], [],
		[MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
				-fPIE \
			], [$my_cxxfatal], [$CPPFLAGS], [:])
		MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
				-fPIE \
			], [$my_ldfatal], [], [:])
		MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
				-pie \
			], [$my_cxxfatal], [$CPPFLAGS], [:])
		MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
				-pie \
				], [$my_ldfatal], [], [:])])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-Wl,-z,retpolineplt \
			-Wl,-z,now \
			-Wl,-z,relro \
			-Wl,-z,noexecstack \
		], [$my_ldfatal], [], [:])
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
			$my_stackprotector \
			], [$my_ldfatal], [], [break])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			-mretpoline \
			-mindirect-branch=thunk \
			-mfunction-return=thunk \
			-D_FORTIFY_SOURCE=2 \
		], [$my_cxxfatal], [$CPPFLAGS], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			$my_stackprotector \
			], [$my_cxxfatal], [$CPPFLAGS], [break])])
AS_IF([$strong_security],
	# We try to add -fsanitize=... to LDFLAGS before adding to CXXFLAGS
	# since on some systems it requires also linking with a library.
	[AS_VAR_SET([my_sanitize], ["
			-faddress-sanitizer
			-fsanitize=address
			-fsanitize=leak
			-fsanitize=undefined
			-fsanitize=vla-bound
			-fsanitize=shift
			-fsanitize=signed-integer-overflow
			-fsanitize=integer-divide-by-zero
			-fsanitize=unreachable
			-fsanitize=null
			-fsanitize=return
			-fsanitize=float-divide-by-zero
			-fsanitize=float-cast-overflow
			-fsanitize=bounds
			-fsanitize=bounds-strict
			-fsanitize=alignment
			-fsanitize=object-size
			-mmpx
			-fcheck-pointer-bounds
			-Wchkp
		"])
	# The following -fsanitize do not work properly
	# -fsanitize=threads
	# -fsanitize=vptr
	MV_ADDFLAGS([my_ldadd], [LDFLAGS], [MV_RUN_IFELSE_LINK], [ \
			$my_sanitize \
		], [$my_ldfatal], [], [:])
	MV_ADDFLAGS([my_cxxadd], [CXXFLAGS], [MV_RUN_IFELSE_LINK], [ \
			$my_sanitize \
		], [$my_cxxfatal], [$CPPFLAGS], [:])])

# We are done with testing, the real flag mangling takes place:
AS_VAR_COPY([CXXFLAGS], [oricxxflags])
MV_PREPEND([CXXFLAGS], [$my_cxxadd])
AS_VAR_COPY([LDFLAGS], [orildflags])
AS_CASE([" $LDFLAGS $my_ldadd "],
	[*" -flto "*], [MV_APPEND([my_ldadd], [$CXXFLAGS])])
MV_PREPEND([LDFLAGS], [$my_ldadd])

AS_VAR_COPY([my_cxxflags], [CXXFLAGS])
AS_VAR_COPY([my_ldflags], [LDFLAGS])

# Provide external tools
AC_PROG_LN_S()
AC_PROG_MKDIR_P()
AC_PROG_SED()

# Add the libintl support if needed.
dnl LT_INIT()
AM_GNU_GETTEXT_REQUIRE_VERSION([0.19.6])
AM_GNU_GETTEXT([external])
AS_CASE([$USE_NLS],
	[yes],
		[AS_VAR_SET([using_nls], [:])
		AS_VAR_SET([INST_MAN_LINGUAS], [${LINGUAS-*}])],
	[AS_VAR_SET([using_nls], [false])
	AS_VAR_SET([INST_MAN_LINGUAS], [])])
AM_CONDITIONAL([USING_NLS],
	[$using_nls])
AC_SUBST([INST_MAN_LINGUAS])

# This must be an early test, since we need the null device later in this file:
AC_MSG_CHECKING([for null-device])
AC_ARG_WITH([dev-null],
	[AS_HELP_STRING([--with-dev-null=STR],
		[null device of the system, usually /dev/null])],
	[AS_VAR_COPY([DEV_NULL], [withval])],
	[AS_VAR_SET([DEV_NULL], [])])
AS_VAR_IF([DEV_NULL], [],
	[AS_VAR_SET([DEV_NULL], [/dev/null])])
MV_MSG_RESULT_VAR([DEV_NULL])
AC_DEFINE_UNQUOTED([DEV_NULL],
	["$DEV_NULL"],
	[Null device of the system])
AC_SUBST([DEV_NULL])

# Separate binaries?
AC_ARG_ENABLE([separate_binaries],
	[AS_HELP_STRING([--enable-separate-binaries],
		[create a separate binary for each of eix, eix-update, eix-diff])],
	[MV_ENABLE([separate_binaries])],
	[AS_VAR_SET([separate_binaries], [false])])
AM_CONDITIONAL([SEPARATE_BINARIES],
	[$separate_binaries])

# Separate eix-update?
AC_ARG_ENABLE([separate_update],
	[AS_HELP_STRING([--enable-separate-update],
		[create a separate eix-update binary])],
	[MV_ENABLE([separate-update])],
	[AS_VAR_SET([separate_update], [false])])
AS_IF([$separate_binaries],
	[AS_VAR_SET([separate_update], [:])])
AM_CONDITIONAL([SEPARATE_UPDATE],
	[$separate_update])

# Separate tools?
AC_ARG_ENABLE([separate_tools],
	[AS_HELP_STRING([--enable-separate-tools],
		[create separate binaries for versionsort, eix-drop-permissions])],
	[MV_ENABLE([separate-tools])],
	[AS_VAR_SET([separate_tools], [false])])
AM_CONDITIONAL([SEPARATE_TOOLS],
	[$separate_tools])

# Look for basic functions:
AC_SYS_LARGEFILE()
AC_FUNC_FSEEKO()
AC_CHECK_FUNCS([regcomp strchr strerror strrchr fnmatch memset],
	[],
	[AC_MSG_ERROR([We really need this function ..])])

# We need at least one of the following, the former the better
AC_CHECK_FUNCS([strtoull strtoul],
	[break],
	[:])
AC_CHECK_FUNCS([strtoll strtol atoi],
	[break],
	[AC_MSG_ERROR([At least one of stroll/strtol/atoi is needed ..])])

# Optional header files:
AC_CHECK_HEADERS([sys/param.h \
	termios.h \
	stropts.h \
	sys/file.h \
	sys/ioctl.h \
	sys/stream.h \
	sys/ptem.h \
	sys/tty.h \
	sys/pty.h \
	grp.h \
	interix/security.h \
	])

# We use these optionally:
# Workarounds are used if they are not available
AC_CHECK_FUNCS([ \
	fileno \
	flock \
	sigaction \
	canonicalize_file_name \
	realpath \
	vfork \
	setenv \
	setuid \
	setgid \
	seteuid \
	setegid \
	setuser \
	setgroups \
	initgroups \
	])

AC_DEFUN([SETGETXPROGRAM], [AC_LANG_PROGRAM([[
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_INTERIX_SECURITY_H
#include <interix/security.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif
$1
]], [[
$2
]])])

AC_DEFUN([PROTO_SETGETX],
	[AC_MSG_CHECKING([whether $1 prototype works])
	AC_LINK_IFELSE([SETGETXPROGRAM([], [$2;])],
		[MV_MSG_RESULT([yes])],
		[AC_LINK_IFELSE([SETGETXPROGRAM([$3], [$2;])],
			[MV_MSG_RESULT([no, but can be provided])
			$4],
			[MV_MSG_RESULT([no and cannot be provided])
			$5])])])

AC_DEFUN([PROTO_SETX],
	[PROTO_SETGETX([$1], [$1(0)], [$2], [$3], [$4])])

AC_DEFUN([PROTO_GETX],
	[PROTO_SETGETX([$1], [return($1() == 0)], [$2], [$3], [$4])])

PROTO_SETX([setuid], [
int setuid(uid_t uid);
	],
	[AC_DEFINE([NEED_SETUID_PROTO],
		[1],
		[Define if setuid needs a prototype])],
	[AC_DEFINE([BROKEN_SETUID],
		[1],
		[Define if setuid is broken])])
PROTO_SETX([seteuid], [
int seteuid(uid_t uid);
	],
	[AC_DEFINE([NEED_SETEUID_PROTO],
		[1],
		[Define if seteuid needs a prototype])],
	[AC_DEFINE([BROKEN_SETEUID],
		[1],
		[Define if seteuid is broken])])
PROTO_SETX([setgid], [
int setgid(gid_t gid);
	],
	[AC_DEFINE([NEED_SETGID_PROTO],
		[1],
		[Define if setgid needs a prototype])],
	[AC_DEFINE([BROKEN_SETGID],
		[1],
		[Define if setgid is broken])])
PROTO_SETX([setegid], [
int setegid(gid_t gid);
	],
	[AC_DEFINE([NEED_SETEGID_PROTO],
		[1],
		[Define if setegid needs a prototype])],
	[AC_DEFINE([BROKEN_SETEGID],
		[1],
		[Define if setegid is broken])])

PROTO_GETX([getuid], [
uid_t getuid();
	],
	[AC_DEFINE([NEED_GETUID_PROTO],
		[1],
		[Define if getuid needs a prototype])],
	[AC_DEFINE([BROKEN_GETUID],
		[1],
		[Define if getuid is broken])])
PROTO_GETX([geteuid], [
uid_t geteuid();
	],
	[AC_DEFINE([NEED_GETEUID_PROTO],
		[1],
		[Define if geteuid needs a prototype])],
	[AC_DEFINE([BROKEN_GETEUID],
		[1],
		[Define if geteuid is broken])])
PROTO_GETX([getgid], [
gid_t getgid();
	],
	[AC_DEFINE([NEED_GETGID_PROTO],
		[1],
		[Define if getgid needs a prototype])],
	[AC_DEFINE([BROKEN_GETGID],
		[1],
		[Define if getgid is broken])])
PROTO_GETX([getegid], [
git_t getegid();
	],
	[AC_DEFINE([NEED_GETEGID_PROTO],
		[1],
		[Define if getegid needs a prototype])],
	[AC_DEFINE([BROKEN_GETEGID],
		[1],
		[Define if getegid is broken])])

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
	[AS_CASE(["$withval"],
		[no], [AS_VAR_SET([extra_doc], [false])],
		[AS_VAR_SET([extra_doc], [:])])],
	[AS_VAR_SET([extra_doc], [false])])
AM_CONDITIONAL([EXTRA_DOC], [$extra_doc])

# Use paranoic asserts?
AC_MSG_CHECKING([whether paranoic asserts are used])
AS_IF([$paranoicassert],
	[MV_MSG_RESULT([yes])
	AC_DEFINE([EIX_PARANOIC_ASSERT],
		[1],
		[Define if paranoic asserts should be used])
	AC_DEFINE([EIX_STATIC_ASSERT],
		[1],
		[Define if asserts for static initializers should be used])],
	[MV_MSG_RESULT([no])])

# Use string format runtime tests?
AC_MSG_CHECKING([whether string format runtime tests are used])
AS_IF([$debug_format],
	[MV_MSG_RESULT([yes], [$cmt_debug_format])
	AC_DEFINE([EIX_DEBUG_FORMAT],
		[1],
		[Define if string format runtime tests should be used])],
	[MV_MSG_RESULT([no], [$cmt_debug_format])])

# Check if __builtin_expect works
AC_MSG_CHECKING([whether __builtin_expect can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[]], [[
int zero = 0;
int one = 1;
if(!((__builtin_expect(one, 0)) && (__builtin_expect(one, 1)) &&
	(!__builtin_expect(zero, 0)) && (!__builtin_expect(zero, 1))))
	return 1;
		]])],
	[MV_MSG_RESULT([yes])
	AS_VAR_SET([bi_expect_works], [:])],
	[MV_MSG_RESULT([no])
	AS_VAR_SET([bi_expect_works], [false])])
AS_IF([$bi_expect_works],
	[AC_DEFINE([HAVE___BUILTIN_EXPECT],
		[1],
		[Define if __builtin_expect can be used])])

# The __attribute__ tests are special: They should even break on warnings:
AS_VAR_COPY([CXXFLAGS], [oricxxflags])
AS_VAR_COPY([LDFLAGS], [orildflags])
MV_APPEND([CXXFLAGS], [$my_cxxdialect])
MV_APPEND([CXXFLAGS], [$my_cxxfatal])
MV_APPEND([LDFLAGS], [$my_ldfatal])

# We even switch on explicitly -Wattribute although this should be default:
MV_ADDFLAGS([CXXFLAGS], [CXXFLAGS], [AC_COMPILE_IFELSE], [ \
		-Wattributes \
	])

# Check if _Pragma("GCC diagnostic ...") can be used inside functions
AC_MSG_CHECKING([whether pragma GCC diagnostic can be used everywhere])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
		]], [[
signed char j(0);
unsigned char i;
GCC_DIAG_OFF(sign-conversion)
i = j;
GCC_DIAG_ON(sign-conversion)
GCC_DIAG_OFF(old-style-cast)
return ((int)i);
GCC_DIAG_ON(old-style-cast)
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([gcc_diag_pragma], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([gcc_diag_pragma], [false])])
AS_IF([$gcc_diag_pragma],
	[AC_DEFINE([FULL_GCC_DIAG_PRAGMA],
		[1],
		[Define if _Pragma GCC diagnostic can be used everywhere])])

# Check if [[noreturn]] works
AC_MSG_CHECKING([[whether [[noreturn]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstdlib>
[[noreturn]] static void dummy();
static void dummy() { exit(0); }
		]], [[
dummy()
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([c11attr_noreturn], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([c11attr_noreturn], [false])])
AS_IF([$c11attr_noreturn],
	[AC_DEFINE([HAVE_C11ATTRIBUTE_NORETURN],
		[1],
		[Define if [[noreturn]] can be used])])

# Check if __attribute__ ((noreturn)) works
AC_MSG_CHECKING([whether __attribute__ ((noreturn)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstdlib>
__attribute__ ((noreturn)) static void dummy();
static void dummy() { exit(0); }
		]], [[
dummy()
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_noreturn], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_noreturn], [false])])
AS_IF([$attr_noreturn],
	[AC_DEFINE([HAVE_ATTRIBUTE_NORETURN],
		[1],
		[Define if __attribute__ ((noreturn)) can be used])])

# Check if [[fallthrough]] works
AC_MSG_CHECKING([[whether [[fallthrough]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		]], [[
int i = 0;
switch(i) {
	case 0:
		--i;
		[[fallthrough]];
	default:
		return i + 1;
}
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_fallthrough], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_fallthrough], [false])])
AS_IF([$attr_fallthrough],
	[AC_DEFINE([HAVE_ATTRIBUTE_FALLTHROUGH],
		[1],
		[Define if [[fallthrough]] can be used])])

# Check if [[gnu::fallthrough]] works
AC_MSG_CHECKING([[whether [[gnu::fallthrough]] can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		]], [[
int i = 0;
switch(i) {
	case 0:
		--i;
		[[gnu::fallthrough]];
	default:
		return i + 1;
}
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_gnufallthrough], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_gnufallthrough], [false])])
AS_IF([$attr_gnufallthrough],
	[AC_DEFINE([HAVE_ATTRIBUTE_GNUFALLTHROUGH],
		[1],
		[Define if [[gnu::fallthrough]] can be used])])

# Check if __attribute__ ((fallthrough)) works
AC_MSG_CHECKING([[whether __attribute__ ((fallthrough)) can be used]])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
		]], [[
int i = 0;
switch(i) {
	case 0:
		--i;
		__attribute__ ((fallthrough));
	default:
		return i + 1;
}
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_afallthrough], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_afallthrough], [false])])
AS_IF([$attr_afallthrough],
	[AC_DEFINE([HAVE_ATTRIBUTE_AFALLTHROUGH],
		[1],
		[Define if __attribute__ ((fallthrough)) can be used])])

# Check if __attribute__ ((signal)) works
AC_MSG_CHECKING([whether __attribute__ ((signal)) can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <csignal>
__attribute__ ((signal)) void my_sig_handler(int sig);
class my_int {
friend __attribute__ ((signal)) void my_sig_handler(int sig);
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
		AS_VAR_SET([attr_signal], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_signal], [false])])
AS_IF([$attr_signal],
	[AC_DEFINE([HAVE_ATTRIBUTE_SIGNAL],
		[1],
		[Define if __attribute__ ((signal)) can be used])])

# Check if __attribute__ ((const)) works
AC_MSG_CHECKING([whether __attribute__ ((const)) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	static int c(int i) __attribute__ ((const)) { return i; }
};
int a(int i) __attribute__ ((const));
int a(int i) { b x; return x.c(i); }
		]], [[
return !((a(4) == 4) && (a(5) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_const], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_const], [false])])
AS_IF([$attr_const],
	[AC_DEFINE([HAVE_ATTRIBUTE_CONST],
		[1],
		[Define if __attribute__ ((const)) can be used])])

# Check if __attribute__ ((pure)) works
AC_MSG_CHECKING([whether __attribute__ ((pure)) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	static int c(int *i) __attribute__ ((pure)) { return *i; }
};
__attribute__ ((pure)) int a(int i);
int a(int i) { b x; return x.c(&i); }
		]], [[
return !((a(4) == 4) && (a(5) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_pure], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_pure], [false])])
AS_IF([$attr_pure],
	[AC_DEFINE([HAVE_ATTRIBUTE_PURE],
		[1],
		[Define if __attribute__ ((pure)) can be used])])

# I do not know a reliable way to check whether __attribute__ ((const))
# or  __attribute__ ((const)) can be used for const virtuals.
# If in doubt use __attribute__ ((pure)) but not __attribute__ ((const))

AC_MSG_CHECKING([whether __attribute__ ((pure)) should be used for const virtuals])
AC_ARG_ENABLE([pure_for_const_virtuals],
	[AS_HELP_STRING([--disable-pure-for-const-virtuals],
		[do not use __attribute__((pure)) for const virtuals])],
	[MV_ENABLE([pure_for_const_virtuals])
	AS_VAR_SET([cmt_pure_for_const_virtuals], ["on request"])],
	[AS_VAR_SET([pure_for_const_virtuals], [:])
	AS_VAR_SET([cmt_pure_for_const_virtuals], ["default"])])
MV_MSG_RESULT_BIN([$pure_for_const_virtuals], [$cmt_pure_for_const_virtuals])
AS_IF([$pure_for_const_virtuals],
	[AC_DEFINE([USE_PURE_FOR_CONST_VIRTUALS],
		[1],
		[Define if __attribute__ ((pure)) can be used for const virtuals])])

AC_MSG_CHECKING([whether __attribute__ ((const)) should be used for const virtuals])
AC_ARG_ENABLE([const_for_const_virtuals],
	[AS_HELP_STRING([--enable-const-for-const-virtuals],
		[use __attribute__((const)) for const virtuals])],
	[MV_ENABLE([const_for_const_virtuals])
	AS_VAR_SET([cmt_const_for_const_virtuals], ["on request"])],
	[AS_VAR_SET([const_for_const_virtuals], [false])
	AS_VAR_SET([cmt_const_for_const_virtuals], ["default"])])
MV_MSG_RESULT_BIN([$const_for_const_virtuals], [$cmt_const_for_const_virtuals])
AS_IF([$const_for_const_virtuals],
	[AC_DEFINE([USE_PURE_FOR_CONST_VIRTUALS],
		[1],
		[Define if __attribute__ ((const)) can be used for const virtuals])])

# Check if __attribute__ ((nonnull)) works
AC_MSG_CHECKING([whether __attribute__ ((nonnull)) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	__attribute__ ((nonnull)) int c(int *i) { return *i; }
};
int a(int *i) __attribute__ ((nonnull));
int a(int *i) { b x; return x.c(i); }
		]], [[
int i = 4, j = 5;
return !((a(&i) == 4) && (a(&j) == 5));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_nonnull_], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_nonnull_], [false])])
AS_IF([$attr_nonnull_],
	[AC_DEFINE([HAVE_ATTRIBUTE_NONNULL_],
		[1],
		[Define if __attribute__ ((nonnull)) can be used])])

# Check if __attribute__ ((nonnull (x, y))) works
AC_MSG_CHECKING([whether __attribute__ ((nonnull (x, y))) can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class b {
public:
	__attribute__ ((nonnull (1, 3))) static int c(int *i, int *j, int *k) {
		return (j ? ((*i) - (*k)) : ((*i) + (*k)));
	}
	__attribute__ ((nonnull (2, 4))) int d(int *i, int *j, int *k) {
		return c(i,j,k);
	}
};
__attribute__ ((nonnull (1, 3))) int a(int *i, int *j, int *k);
int a(int *i, int *j, int *k) {
	b x; return x.d(i, j, k) + x.d(i, 0, k);
}
		]], [[
int i = 1;
b x;
return !((x.d(&i, 0, &i) == 2) && (x.d(&i, 0, &i) == 2) && (a(&i, 0, &i) == 4));
		]])],
		[MV_MSG_RESULT([yes])
		AS_VAR_SET([attr_nonnull], [:])],
		[MV_MSG_RESULT([no])
		AS_VAR_SET([attr_nonnull], [false])])
AS_IF([$attr_nonnull],
	[AC_DEFINE([HAVE_ATTRIBUTE_NONNULL],
		[1],
		[Define if __attribute__ ((nonnull (list))) can be used])])

# Check if nullptr works
AC_MSG_CHECKING([whether nullptr can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
static const char *a = nullptr;
	]], [[
const char *b(nullptr);
return !((a == b) && (b == nullptr));
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_NULLPTR], [1],
			[Define if C++ dialect has nullptr type])],
		[MV_MSG_RESULT([no])])

# Check if constexpr works
AC_MSG_CHECKING([whether constexpr can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class T {
public:
	static constexpr const int a = 5;
};
const int T::a;
	]], [[
return (T::a != 5)
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_CONSTEXPR], [1],
			[Define if C++ dialect has constexpr modifier])],
		[MV_MSG_RESULT([no])])

# Check if overrride can be used
AC_MSG_CHECKING([whether override can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class T {
public:
	virtual int foo() {return 1;}
};
class S : public T {
public:
	int foo() override {return 0;}
};
	]], [[
S a;
T *b(&a);
return b->foo();
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_OVERRIDE], [1],
			[Define if C++ dialect has override modifier])],
		[MV_MSG_RESULT([no])])

# Check if final can be used
AC_MSG_CHECKING([whether final can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
class T final {
public:
	virtual int foo() final {return 0;}
};
	]], [[
T b;
return b.foo();
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_FINAL], [1],
			[Define if C++ dialect has final modifier])],
		[MV_MSG_RESULT([no])])

# Check if delete can be assigned
AC_MSG_CHECKING([whether delete can be assigned])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <utility>
class T {
private:
	T(const T& s) = delete;
	T& operator=(const T& s) = delete;
public:
	T() { }
};
	]], [[
T a;
return 0;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_DELETE], [1],
			[Define if C++ dialect supports assigning delete])],
		[MV_MSG_RESULT([no])])

# Check if noexcept can be used
AC_MSG_CHECKING([whether noexcept can be used])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[
class T {
public:
	int s;
	T() noexcept : s(0) {
	}
};
		]], [[
return T().s
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_NOEXCEPT], [1],
			[Define if C++ dialect supports noexcept])],
		[MV_MSG_RESULT([no])])

# Check if move constructor can be used
AC_MSG_CHECKING([whether move constructor can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <utility>
class T {
public:
	T() { }
	~T() { }
	T(T&&) { }
	T& operator=(T&&) { return *this; }
	int ret() { return 0; }
};
	]], [[
T a, b;
T c = std::move(a);
b = std::move(c);
return b.ret();
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_MOVE], [1],
			[Define if C++ dialect supports assigning default move constructor])],
		[MV_MSG_RESULT([no])])

dnl # Check if default move constructor can be asssigned
dnl AC_MSG_CHECKING([whether default move constructor can be assigned])
dnl MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
dnl #include <utility>
dnl class T {
dnl public:
dnl	T() { }
dnl	~T() { }
dnl	T(T&&) = default;
dnl	T& operator=(T&&) = default;
dnl	int ret() { return 0; }
dnl };
dnl	]], [[
dnl T a, b;
dnl T c = std::move(a);
dnl b = std::move(c);
dnl return b.ret();
dnl		]])],
dnl		[MV_MSG_RESULT([yes])
dnl		AC_DEFINE([ASSIGN_MOVE_DEFAULT], [1],
dnl			[Define if C++ dialect supports assigning default move constructor])],
dnl		[MV_MSG_RESULT([no])])

# Check if array class can be used
AC_MSG_CHECKING([whether array class can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <array>
	]], [[
std::array<int, 5> a;
a.fill(4);
return (a[4] == 4) ? 0 : 1;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_ARRAY_CLASS], [1],
			[Define if STL has array class])],
		[MV_MSG_RESULT([no])])

# Check if forward_list can be used
AC_MSG_CHECKING([whether forward_list can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <forward_list>
	]], [[
std::forward_list<int> a;
a.insert_after(a.before_begin(), 5);
return (*(a.begin()) == 5) ? 0 : 1;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_FORWARD_LIST], [1],
			[Define if STL has forward_list])],
		[MV_MSG_RESULT([no])])

# Check if unordered_set can be used
AC_MSG_CHECKING([whether unordered_set can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <unordered_set>
	]], [[
std::unordered_set<int> a;
a.insert(5);
return (a.count(5) == 1) ? 0 : 1;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_UNORDERED_SET], [1],
			[Define if STL has unordered_set])],
		[MV_MSG_RESULT([no])])

# Check if unordered_map can be used
AC_MSG_CHECKING([whether unordered_map can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <unordered_map>
	]], [[
std::unordered_map<int, int> a;
a[5] = 4;
return (a[5] == 4) ? 0 : 1;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_UNORDERED_MAP], [1],
			[Define if STL has unordered_map])],
		[MV_MSG_RESULT([no])])

# Check if emplace_back can be used
AC_MSG_CHECKING([whether emplace_back can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <utility>
#include <vector>
	]], [[
typedef std::pair<int, int> mypair;
std::vector<mypair> a;
a.push_back(mypair(4, 4));
a.emplace_back(5, 5);
return (a.size() == 2) ? 0 : 1;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_EMPLACE_BACK], [1],
			[Define if STL has emplace_back])],
		[MV_MSG_RESULT([no])])

# Check if emplace can be used
AC_MSG_CHECKING([whether emplace can be used])
MV_RUN_IFELSE_LINK([AC_LANG_PROGRAM([[
#include <set>
#include <utility>
	]], [[
typedef std::pair<int, int> mypair;
std::set<mypair> a;
a.insert(mypair(4, 4));
a.emplace(5, 5);
return (a.size() == 2) ? 0 : 1;
		]])],
		[MV_MSG_RESULT([yes])
		AC_DEFINE([HAVE_EMPLACE], [1],
			[Define if STL has emplace])],
		[MV_MSG_RESULT([no])])

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
	[AS_CASE(["$withval"],
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
AS_IF([$pkgcfg_check_sqlite],
	[PKG_CHECK_MODULES([SQLITE], [sqlite3],
		[AS_VAR_SET([support_sqlite], [:])],
		[AS_IF([$support_sqlite],
			[MV_MSG_RESULT([yes], [although pkg-config failed])
			AS_VAR_SET([manual_sqlite], [:])],
			[MV_MSG_RESULT([no], [autodetected])])])])
AS_IF([$manual_sqlite],
	[AS_VAR_SET([SQLITE_LIBS], ["-lsqlite3"])
	AS_VAR_SET([SQLITE_CFLAGS], [])])
AS_IF([$support_sqlite],
	[AC_DEFINE([WITH_SQLITE],
		[1],
		[Define if cache method sqlite is wanted])],
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
MV_MSG_RESULT_VAR([PORTDIR_CACHE_METHOD], [$description])
AC_DEFINE_UNQUOTED([PORTDIR_CACHE_METHOD],
	["$PORTDIR_CACHE_METHOD"],
	[Default PORTDIR_CACHE_METHOD])
AC_SUBST([PORTDIR_CACHE_METHOD])

AC_MSG_CHECKING([EPREFIX default])
AC_ARG_WITH([eprefix-default],
	[AS_HELP_STRING([--with-eprefix-default=STR],
		[default EPREFIX])],
	[AS_VAR_COPY([EPREFIX_DEFAULT], [withval])],
	[AS_VAR_SET([EPREFIX_DEFAULT],
		["`eval portageq envvar EPREFIX 2>$DEV_NULL`"])])
MV_PREFIX(EPREFIX_DEFAULT)
MV_MSG_RESULT_VAR([EPREFIX_DEFAULT])
AC_DEFINE_UNQUOTED([EPREFIX_DEFAULT],
	["$EPREFIX_DEFAULT"],
	[Default EPREFIX])
AC_SUBST([EPREFIX_DEFAULT])

AC_MSG_CHECKING([for sh shebang])
AS_VAR_SET([SH_SHEBANG], [])
AC_ARG_WITH([sh-shebang],
	[AS_HELP_STRING([--with-sh-shebang],
		[use argument as sh script shebang. Pass auto or yes for autodetection])],
	[AS_CASE(["$withval"],
		[auto], [],
		[yes], [],
		[AS_VAR_COPY([SH_SHEBANG], [withval])])],
	[MV_IF_NONEMPTY([$EPREFIX_DEFAULT],
		[AS_VAR_SET([SH_SHEBANG], [no])])])
MV_IF_EMPTY([$SH_SHEBANG],
	[AS_VAR_SET([SH_SHEBANG], ["`eval command -v sh 2>$DEV_NULL`"])])
AS_VAR_IF([SH_SHEBANG], [no],
	[AS_VAR_SET([SH_SHEBANG], [])])
MV_IF_EMPTY([$SH_SHEBANG],
	[AS_VAR_SET([SH_SHEBANG], ["/usr/bin/env sh"])])
AC_MSG_RESULT([$SH_SHEBANG])
AC_SUBST([SH_SHEBANG])

AC_MSG_CHECKING([EIX_PREFIX default])
AC_ARG_WITH([eix-prefix-default],
	[AS_HELP_STRING([--with-eix-prefix-default=STR],
		[default EIX_PREFIX])],
	[AS_VAR_COPY([EIX_PREFIX_DEFAULT], [withval])],
	[AS_VAR_SET([EIX_PREFIX_DEFAULT], [])])
MV_PREFIX(EPREFIX_DEFAULT)
MV_MSG_RESULT_VAR([EIX_PREFIX_DEFAULT])
AC_DEFINE_UNQUOTED([EIX_PREFIX_DEFAULT],
	["$EIX_PREFIX_DEFAULT"],
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
	["$ROOT_DEFAULT"],
	[Default ROOT])
AC_SUBST([ROOT_DEFAULT])

AC_MSG_CHECKING([PORTAGE_ROOTPATH default])
AC_ARG_WITH([portage-rootpath],
	[AS_HELP_STRING([--with-portage-rootpath=STR],
		[default PORTAGE_ROOTPATH])],
	[AS_VAR_COPY([PORTAGE_ROOTPATH_DEFAULT], [withval])],
	[AS_VAR_SET([PORTAGE_ROOTPATH_DEFAULT], [])])
MV_MSG_RESULT_VAR([PORTAGE_ROOTPATH_DEFAULT])
AC_DEFINE_UNQUOTED([PORTAGE_ROOTPATH_DEFAULT],
	["$PORTAGE_ROOTPATH_DEFAULT"],
	[Default PORTAGE_ROOTPATH])
AC_SUBST([PORTAGE_ROOTPATH_DEFAULT])

AC_MSG_CHECKING([default DEFAULT_ARCH])
AC_ARG_WITH([default-arch],
	[AS_HELP_STRING([--with-default-arch=STR],
		[default ARCH])],
	[AS_VAR_COPY([ARCH_DEFAULT], [withval])],
	[AS_VAR_COPY([ARCH_DEFAULT], [ARCH])])
MV_MSG_RESULT_VAR([ARCH_DEFAULT])
AC_DEFINE_UNQUOTED([ARCH_DEFAULT],
	["$ARCH_DEFAULT"],
	[Default DEFAULT_ARCH])
AC_SUBST([ARCH_DEFAULT])

AC_MSG_CHECKING([swap remote paths])
AC_ARG_ENABLE([swap-remote],
	[AS_HELP_STRING([--enable-swap-remote],
		[swap the remote paths])],
	[MV_ENABLE([swap_remote])],
	[AS_VAR_SET([swap_remote], [false])])
MV_MSG_RESULT_BIN([$swap_remote])

AC_MSG_CHECKING([default remote path 1])
AC_ARG_WITH([remote-file1],
	[AS_HELP_STRING([--with-remote-path1=STR],
		[default remote path 1 for eix-remote])],
	[AS_VAR_COPY([DEFAULT_REMOTE_PATH1], [withval])],
	[AS_VAR_SET([DEFAULT_REMOTE_PATH1],
		[http://gpo.zugaina.org/eix_cache/eix-cache.tbz2])])
AC_ARG_WITH([remote-file2],
	[AS_HELP_STRING([--with-remote-path2=STR],
		[default remote path 2 for eix-remote])],
	[AS_VAR_COPY([DEFAULT_REMOTE_PATH2], [withval])],
	[AS_VAR_SET([DEFAULT_REMOTE_PATH2],
		[http://dev.gentooexperimental.org/eix_cache/eix-caches.tbz2])])
AS_IF([$swap_remote],
	[AS_VAR_COPY([swap_remote], [DEFAULT_REMOTE_PATH1])
	AS_VAR_COPY([DEFAULT_REMOTE_PATH1], [DEFAULT_REMOTE_PATH2])
	AS_VAR_COPY([DEFAULT_REMOTE_PATH2], [swap_remote])])
MV_MSG_RESULT_VAR([DEFAULT_REMOTE_PATH1])
AC_SUBST([DEFAULT_REMOTE_PATH1])
AC_MSG_CHECKING([default remote path 2])
MV_MSG_RESULT_VAR([DEFAULT_REMOTE_PATH2])
AC_SUBST([DEFAULT_REMOTE_PATH2])

AC_MSG_CHECKING([ALWAYS_ACCEPT_KEYWORDS default])
AC_ARG_WITH([always-accept-keywords],
	[AS_HELP_STRING([--with-always-accept-keywords],
		[default always-accept-keywords])],
	[AS_CASE(["$withval"],
		[no], [AS_VAR_SET([ALWAYS_ACCEPT_KEYWORDS_DEFAULT], ["false"])],
		[AS_VAR_SET([ALWAYS_ACCEPT_KEYWORDS_DEFAULT], ["true"])])],
	[AS_VAR_SET([ALWAYS_ACCEPT_KEYWORDS_DEFAULT], ["false"])])
MV_MSG_RESULT_VAR([ALWAYS_ACCEPT_KEYWORDS_DEFAULT])
AC_DEFINE_UNQUOTED([ALWAYS_ACCEPT_KEYWORDS_DEFAULT],
	["$ALWAYS_ACCEPT_KEYWORDS_DEFAULT"],
	[Default ALWAYS_ACCEPT_KEYWORDS])
AC_SUBST([ALWAYS_ACCEPT_KEYWORDS_DEFAULT])

AC_MSG_CHECKING([DEP default])
AC_ARG_WITH([dep-default],
	[AS_HELP_STRING([--with-dep-default],
		[default to DEP=true])],
	[AS_CASE(["$withval"],
		[no], [AS_VAR_SET([DEP_DEFAULT], ["false"])],
		[AS_VAR_SET([DEP_DEFAULT], ["true"])])],
	[AS_VAR_SET([DEP_DEFAULT], ["false"])])
MV_MSG_RESULT_VAR([DEP_DEFAULT])
AC_DEFINE_UNQUOTED([DEP_DEFAULT],
	["$DEP_DEFAULT"],
	[Default DEP])
AC_SUBST([DEP_DEFAULT])

AC_MSG_CHECKING([REQUIRED_USE default])
AC_ARG_WITH([required-use-default],
	[AS_HELP_STRING([--with-required-use-default],
		[default to REQUIRED_USE=true])],
	[AS_CASE(["$withval"],
		[no], [AS_VAR_SET([REQUIRED_USE_DEFAULT], ["false"])],
		[AS_VAR_SET([REQUIRED_USE_DEFAULT], ["true"])])],
	[AS_VAR_SET([REQUIRED_USE_DEFAULT], ["false"])])
MV_MSG_RESULT_VAR([REQUIRED_USE_DEFAULT])
AC_DEFINE_UNQUOTED([REQUIRED_USE_DEFAULT],
	["$REQUIRED_USE_DEFAULT"],
	[Default REQUIRED_USE])
AC_SUBST([REQUIRED_USE_DEFAULT])

AC_MSG_CHECKING([zsh completion])
AS_VAR_SET([ZSH_COMPLETION], [$datadir/zsh/site-functions])
AC_ARG_WITH([zsh-completion],
	[AS_HELP_STRING([--with-zsh-completion=STR],
		[install zsh-completion file in directory STR])],
	[AS_CASE([$withval],
		[/*], [AS_VAR_COPY([ZSH_COMPLETION], [withval])],
		[no], [AS_VAR_SET([ZSH_COMPLETION], [])])])
MV_MSG_RESULT_VAR([ZSH_COMPLETION])
AC_SUBST([ZSH_COMPLETION])
MV_IF_EMPTY([$ZSH_COMPLETION],
	[AS_VAR_SET([install_zsh_completion], [false])],
	[AS_VAR_SET([install_zsh_completion], [:])])
AM_CONDITIONAL([INSTALL_ZSH_COMPLETION],
	[$install_zsh_completion])

# Provide basic types:
AC_TYPE_UINT8_T()
AC_TYPE_UINT16_T()
AC_TYPE_UINT32_T()

AS_VAR_SET([EIX_CACHEFILE], [/var/cache/eix/portage.eix])
AC_DEFINE_UNQUOTED([EIX_CACHEFILE],
	["$EIX_CACHEFILE"],
	[Location of the cachefile])
AC_SUBST([EIX_CACHEFILE])

AS_VAR_SET([EIX_PREVIOUS], [/var/cache/eix/previous.eix])
AC_DEFINE_UNQUOTED([EIX_PREVIOUS],
	["$EIX_PREVIOUS"],
	[Location of the previous cachefile])
AC_SUBST([EIX_PREVIOUS])

AS_VAR_SET([EIX_REMOTECACHEFILE1], [/var/cache/eix/remote.eix])
AC_DEFINE_UNQUOTED([EIX_REMOTECACHEFILE1],
	["$EIX_REMOTECACHEFILE1"],
	[Location of the remote cachefile 1])
AC_SUBST([EIX_REMOTECACHEFILE1])

AS_VAR_SET([EIX_REMOTECACHEFILE2], [/var/cache/eix/remote2.eix])
AC_DEFINE_UNQUOTED([EIX_REMOTECACHEFILE2],
	["$EIX_REMOTECACHEFILE2"],
	[Location of the remote cachefile 2])
AC_SUBST([EIX_REMOTECACHEFILE2])

AS_VAR_SET([EIX_REMOTEARCHIVE1], [/var/cache/eix/remote.tar.bz2])
AC_DEFINE_UNQUOTED([EIX_REMOTEARCHIVE1],
	["$EIX_REMOTEARCHIVE1"],
	[Location of the local remote cache archive 1])
AC_SUBST([EIX_REMOTEARCHIVE1])

AS_VAR_SET([EIX_REMOTEARCHIVE2], [/var/cache/eix/remote2.tar.bz2])
AC_DEFINE_UNQUOTED([EIX_REMOTEARCHIVE2],
	["$EIX_REMOTEARCHIVE2"],
	[Location of the local remote cache archive 2])
AC_SUBST([EIX_REMOTEARCHIVE2])

AC_DEFINE_UNQUOTED([LEVENSHTEIN_DISTANCE_DEFAULT],
	["2"],
	[Default Levenshtein distance for matches (as string)])
AC_SUBST([LEVENSHTEIN_DISTANCE_DEFAULT])

m4_if([dnl This is the second part of the check near the top of this file.
dnl To use it you must also uncomment the above check
# Use -include config.h if possible to get more hits with ccache
MV_IF_NONEMPTY([$add_incopt],
	[AS_VAR_SET([CXXFLAGS], ["$add_incopt $CXXFLAGS"])
	AS_VAR_SET([my_cxxadd], ["$add_incopt $my_cxxadd"])])])

# Done!
AC_OUTPUT()

AS_ECHO(["

$PACKAGE-$VERSION configured successfully.

"])
MV_IF_NONEMPTY([$CXX],
	[AS_ECHO(["CXX: $CXX
"])])
AS_ECHO(["CXXFLAGS: $oricxxflags"])
MV_IF_NONEMPTY([$my_cxxadd],
	[AS_ECHO(["PREPEND_CXXFLAGS: $my_cxxadd
"])])
AS_ECHO(["LDFLAGS: $orildflags"])
MV_IF_NONEMPTY([$my_ldadd],
	[AS_ECHO(["PREPEND_LDFLAGS: $my_ldadd"])])
AS_ECHO([])
