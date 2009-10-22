dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ADDLDFLAGS([ADDLDFLAGS], [list of flags], [fatal-flags], [mode])
dnl adds flags to ADDLDFLAGS if known to the linker or if [mode] is given
dnl and true; flags contained in ADDLDFLAGS or LDFLAGS are always skipped.
dnl It is admissible that [ADDCXXFLAGS] is [CXXFLAGS].
dnl The [fatal-flags] (if given) are added to LDFLAGS during testing.
dnl The idea of the latter is that this should be something like
dnl -Wl,--fatal-warnings to avoid false positives as in
dnl http://bugs.gentoo.org/show_bug.cgi?id=209239
AC_DEFUN([MV_ADDLDFLAGS],
	[export LDFLAGS
	for mv_currflag in $2
	do
		AS_CASE([" ${LDFLAGS} ${$1} "],
			[*" ${mv_currflag} "*], [],
			[AC_MSG_CHECKING([whether LDFLAG ${mv_currflag} is known])
			mv_saveflags="${LDFLAGS}"
			LDFLAGS="${LDFLAGS} ${mv_currflag} $3"
			AS_IF([m4_ifval([$4], [$4], [false])],
				[MV_MSG_RESULT([yes], [quickcheck])
				$1="${$1} ${mv_currflag}"],
				[AC_LINK_IFELSE([AC_LANG_PROGRAM([[]],[[]])],
					[AC_MSG_RESULT([yes])
					LDFLAGS="${mv_saveflags}"
					$1="${$1} ${mv_currflag}"],
					[AC_MSG_RESULT([no])
					LDFLAGS="${mv_saveflags}"])])])
	done])
