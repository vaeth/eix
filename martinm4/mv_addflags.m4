dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin V\"ath <martin@mvath.de>
dnl
dnl MV_ADDFLAGS([ADDCFLAGS], [CFLAGS], [AC_LINK_IFELSE],
dnl     [sh-list of flags], [sh-list of fatal-flags],
dnl     [sh-list of skip-flags, e.g. $CPPFLAGS], [action], [mode])
dnl adds each flag of [sh-list of flags] to [ADDCFLAGS] if [AC_LINK_IFELSE]
dnl succeeds with the corresponding [CFLAGS]. The flag is skipped if it is
dnl already contained in [ADDCFLAGS] or in [CFLAGS] or in
dnl [sh-list of skip-flags] or if it was already tested earlier
dnl (in case of positive earlier test, it is added, of course).
dnl [sh-list of skip-flags] must be *space* separated (not by newlines, tabs).
dnl It is admissible that [ADDCFLAGS] is the same as [CFLAGS].
dnl If [mode] is given and :/false, no COMPILE|LINK is done but just the
dnl result of [mode] is used.
dnl The [sh-list of fatal-flags] (if given) is added to CFLAGS during testing.
dnl The idea of the latter is that [fatal-flags] should be something like
dnl -Werror to make compile/link really fail on false flags and thus to
dnl avoid things like http://bugs.gentoo.org/show_bug.cgi?id=209239
dnl If [action] is nonempty then in case of a positive match the flag is
dnl also prepended to CFLAGS, and [action] is executed. action=break means that
dnl after a successful match no further tests are attempted.
dnl Note that if [ADDFLAGS] = [CFLAGS] and [action] is nonempty then
dnl the flag is appended and prepended, hence added twice; so better use
dnl a dummy variable in [ADDFLAGS] in this case.
AC_DEFUN([MV_ADDFLAGS],
	[export $2
	for mv_currflag in $4
	do
		AS_CASE([" $$1 $$2 $6 $mv_f$2_cache "],
			[*" $mv_currflag "*], [],
			[AS_CASE([" ${mv_s$2_cache} "],
				[*" $mv_currflag "*],
					[AS_VAR_SET([mv_result], [:])],
				[AC_MSG_CHECKING([whether $2=$mv_currflag is known])
				MV_IF_EMPTY([$8],
					[AS_VAR_COPY([mv_saveflags], [$2])
					MV_APPEND([$2], [$5])
					MV_APPEND([$2], [$mv_currflag])
					m4_indir([$3],
dnl The program should use STL and C libraries and "stress" the linker with
dnl constant and dynamic variables, but also be standard, short and quick...
dnl It should also use a larger local array to test the stack.
dnl The unistd.h and string are here to trigger clang/gcc/glibc incompatibily,
dnl see https://bugs.gentoo.org/show_bug.cgi?id=510102
						[AC_LANG_PROGRAM([[
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
int my_func();
int my_func() { std::vector<const char*> a(1, "a"); int my_a[1000];
for (std::vector<const char*>::size_type j = 999; j < 999 + a.size(); ++j) my_a[j] = 0;
return (strchr(a[0], *a[0]) == *(a.begin())) ? my_a[999] : 1; }
#ifdef __cplusplus
#if __cplusplus >= 201103L
#include <type_traits>
void func_noexcept() noexcept;
void func_noexcept() noexcept {}
#define C11TESTCALL if(!std::is_function<decltype(func_noexcept)>::value) { return 1; }
#else
#define C11TESTCALL
#endif
#else
#define C11TESTCALL
#endif
						]], [[
C11TESTCALL
return my_func();
						]])],
						[AC_MSG_RESULT([yes])
						AS_VAR_SET([mv_result], [:])],
						[AC_MSG_RESULT([no])
						AS_VAR_SET([mv_result], [false])])
					AS_VAR_COPY([$2], [mv_saveflags])],
					[AS_IF([$8],
						[MV_MSG_RESULT([yes], [on request])
						AS_VAR_SET([mv_result], [:])],
						[MV_MSG_RESULT([no], [on request])
						AS_VAR_SET([mv_result], [false])])])
				AS_IF([$mv_result],
					[MV_APPEND([mv_s$2_cache],
						[$mv_currflag])],
					[MV_APPEND([mv_f$2_cache],
						[$mv_currflag])])])
			AS_IF([$mv_result],
				MV_APPEND([$1], [$mv_currflag])
				[m4_ifval([$7],
					[MV_PREPEND([$2], [$mv_currflag])
					$7
					])])])
	done])
