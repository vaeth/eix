#!/usr/bin/env sh

export LC_ALL=C
umask 022

proj='eix'

name=${0##*/}

Echo() {
	printf '%s\n' "${*}"
}

Echon() {
	printf '%s' "${*}"
}

Eecho() {
	Echo "# ${*}"
}

Exec() {
	Eecho "${*}"
	"${@}"
}

Usage() {
	Echo "Usage: ${name}
Marks a new release in the git repository.
Verify in advance to have AC_INIT set correctly in configure.ac
and that you have committed your last changes."
	exit ${1:-1}
}

Die() {
	Echo "${name}: ${1}" >&2
	exit ${2:-1}
}

Warn() {
	wret=${?}
	Echo "${name}: warning: ${1}" >&2
	return ${wret}
}

ExecWarn() {
	Exec "${@}" || Warn "${*} failed: ${?}"
}

ExecDie() {
	Exec "${@}" || Die "${*} failed" ${?}
}

[ ${#} -eq 0 ] || Usage

test -f "contrib/${name}" || {
	test -f "${name}" && cd ..
}
test -f "contrib/${name}" || Die 'must be run from the main directory'

ver=`sed -ne 's/^[[:space:]]*AC_INIT[[:space:]]*([^,]*,[[:space:][]*\([^],[:space:]]*\).*$/\1/p' configure.ac`

KeyCheck() {
	Echon "${*}? (y/n) "
	while :
	do	t=`stty -g`
		stty -icanon -echo
		a=`dd count=1 bs=1 2>/dev/null` || a=
		stty ${t}
		case ${a} in
		[nN]*)	Echo 'No'
			return 1
			;;
		[yY]*)	Echo 'Yes'
			return
			;;
		esac
	done
}

Echo 'Usage of this script requires that you committed the latest changes'
if KeyCheck "Are you sure you want to tag v${ver} of ${proj}"
then	Eecho "git tag -a v${ver} -m ..."
#svnroot="https://svn.gentooexperimental.org/${proj}"
#svn copy "${svnroot}/trunk" "${svnroot}/tags/v${ver}"
	git tag -a "v${ver}" \
		-m "Tagging the ${ver} release" || \
		KeyCheck "git tagging failed. Continue anyway" || exit 2
fi

KeyCheck '
Optionally, you can keep a tarball branch with the current tarball content.
This is only reasonable if the only reasonable way to publish the tarball is
its automatic generation directly from the git repository.
Otherwise, this would be just a waste of space.

To use this option, you must have created the tarball first.

Have you done this, and do you want to update the tarball branch' || exit 0
for j in tar.xz tar.bz2 tar.gz zip
do	tarball="${proj}-${ver}.${j}"
	test -f "${tarball}" && break
done
test -f "${tarball}" || \
	Die "You must first create the tarball with contrib/tarball.sh"

# The rest must be a function, since the original file
# will be removed during execution, and so parsing of this file might fail

TagTarball() {
	ExecDie git checkout -B tarball
	ExecDie tar xaf "${tarball}"

	Echo "Replacing content by unpacked tarball"
	for i in *
	do	[ "${i}" = "${proj}-${ver}" ] || rm -rf "./${i}"
	done
	Eecho "mv ${proj}-${ver}/* ."
	mv "${proj}-${ver}"/* . || Die "mv ${proj}-${ver}/* . failed"
	Eecho "mv ${proj}-${ver}/.* ."
	mv "${proj}-${ver}"/.* . || Warn "mv ${proj}-${ver}/.* . failed"
	ExecDie rmdir "${proj}-${ver}"

	ExecWarn git merge master
	ExecWarn git add .
	Eecho "git commit -a -m ..."
	git commit -a -m "tarball ${proj}-${ver} preparation" || \
		KeyCheck "This is only ok if there is nothing to commit.
Otherwise you should better stop now and call git checkout master.
Are you sure you want to tag tarball ${proj}-${ver} anyway" || exit 1
	Eecho "git tag -a tarball-${ver} -m ..."
	git tag -a "tarball-${ver}" \
		-m "Tagging the ${ver} tarball of the ${proj} project" || \
		Warn "git tagging failed"
	ExecWarn git checkout master
	exit
}

TagTarball
