dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_MSG_RESULT([text], [comment]): behaves like
dnl AC_MSG_RESULT([text (comment)]) or AC_MSG_RESULT([text])
dnl depending on whether the argument comment is given or not.
AC_DEFUN([MV_MSG_RESULT],
	[m4_ifval([$2],
		[AC_MSG_RESULT([$1 ($2)])],
		[AC_MSG_RESULT([$1])])])
dnl
dnl MV_MSG_RESULTVAR([${var}], [comment]) behaves like
dnl MV_MSG_RESULT([${var:-(empty)}],[comment]) (but is more compatible)
AC_DEFUN([MV_MSG_RESULTVAR],
	[AS_IF([test -z "$1"],
		[MV_MSG_RESULT([(empty)], $2)],
		[MV_MSG_RESULT([$1], $2)])])
