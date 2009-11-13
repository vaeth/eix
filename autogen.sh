#! /usr/bin/env sh

run ()
{
	echo ">>> ${*}" 1>&2
	if ! "${@}"
	then
		echo BUMMM 1>&2
		exit 1
	fi
}

run mkdir -p config
if command -v glibtoolize >/dev/null 2>&1
then
	echo Skipping glibtoolize --force --copy --automake
else
	echo Skipping libtoolize --force --copy --automake
fi
run autopoint
run aclocal -I m4 -I martinm4
run autoconf
run autoheader
run automake -af --copy
