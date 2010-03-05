#! /usr/bin/env sh

run () {
	printf '%s\n' ">>> ${*}" 1>&2
	if ! "${@}"
	then	printf '%s\n' "failure" 1>&2
		exit 1
	fi
}

run mkdir -p config
if command -v glibtoolize >/dev/null 2>&1
then	: run glibtoolize --force --copy --automake
else	: run libtoolize --force --copy --automake
fi
run autopoint
run aclocal -I m4 -I martinm4
run autoconf
run autoheader
run automake -af --copy
