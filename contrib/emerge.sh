#!/usr/bin/env sh

category='app-portage'
project='eix'

export LC_ALL=C

Echo() {
	printf '%s\n' "${*}"
}

Die() {
	Echo "${0##*/}: ${1}" >&2
	exit ${2:-1}
}

[ "`id -u`" -eq 0 ] || Die "only root may execute this script"
curr='.'

distdir=`portageq distdir`
[ -n "${distdir:++}" ] && test -d "${distdir}" || Die "cannot determine DISTDIR"

GetEdir() {
	eroot=`portageq envvar EROOT`
	for overlay in `portageq get_repos "${eroot}"`
	do	edir=`portageq get_repo_path "${eroot}" "${overlay}"`/${category}/${project}
		test -d "${edir}" || continue
		for jebuild in "${edir}/${project}-"*.ebuild
		do	case ${jebuild##*/} in
			*9999.ebuild)
				continue;;
			esac
			test -f "${jebuild}" || continue
			repo_name=${overlay}
			return 0
		done
	done
	return 1
}
GetEdir || Die "cannot find ${category}/${project} in overlay"

MoveEbuild() {
	test -f "${1}" && return
	latest=
	for find_latest in "${edir}/${project}-"*.ebuild
	do	case ${find_latest##*/} in
		*9999.ebuild)
			continue;;
		esac
		test -f "${find_latest}" && latest=${find_latest}
	done
	[ -n "${latest:++}" ] || return
	Echo "Moving ${latest} -> ${1##*/}"
	mv -- "${latest}" "${1}" || Die "cannot mv ${latest} ${1}"
}

RmOpt() {
	! test -f "${1}" || rm -- "${1}"
}

InstallEbuildVersion() {
	chown portage:portage -- "${1}"
	chmod 664 -- "${1}"
	Echo "Moving ${1} -> ${distdir}"
	mv -- "${1}" "${distdir}/${1##*/}"
	MoveEbuild "${edir}/${project}-${2}.ebuild" || \
		Die "failed to rename ebuild in ${edir}"
	(	cd -- "${edir}" || Die "cannot change to ${edir}"
		RmOpt "Manifest" || Die "cannot remove Manifest"
		Echo "ebuild ${edir}/${project}-${2}.ebuild manifest"
		ebuild "${project}-${2}.ebuild" manifest || Die "ebuild failed"
	) || exit
	if [ -n "${repo_name:++}" ] && command -v egencache >/dev/null 2>&1
	then	Echo "egencache --repo=${repo_name} --update"
		egencache --repo="${repo_name}" --update || \
			Die "egencache failed"
	fi
	Echo "emerge -1O =${category}/${project}-${2}"
	exec emerge -1O "=${category}/${project}-${2}"
	Die "cannot execute emerge"
}

for j in tar.xz tar.bz2 tar.gz zip
do	for i in "${curr}/${project}-"*."${j}"
	do	test -e "${i}" || continue
		v=${i##*/}
		v=${v#"${project}-"}
		v=${v%".${j}"}
		InstallEbuildVersion "${i}" "${v}"
		exit
	done
done
Die "No tarball found in ${curr}"
