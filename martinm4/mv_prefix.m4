dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_PREFIX([var]) sets var to "" if it is "/".
AC_DEFUN([MV_PREFIX],
	[AS_IF([test x"${$1}" = x"/"],
		[$1=""])])
