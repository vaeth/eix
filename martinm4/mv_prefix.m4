dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin V\"ath <martin@mvath.de>
dnl
dnl MV_PREFIX([var]) sets var to "" if it is "/".
AC_DEFUN([MV_PREFIX],
	[AS_VAR_IF([$1], ["/"],
		[AS_VAR_SET([$1], [])])])
