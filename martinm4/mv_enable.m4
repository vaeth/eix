dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_ENABLE([varname], [option]):
dnl Checks whether ${enableval} is yes or no and sets ${varname}=true/false.
dnl If neither is the case, it dies with an error output for --enable-option
AC_DEFUN([MV_ENABLE],
	[AS_CASE(["${enableval}"],
		[yes], [$1=true],
		[no], [$1=false],
		[AC_MSG_ERROR([bad value ${enableval} for --enable-$2])])])
