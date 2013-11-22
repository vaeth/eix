dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin V\"ath <martin@mvath.de>
dnl
dnl MV_RUN_IFELSE_LINK(INPUT, [ACTION-IF-TRUE], [ACTION-IF-FALSE],)
dnl same as AC_RUN_IFELSE or AC_LINK_IFELSE if crosscompiling, respectively
dnl
AC_DEFUN([MV_RUN_IFELSE_LINK],
	[AC_RUN_IFELSE([$1], [$2], [$3],
		[AC_LINK_IFELSE([$1], [$2], [$3])])])

dnl MV_RUN_IFELSE_FALSE(INPUT, [ACTION-IF-TRUE], [ACTION-IF-FALSE],)
dnl as AC_RUN_IFELSE or ACTION-IF-FALSE if crosscompiling, respectively
dnl
AC_DEFUN([MV_RUN_IFELSE_FALSE],
	[AC_RUN_IFELSE([$1], [$2], [$3], [$3])])

dnl MV_RUN_IFELSE_TRUE(INPUT, [ACTION-IF-TRUE], [ACTION-IF-FALSE],)
dnl as AC_RUN_IFELSE or ACTION-IF-TRUE if crosscompiling, respectively
dnl
AC_DEFUN([MV_RUN_IFELSE_TRUE],
	[AC_RUN_IFELSE([$1], [$2], [$3], [$2])])
