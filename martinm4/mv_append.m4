dnl This file is part of the eix project and distributed under the
dnl terms of the GNU General Public License v2.
dnl
dnl Copyright (c)
dnl  Martin Väth <vaeth@mathematik.uni-wuerzburg.de>
dnl
dnl MV_APPEND([VAR], [text], [sep])
dnl Append [text] to VAR, inserting  [sep] if VAR and [text] are nonempty.
dnl If [sep] is empty, a space is assumed as [sep].
AC_DEFUN([MV_APPEND],
	[MV_IF_NONEMPTY([$2],
		[MV_IF_EMPTY([${$1}],
			[$1="$2"],
			[$1="${$1}m4_ifval([$3], [$3], [ ])$2"])])])
dnl
dnl MV_PREPEND([VAR], [text], [sep])
dnl Prepend [text] to VAR, inserting  [sep] if VAR and [text] are nonempty.
dnl If [sep] is empty, a space is assumed as [sep].
AC_DEFUN([MV_PREPEND],
	[MV_IF_NONEMPTY([$2],
		[MV_IF_EMPTY([${$1}],
			[$1="$2"],
			[$1="$2m4_ifval([$3], [$3], [ ])${$1}"])])])