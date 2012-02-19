dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin V\"ath <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ENABLE([option]):
dnl Checks whether ${enableval} is yes or no and sets ${option}=:/false.
dnl (the variable undergoes AS_TR_SH, of course).
dnl If neither is the case, it dies with an error output for --enable-option
AC_DEFUN([MV_ENABLE],
	[AS_CASE(["${enableval}"],
		[yes], [AS_VAR_SET([AS_TR_SH([$1])], [:])],
		[no], [AS_VAR_SET([AS_TR_SH([$1])], [false])],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-$1])])])
