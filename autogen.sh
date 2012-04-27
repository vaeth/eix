#! /usr/bin/env sh

Echo() {
	printf '%s\n' "${*}" >&2
}

Die() {
	Echo "${*}"
	exit 1
}

Run() {
	Echo ">>> ${*}"
	"${@}" || Die 'failure'
}

Run mkdir -p config
Run autopoint
Run aclocal -I m4 -I martinm4
Run autoconf
Run autoheader
Run automake -a --copy "${@}"
