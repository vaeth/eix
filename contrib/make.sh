#!/usr/bin/env sh
set -u

LC_ALL=C
export LC_ALL

Echo() {
	printf '%s\n' "$*"
}

Usage() {
	Echo "Usage: ${0##*/} [options] args-for-make
Available options are
  -q  quiet
  -n  Stop after ./configure, i.e. do not run make
  -e  Keep environment - do not modify LDFLAGS, CXXFLAGS, CFLAGS, CC
  -1/2/3 Generate 1/2/3 binaries for eix-{,diff,update} but no separate tools
      (unless combined with -4)
  -4  Enable separate tools but no separate binaries (unless combined with 2/3)
  -w  Use -werror
  -W  Do not use --enable-strong-warnings
  -g  Use clang++, setting CXX, filtering some flags (default if available)
  -G  Use default CXX (mnemonic: GNU)
  -s  With sqlite
  -S  Without sqlite
  -C  Avoid CCACHE
  -x  Recache (i.e. ignore broken ccache)
  -X  Clear CCACHE
  -y  Use new c++ dialect
  -Y  Do not use new c++ dialect
  -jX Use -jX (currently $jarg)
  -J Use --disable-jumbo-build
  -c OPT Add OPT to ./configure
  -M OPT Add OPT to meson
  -m  Use meson instead of autotools
  -d  Use --enable-debugging -Ddebugging=true and no --buildtype=release
  -O  As -c --enable-strong-optimization -c --enable-security (and -M ...)
  -o  Do not pass --enable-nopie-security
  -R  Remove builddir (for meson)
  -r  Change also directory permissions to root (for fakeroot-ng)"
	exit ${1:-1}
}

Info() {
	$quiet || printf '\033[01;32m%s\033[0m\n' "$*"
}

InfoVerbose() {
	$quiet || printf '\n\033[01;33m%s\033[0m\n\n' "$*"
}

Die() {
	Echo "${0##*/}: error: $1" >&2
	exit ${2:-1}
}

SetCcache() {
	dircc='/usr/lib/ccache/bin'
	if $use_ccache && case ":$PATH:" in
	*":$dircc:"*)
		false;;
	esac
	then	Info "export PATH=$dircc:\$PATH"
		PATH=$dircc:$PATH
		export PATH
	fi
	if $recache
	then	Info "export CCACHE_RECACHE=true"
		CCACHE_RECACHE=true
		export CCACHE_RECACHE
	fi
	CCACHE_SLOPPINESS='file_macro,time_macros,include_file_mtime'
	Info "export CCACHE_SLOPPINESS='$CCACHE_SLOPPINESS'"
	export CCACHE_SLOPPINESS
	testcc=
	for dircc in \
		"$HOME/.ccache" \
		../.ccache \
		../ccache \
		../../.ccache \
		../../ccache
	do	[ -z "${dircc:++}" ] || ! test -d "$dircc" && continue
		testcc=`cd -P -- "$dircc" >/dev/null 2>&1 && \
				printf '%sA' "$PWD"` && \
			testcc=${testcc%A} && [ -n "${testcc:++}" ] \
			&& test -d "$testcc" && break
	done
	if [ -n "${testcc:++}" ]
	then	Info "export CCACHE_DIR=$testcc"
		Info 'export CCACHE_COMPRESS=true'
		CCACHE_DIR=$testcc
		CCACHE_COMPRESS=true
		export CCACHE_DIR CCACHE_COMPRESS
	fi
	$clear_ccache || return 0
	Info 'ccache -C'
	ccache -C
}

clang_cxx=`PATH=${PATH-}${PATH:+:}/usr/lib/llvm/*/bin command -v clang++ 2>/dev/null` \
  && [ -n "${clang_cxx:++}" ] && clang=: || clang=false

meson=false
remove_builddir=false
quiet=false
earlystop=false
keepenv=false
warnings=:
werror=false
separate_all=:
use_chown=false
jarg='-j3'
use_ccache=:
automake_extra=
meson_extra='--prefix=/usr --sysconfdir=/etc'
configure_extra=$meson_extra
optimization=false
recache=false
clear_ccache=false
debugging=false
dialect=:
nopie_security=:
OPTIND=1
while getopts 'mJRq1234gGnewWsSrOoCxXyYdM:c:j:hH' opt
do	case $opt in
	m)	meson=:;;
	J)	meson_extra=$meson_extra' -Djumbo-build=false'
		configure_extra=$configure_extra' --disable-jumbo-build';;
	R)	remove_builddir=:;;
	q)	quiet=:;;
	1)	separate_all=false;;
	2)	separate_all=false
		meson_extra=$meson_extra' -Dseparate-update=true'
		configure_extra=$configure_extra' --enable-separate-update';;
	3)	separate_all=false
		meson_extra=$meson_extra' -Dseparate-binaries=true'
		configure_extra=$configure_extra' --enable-separate-binaries';;
	4)	separate_all=false
		meson_extra=$meson_extra' -Dseparate-tools=true'
		configure_extra=$configure_extra' --enable-separate-tools';;
	g)	clang=:;;
	G)	clang=false;;
	n)	earlystop=:;;
	e)	keepenv=:;;
	w)	werror=:;;
	W)	warnings=false;;
	s)	meson_extra=$meson_extra' -Dsqlite=true'
		configure_extra=$configure_extra' --with-sqlite';;
	S)	meson_extra=$meson_extra' -Dsqlite=false'
		configure_extra=$configure_extra' --without-sqlite';;
	r)	use_chown=:;;
	O)	optimization=:;;
	o)	nopie_security=false;;
	C)	use_ccache=false;;
	x)	recache=:;;
	X)	clear_ccache=:;;
	y)	dialect=:;;
	Y)	dialect=false;;
	d)	debugging=:;;
	M)	meson_extra=$meson_extra" $OPTARG";;
	c)	configure_extra=$configure_extra" $OPTARG";;
	j)	[ -n "${OPTARG:++}" ] && jarg='-j'$OPTARG || jarg=;;
	'?')	exit 1;;
	*)	Usage 0;;
	esac
done
if [ $OPTIND -gt 1 ]
then	( eval '[ "$(( 0 + 1 ))" = 1 ]' ) >/dev/null 2>&1 && \
	eval 'shift "$(( $OPTIND - 1 ))"' || shift "`expr $OPTIND - 1`"
fi

SetCcache
if $dialect
then	meson_extra=$meson_extra' -Dnew-dialect=true'
	configure_extra=$configure_extra' --enable-new-dialect'
else	meson_extra=$meson_extra' -Dnew-dialect=false'
	configure_extra=$configure_extra' --disable-new-dialect'
fi
if $separate_all
then	meson_extra=$meson_extra' -Dseparate-binaries=true -Dseparate-tools=true'
	configure_extra=$configure_extra' --enable-separate-binaries --enable-separate-tools'
fi
if $optimization
then	meson_extra=$meson_extra' -Dstrong-optimization=true -Dsecurity=true'
	configure_extra=$configure_extra' --enable-strong-optimization --enable-security'
fi
if $nopie_security
then	meson_extra=$meson_extra' -Dnopie-security=true'
	configure_extra=$configure_extra' --enable-nopie-security'
fi
if $debugging
then	meson_extra=$meson_extra' -Ddebugging=true'
	configure_extra=$configure_extra' --enable-debugging'
else	meson_extra=$meson_extra' --buildtype=release'
fi
if $quiet
then	quietredirect='>/dev/null'
else	quietredirect=
fi

if $use_chown
then	ls /root >/dev/null 2>&1 && \
		Die 'you should not really be root when you use -r' 2
	chown -R root:root .
fi

FirstMatchesFilter() {
	firstmatches=$1
	shift
	for filterpattern
	do	case $firstmatches in
		$filterpattern)
			return 0;;
		esac
	done
	return 1
}

Filter() {
	for currvar in \
		CFLAGS \
		CXXFLAGS \
		LDFLAGS \
		CPPFLAGS
	do	eval oldflags=\$$currvar
		newflags=
		for currflag in $oldflags
		do	FirstMatchesFilter "$currflag" "$@" || \
				newflags=$newflags${newflags:+ }$currflag
		done
		eval $currvar=\$newflags
	done
}

FilterClang() {
	Filter \
		'-f*enforce-eh-specs' \
		'-f*ident' \
		'-f*semantic-interposition' \
		'-fdevirtualize-speculatively' \
		'-fdirectives*' \
		'-fgcse*' \
		'-fgraphite*' \
		'-finline-functions' \
		'-fipa-pta' \
		'-fira-loop-pressure' \
		'-fisolate-erroneous-paths-attribute' \
		'-fivopts' \
		'-floop*' \
		'-flto-*' \
		'-fmodulo*' \
		'-fnothrow-opt' \
		'-fpredictive-commoning' \
		'-frename-registers' \
		'-freorder-functions' \
		'-frerun-cse-after-loop' \
		'-fsched*' \
		'-fsection-anchors' \
		'-ftree*' \
		'-funsafe-loop*' \
		'-fuse-linker-plugin' \
		'-fvect-cost-model' \
		'-fweb' \
		'-fwhole-program' \
		'-mvectorize*'
}

if ! $keepenv
then	unset CFLAGS CXXFLAGS LDFLAGS CPPFLAGS CXX
	CFLAGS=`portageq envvar CFLAGS`
	CXXFLAGS=`portageq envvar CXXFLAGS`
	LDFLAGS=`portageq envvar LDFLAGS`
	CPPFLAGS=`portageq envvar CPPFLAGS`
	CXX=`portageq envvar CXX`
	export CFLAGS CXXFLAGS LDFLAGS CPPFLAGS
	if $clang
	then	CXX=$clang_cxx
		FilterClang
	fi
	[ -z "$CXX" ] || export CXX
	if $warnings
	then	meson_extra=$meson_extra' -Dstrong-warnings=true'
		configure_extra=$configure_extra' --enable-strong-warnings'
		automake_extra=$automake_extra' -Wall'
	fi
	if $werror
	then	CXXFLAGS=$CXXFLAGS' -Werror'
		automake_extra=$automake_extra' -Werror'
	fi
fi
[ -z "${CXXFLAGS-}" ] || Info "export CXXFLAGS='${CXXFLAGS}'"
[ -z "${LDFLAGS-}" ] || Info "export LDFLAGS='${LDFLAGS}'"
[ -z "${CPPFLAGS-}" ] || Info "export CPPFLAGS='${CPPFLAGS}'"
[ -z "${CXX-}" ] || Info "export CXX='${CXX}'"
if $remove_builddir && test -d builddir
then	InfoVerbose rm -rf builddir
	rm -rf builddir
fi
if $meson
then	if ! test -d builddir
	then	InfoVerbose meson $meson_extra builddir
		unset LC_ALL
		eval "meson \$meson_extra builddir $quietredirect" || Die 'meson failed'
	fi
	$earlystop && exit
	InfoVerbose ninja -C builddir $jarg
	if $quiet
	then	exec ninja -C builddir $jarg ${1+"$@"} >/dev/null
	else	exec ninja -C builddir $jarg ${1+"$@"}
	fi
	exit
fi
if ! test -e Makefile
then	if ! test -e configure || ! test -e Makefile.in
	then	InfoVerbose './autogen.sh' $automake_extra
		eval "./autogen.sh $automake_extra $quietredirect" \
			|| Die 'autogen failed'
	fi
	InfoVerbose './configure' $configure_extra
	eval "./configure $configure_extra $quietredirect" || \
		Die 'configure failed'
fi
$earlystop && {
	InfoVerbose 'make' $jarg config.h
	exec make $jarg config.h
	Die 'cannot exec make'
}
InfoVerbose 'make' $jarg $*
command -v make >/dev/null 2>&1 || Die 'cannot find make'
if $quiet
then	exec make $jarg ${1+"$@"} >/dev/null
else	exec make $jarg ${1+"$@"}
fi
