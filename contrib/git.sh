#!/usr/bin/env sh
set -u

export LC_ALL=C
umask 022

Echo() {
	printf '%s\n' "$*"
}

Die() {
	s=$?
	Echo "$1" >&2
	exit ${2:-$s}
}

ExecDie() {
	Echo "# $*"
	"$@" || Die "$* returned $?"
}

ExecWarn() {
	Echo "# $*"
	"$@" || Echo "warning: $* returned $?"
}

Usage() {
	Echo "Usage: $0 commit [--amend]
or     $0 l[og] [version]
or     $0 gc
or     $0 h[elp]"
	exit ${1:-1}
}

mode=$1
[ -n "$mode" ] && shift
case $mode in
commit)
	ExecDie git add --all .
	ExecDie git commit -a ${1+"$@"};;
gc)
	ExecWarn git prune
	ExecWarn git repack -a -d
	ExecWarn git gc --aggressive
	ExecWarn git repack -a -d
	ExecWarn git prune;;
l|log)
	ExecDie git log --decorate --graph --all --full-history ${1+"$@"};;
h|help)
	Usage 0;;
*)
	Usage;;
esac
