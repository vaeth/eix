dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin V\"ath <martin@mvath.de>
dnl
dnl MV_MSG_RESULT([text], [comment]): behaves like
dnl AC_MSG_RESULT([text (comment)]) or AC_MSG_RESULT([text])
dnl depending on whether the argument comment is given and nonempty or not.
dnl Do not quote text or comment!
AC_DEFUN([MV_MSG_RESULT],
	[MV_IF_NONEMPTY([$2],
		[AC_MSG_RESULT([$1 ($2)])],
		[AC_MSG_RESULT([$1])])])
dnl
dnl MV_MSG_RESULT_VAR([var], [comment]) behaves like
dnl MV_MSG_RESULT([${var:-(empty)}], [comment]) (but is more compatible)
dnl Do not quote comment!
AC_DEFUN([MV_MSG_RESULT_VAR],
	[AS_VAR_IF([$1], [],
		[MV_MSG_RESULT([(empty)], [$2])],
		[MV_MSG_RESULT([$$1], [$2])])])
dnl
dnl MV_MSG_RESULT_BIN([res], [comment] behaves like
dnl MV_MSG_RESULT([yes/no], [comment]) depending in whether res is true
dnl Do not quote comment!
AC_DEFUN([MV_MSG_RESULT_BIN],
	[AS_IF([$1],
		[MV_MSG_RESULT([yes], $2)],
		[MV_MSG_RESULT([no], $2)])])
