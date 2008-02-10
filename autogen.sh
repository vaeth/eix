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
type glibtoolize >/dev/null 2>&1 && run glibtoolize --force --copy --automake || run libtoolize --force --copy --automake
run aclocal
run autoheader
run autoconf
run automake -af --copy
