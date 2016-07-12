#!/usr/bin/env sh
set -u

export LC_ALL=C

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
  -d  Do not use dep-default, required-use-default
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
  -c OPT Add OPT to ./configure
  -d  As -c --enable-debugging
  -O  As -c --enable-strong-optimization -c --enable-security
  -o  Do not pass --enable-nopie-security
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
		export CCACHE_RECACHE=true
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
		export CCACHE_DIR
		export CCACHE_COMPRESS=true
	fi
	$clear_ccache || return 0
	Info 'ccache -C'
	ccache -C
}

quiet=false
dep_default=:
earlystop=false
keepenv=false
warnings=:
werror=false
separate_all=:
use_chown=false
jarg='-j3'
use_ccache=:
automake_extra=
configure_extra='--prefix=/usr --sysconfdir=/etc'
optimization=false
recache=false
clear_ccache=false
debugging=false
command -v clang++ >/dev/null 2>&1 && clang=: || clang=false
dialect='enable'
nopie_security=:
OPTIND=1
while getopts 'q1234gGdnewWsSrOoCxXyYdc:j:hH?' opt
do	case $opt in
	q)	quiet=:;;
	1)	separate_all=false;;
	2)	separate_all=false
		configure_extra=$configure_extra' --enable-separate-update';;
	3)	separate_all=false
		configure_extra=$configure_extra' --enable-separate-binaries';;
	4)	separate_all=false
		configure_extra=$configure_extra' --enable-separate-tools';;
	g)	clang=:;;
	G)	clang=false;;
	d)	dep_default=false;;
	n)	earlystop=:;;
	e)	keepenv=:;;
	w)	werror=:;;
	W)	warnings=false;;
	s)	configure_extra=$configure_extra' --with-sqlite';;
	S)	configure_extra=$configure_extra' --without-sqlite';;
	r)	use_chown=:;;
	O)	optimization=:;;
	o)	nopie_security=false;;
	C)	use_ccache=false;;
	x)	recache=:;;
	X)	clear_ccache=:;;
	y)	dialect='enable';;
	Y)	dialect='disable';;
	d)	debugging=:;;
	c)	configure_extra=$configure_extra" $OPTARG";;
	j)	[ -n "${OPTARG:++}" ] && jarg='-j'$OPTARG || jarg=;;
	*)	Usage 0;;
	esac
done
if [ $OPTIND -gt 1 ]
then	( eval '[ "$(( 0 + 1 ))" = 1 ]' ) >/dev/null 2>&1 && \
	eval 'shift "$(( $OPTIND - 1 ))"' || shift "`expr $OPTIND - 1`"
fi

SetCcache
[ -n "${dialect:++}" ] && configure_extra=$configure_extra" --$dialect-new-dialect"
$dep_default && configure_extra=$configure_extra' --with-dep-default --with-required-use-default'
$separate_all && configure_extra=$configure_extra' --enable-separate-binaries --enable-separate-tools'
$optimization && configure_extra=$configure_extra' --enable-strong-optimization --enable-security'
$nopie_security && configure_extra=$configure_extra' --enable-nopie-security'
$debugging && configure_extra=$configure_extra' --enable-debugging'

$quiet && quietredirect='>/dev/null' || quietredirect=

if $use_chown
then	ls /root >/dev/null 2>&1 && \
		Die 'you should not really be root when you use -r' 2
	chown -R root:root .
fi

Filter() {
	for currvar in \
		CFLAGS \
		CXXFLAGS \
		LDFLAGS \
		CPPFLAGS
	do	eval oldflags=\$$currvar
		newflags=
		for currflag in $oldflags
		do	case " $* " in
			*" $currflag "*)	continue;;
			esac
			newflags=$newflags${newflags:+ }$currflag
		done
		eval $currvar=\$newflags
	done
}

FilterClang() {
	Filter \
		-fnothrow-opt \
		-frename-registers \
		-funsafe-loop-optimizations \
		-fgcse-sm \
		-fgcse-las \
		-fgcse-after-reload \
		-fpredictive-commoning \
		-ftree-switch-conversion \
		-fno-ident \
		-flto-partition=none \
		-ftree-vectorize \
		-fweb \
		-fgraphite \
		-fgraphite-identity \
		-floop-interchange \
		-floop-strip-mine \
		-floop-block \
		-fno-enforce-eh-specs \
		-fdirectives-only \
		-ftracer
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
	then	CXX='clang++'
		FilterClang
	fi
	[ -z "$CXX" ] || export CXX
	if $warnings
	then	configure_extra=$configure_extra' --enable-strong-warnings'
		automake_extra=$automake_extra' -Wall'
	fi
	if $werror
	then	CXXFLAGS=$CXXFLAGS' -Werror'
		automake_extra=$automake_extra' -Werror'
	fi
fi
[ -z "${CXXFLAGS-}" ] ||Info "export CXXFLAGS='${CXXFLAGS}'"
[ -z "${LDFLAGS-}" ] ||Info "export LDFLAGS='${LDFLAGS}'"
[ -z "${CPPFLAGS-}" ] ||Info "export CPPFLAGS='${CPPFLAGS}'"
[ -z "${CXX-}" ] ||  Info "export CXX='${CXX}'"
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
