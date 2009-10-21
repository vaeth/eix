dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ADDLDFLAGS([list of flags], [mode]) adds flags to LDFLAGS
dnl if known to compiler or if the second argument is given and true.
AC_DEFUN([MV_ADDLDFLAGS],
	[export LDFLAGS
	for mv_currflag in $1
	do
		AS_CASE([" ${LDFLAGS} "],
			[*" ${mv_currflag} "*], [],
			[AC_MSG_CHECKING([whether LDFLAG ${mv_currflag} is known])
			mv_saveflags="${LDFLAGS}"
			LDFLAGS="${LDFLAGS} ${mv_currflag}"
			AS_IF([m4_ifval([$2], [$2], [false])],
				[MV_MSG_RESULT([yes], [quickcheck])],
				[AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
					[AC_MSG_RESULT([yes])],
					[AC_MSG_RESULT([no])
					LDFLAGS="${mv_saveflags}"])])])
	done])
