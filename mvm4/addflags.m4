dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ADDFLAGS([CXXFLAGS], [ADDCXXFLAGS],
dnl	[list of flags],
dnl	[fatal-flags], [mode])
dnl adds flags to ADDCXXFLAGS if known to compiler or if [mode] is given
dnl and true; flags contained in ADDCXXFLAGS or CXXFLAGS are always skipped.
dnl It is admissible that [ADDCXXFLAGS] is the same as [CXXFLAGS].
dnl The [fatal-flags] (if given) are added to CXXFLAGS during testing.
dnl The idea of the latter is that this should be something like
dnl -Werror to avoid false positives as in
dnl http://bugs.gentoo.org/show_bug.cgi?id=209239
AC_DEFUN([MV_ADDFLAGS],
	[export $1
	for mv_currflag in $3
	do
		AS_CASE([" ${$1} ${$2} "],
			[*" ${mv_currflag} "*], [],
			[AC_MSG_CHECKING([whether flag ${mv_currflag} is known])
			mv_saveflags="${$1}"
			$1="${$1} ${mv_currflag} $4"
			AS_IF([m4_ifval([$5], [$5], [false])],
				[MV_MSG_RESULT([yes], [quickcheck])
				$2="${$2} ${mv_currflag}"],
				[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
					[AC_MSG_RESULT([yes])
					$1="${mv_saveflags}"
					$2="${$2} ${mv_currflag}"],
					[AC_MSG_RESULT([no])
					$1="${mv_saveflags}"])])])
	done])
