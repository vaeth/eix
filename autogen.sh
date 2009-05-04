#! /usr/bin/env sh

run ()
{
	echo ">>> $@" 1>&2
	if ! "$@"; then
		echo BUMMM 1>&2
		exit 1
	fi
}

run mkdir -p config
if command -v glibtoolize >/dev/null 2>&1
then
	run glibtoolize --force --copy --automake
else
	run libtoolize --force --copy --automake
fi
run autopoint
run aclocal
run autoconf
run autoheader
run automake -af --copy
