#!/usr/bin/env sh
set -u

export LC_ALL=C
umask 022

proj='eix'

name=${0##*/}

Echo() {
	printf '%s\n' "$*"
}

Echon() {
	printf '%s' "$*"
}

Exec() {
	Echo "$*"
	"$@"
}

Usage() {
	Echo "Usage: $name
Call git-tag VERSION"
	exit ${1:-1}
}

Warn() {
	Echo "$name: warning: $1" >&2
}

Die() {
	Echo "$name: $1" >&2
	exit ${2:-1}
}

ExecDie() {
	Exec "$@" || Die "$* failed" $?
}

command -v git-tag >/dev/null 2>&1 || Die \
'"git-tag" does not exist in $PATH (or is not executable).
"git-tag" should be a wrapper to git which accepts the tag name as the
first argument. In the simplest case, it can be a script with the content

#!/usr/bin/env sh
exec git tag -s "$1" -m "Release $1"

A more luxury variant (with semi-automatic changing between -s and -a) is in
	https://github.com/vaeth/git-wrappers-mv/
(this is dev-vcs/git-wrappers-mv in the mv overlay, available as an overlay)'

[ $# -eq 0 ] || Usage

test -f "contrib/$name" || {
	test -f "$name" && cd ..
}
test -f "contrib/$name" || Die 'must be run from the main directory'

ver=`sed -ne 's/^[[:space:]]*AC_INIT[[:space:]]*([^,]*,[[:space:][]*\([^],[:space:]]*\).*$/\1/p' configure.ac`
meson_ver=`sed -ne 's/^[[:space:]]*version[[:space:]]*:[[:space:]]*["'\'']\([^"'\'']*\).*$/\1/p' meson.build`
[ "$ver" = "$meson_ver" ] || Die "Version $ver and meson version $meson_ver do not match"
v=v$ver

check=`git tag -l "$v"` || Die "git tag -l $v failed"
[ -z "$check" ] || Die "eix $ver is already tagged.
To tag a new release update first the AC_INIT call in configure.ac"

KeyCheck() {
	Echon "$*? (y/n) "
	while :
	do	t=`stty -g`
		stty -icanon -echo
		a=`dd count=1 bs=1 2>/dev/null` || a=
		stty $t
		case $a in
		[nN]*)
			Echo 'No'
			return 1;;
		[yY]*)
			Echo 'Yes'
			return;;
		esac
	done
}

set -- eix*.tar.xz
test -f "$1" || Warn 'No eix*.tar.xz tarball found.
This is suspicious: You should have created one with contrib/tarball.sh'

Echo 'Usage of this script requires that you committed the latest changes.'
! KeyCheck "Execute 'git-tag $v'" || ExecDie git-tag "$v"

if test -f "$1"
then	Echo "
Next (and final) step:"
else	Echo "
The next step is to create the tarball with contrib/tarball.sh
The last step after this:"
fi
Echo "Create signature files for the tarball and for the git tag
e.g. by using git-archive from https://github.com/vaeth/git-wrappers-mv/
Then execute git-push [git push -u origin master]
and upload tarball and signature for the relase"

