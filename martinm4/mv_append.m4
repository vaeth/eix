dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_APPEND([VAR], [text], [separator])
dnl Append [text] to VAR, inserting  [separator] if VAR and [text] are nonempty.
dnl If [separator] is empty, a space is assumed as a separator.
AC_DEFUN([MV_APPEND],
	[MV_IF_NONEMPTY([$2],
		[MV_IF_EMPTY([${$1}],
			[$1="$2"],
			[$1="${$1}m4_ifval([$3], [$3], [ ])$2"])])])
