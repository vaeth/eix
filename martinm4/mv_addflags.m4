dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ADDFLAGS([ADDCFLAGS], [CFLAGS], [COMPILE|LINK|RUN],
dnl     [sh-list of flags], [sh-list of fatal-flags], [mode])
dnl adds each flag of [sh-list of flags] to [ADDCFLAGS] if [COMPILE|LINK|RUN]
dnl succeeds with the corresponding [CFLAGS]. The flag is skipped if it is
dnl already contained in [ADDCFLAGS] or in [CFLAGS] or if it was already
dnl tested earlier (in case of positive earlier test, it is added, of course).
dnl It is admissible that [ADDCFLAGS] is the same as [CFLAGS].
dnl If [mode] is given and :/false, no COMPILE|LINK is done but just the
dnl result of [mode] is used.
dnl The [sh-list of fatal-flags] (if given) is added to CFLAGS during testing.
dnl The idea of the latter is that [fatal-flags] should be something like
dnl -Werror to make compile/link really fail on false flags and thus to
dnl avoid things like http://bugs.gentoo.org/show_bug.cgi?id=209239
AC_DEFUN([MV_ADDFLAGS],
	[export $2
	for mv_currflag in $4
	do
		AS_CASE([" ${$1} ${$2} ${mv_f$2_cache} "],
			[*" ${mv_currflag} "*], [],
			[AS_CASE([" ${mv_s$2_cache} "],
				[*" ${mv_currflag} "*], [mv_result=:],
				[AC_MSG_CHECKING([whether $2=${mv_currflag} is known])
				MV_IF_EMPTY([$6],
					[AS_VAR_COPY([mv_saveflags], [$2])
					MV_APPEND([$2], [$5])
					MV_APPEND([$2], [${mv_currflag}])
					m4_indir([AC_$3_IFELSE],
dnl The program should use STL and C libraries and "stress" the linker with
dnl constant and dynamic variables, but also be standard, short and quick...
dnl It should also use a larger local array to test the stack.
						[AC_LANG_PROGRAM([[
#include <vector>
#include <cstring>
};
						]], [[
std::vector<const char*> a(1, "a");
return (strchr(a[0], *a[0]) == *(a.begin())) ? my_a[999] : 1;
						]])],
						[AC_MSG_RESULT([yes])
						AS_VAR_SET([mv_result], [:])],
						[AC_MSG_RESULT([no])
						AS_VAR_SET([mv_result], [false])])
					AS_VAR_COPY([$2], [mv_saveflags])],
					[AS_IF([$6],
						[MV_MSG_RESULT([yes], [on request])
						AS_VAR_SET([mv_result], [:])],
						[MV_MSG_RESULT([no], [on request])
						AS_VAR_SET([mv_result], [false])])])
				AS_IF([${mv_result}],
					[MV_APPEND([mv_s$2_cache],
						[${mv_currflag}])],
					[MV_APPEND([mv_f$2_cache],
						[${mv_currflag}])])])
			AS_IF([${mv_result}],
				[MV_APPEND([$1], [${mv_currflag}])])])
	done])
