dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ADDFLAGS([CXXFLAGS], [list of flags], [mode]) adds flags to CXXFLAGS
dnl if known to compiler or if the third argument is given and true.
AC_DEFUN([MV_ADDFLAGS],
	[export $1
	for mv_currflag in $2
	do
		AS_CASE([" ${$1} "],
			[*" ${mv_currflag} "*], [],
			[AC_MSG_CHECKING([whether flag ${mv_currflag} is known])
			mv_saveflags="${$1}"
			$1="${$1} ${mv_currflag}"
			AS_IF([m4_ifval([$3], [$3], [false])],
				[MV_MSG_RESULT([yes], [quickcheck])],
				[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
					[AC_MSG_RESULT([yes])],
					[AC_MSG_RESULT([no])
					$1="${mv_saveflags}"])])])
	done])
