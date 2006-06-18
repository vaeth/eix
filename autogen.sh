#!/bin/bash

run ()
{
	echo ">>> $@" 1>&2
	if ! "$@"; then
		echo BUMMM 1>&2
		exit 1
	fi
}

run mkdir -p config 
run libtoolize --force --copy --automake
run aclocal
run autoheader
run autoconf
run automake -af --copy
