#!/usr/bin/env sh
# Check whether the #include directives are reasonable.
# This is only a very heuristic test and does certainly not cover all cases.
# Note that from the tests (after the functions) you can see which #include's
# should be used for which type/function.
#
# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#	Martin V\"ath <vaeth@mathematik.uni-wuerzburg.de>

export LC_ALL='C' GREP_OPTIONS='--color=always'
unset GREP_COLORS GREP_COLOR

name=${0##*/}

Echo() {
	printf '%s\n' "${*}"
}

Die() {
	Echo "${name}: ${*}" >&2
	exit ${1:-1}
}

test -f "contrib/${name}" || {
	test -f "${name}" && cd ..
}
test -f "contrib/${name}" || Die 'must be run from the main directory'

EchoResultPositive() {
	Echo "${*}"
	Echo "${result}"
	echo
}

EchoResultNegative() {
	Echo "${*}" 'none'
}

if [ ${#} -eq 0 ]
then	Echo "${name}: no parameters given: non-verbose"
	echo
	EchoResultNegative() {
	:
}
fi

EchoResult() {
	if [ -n "${result}" ]
	then	EchoResultPositive "${@}"
	else	EchoResultNegative "${@}"
	fi
}

GrepAllSub() {
	find . '(' -name "generate*.sh" -o -name "*.cc" -o -name "*.h" ')' \
		'-!' -name "config.h" -exec grep -l "${@}" -- '{}' '+'
}

GrepAllSubQ() (
	unset GREP_OPTIONS
	GrepAllSub "${@}"
)

GrepAllWith() {
	result=`GrepAllSub "${@}"`
	EchoResult "Files with ${*}:"
}

GrepHWith() {
	result=`find . -name "*.h" -exec grep -l -e "${@}" -- '{}' '+'`
	EchoResult "*.h files with ${*}:"
}

GrepCCWithout() {
	result=`find . '(' -name "generate*.sh" -o -name "*.cc" ')' \
		-exec grep -L -e "${@}" -- '{}' '+'`
	EchoResult "*.cc files without ${*}:"
}

SetG='case ${1} in
':')	shift
	g="using std::${1}[^<]";;
*)	g="include ${1}";;
esac
shift
${colon}'

CheckWithout() {
	local g
	eval "${SetG}" || return 0
	result=`GrepAllSubQ "${@}" | xargs -- grep -L -e "${g}" --`
	EchoResult "Files with ${*} but not ${g}:"
}

CheckWith() {
	local g
	eval "${SetG}"
	result=`GrepAllSubQ -e "${g}" | xargs -- grep -L "${@}" --`
	EchoResult "Files with ${g} but not ${*}:"
}

Check() {
	CheckWithout "${@}"
	CheckWith "${@}"
}

GrepAllWith -e 'ATTRIBUTE_NONNULL_(' -e 'ATTRIBUTE_NONNULL\([^(_]\|$\)' -e 'ATTRIBUTE_NONNULL(\([^(]\|$\)'
GrepHWith 'include .config'
GrepCCWithout '#include <config\.h>'
#Check '<config\.h>' -e '^#if.*HAVE' -e '^#if.*USE' -e '^#if.*WITH' -e '^#if.*[^E]TEST'  -e '^#if.*DEBUG' -e '#if.*ENABLE' -e '[^\"_]EIX_CACHEFILE' -e '#if.*BIGENDIAN' -e '#if.*[^_]TARGET' -e '#if.*CASEFOLD' -e SYSCONFDIR -e ALWAYS_RECALCULATE -e 'ATTRIBUTE_NORETURN' -e 'ATTRIBUTE_SIGNAL' -e 'ATTRIBUTE_CONST' -e 'ATTRIBUTE_PURE' -e '[^a-e_g-z_.]open(' -e '[^a-e_g-z_.]close(' -e 'seek(' -e 'stat(' -e "sys/stat" -e "sys/type" -e "sys/mman" -e "size_t[^y]" -e "off_t" -e "fcntl" -e 'FULL_GCC_DIAG_PRAGMA'
Check '"eixTk/assert\.h"' -e 'eix_assert'
Check '"eixTk/stringtypes\.h"' -e 'WordVec' -e 'WordSet' -e 'WordList' -e 'LineVec' -e 'WordSize'
Check '"eixTk/constexpr\.h"' -e 'CONSTEXPR'
Check '"eixTk/diagnostics\.h"' -e DIAG_OFF -e DIAG_ON
Check '"eixTk/eixint\.h"' -e OffsetType -e UChar -e UNumber -e Treesize -e Catsize -e Versize -e SignedBool -e TinySigned -e TinyUnsigned
Check '"eixTk/exceptions\.h"' -e portage_parse_error
Check '"eixTk/formated\.h"' -e '::format'
Check '"eixTk/i18n\.h"' -e '_('
Check '"eixTk/inttypes\.h"' -e int8 -e int16 -e int32 -e int64
Check '"eixTk/likely\.h"' -e 'likely('
Check '"eixTk/null\.h"' -e 'NULLPTR'
Check '"eixTk/ptr_list\.h"' -e 'eix::ptr'
Check '"eixTk/stringutils\.h"' -e 'split[^- ]' -e isdigit -e '[^a-z]isal[np]' -e isspace -e is_numeric -e to_lower -e trim -e StringHash -e escape_string -e localeC -e match_list -e slot_subslot -e casecontains -e caseequal
Check '"eixTk/unused\.h"' -e '[^_]UNUSED' -e 'ATTRIBUTE_UNUSED'
Check '"portage/basicversion\.h"' -e 'BasicVersion' -e 'BasicPart'

Check '<iostream>' -e '[^a-z]cout[^a-z]' -e '[^a-z]cerr[^a-z]' -e '[^a-z]cin[^a-z]'
Check '<list>' -e '[^_]list<' -e '^list<'
Check '<map>' -e '[^_]map<' -e '^map<'
Check '<set>' -e '[^_]set<' -e '^set<'
Check '<string>' -e '[^_]string[^>".,;a-z ]' -e 'std::string' -e '[^_]string [a-zA-Z_0-9]* *[;=(]' -e '[a-z]<string[,>]' -e 'const string '
Check '<vector>' -e '[^_]vector<' -e '^vector<'
Check '<utility>' -e '[^_]pair<' -e '^pair<'

Check : 'list' -e '[^:_]list<' -e '^list<'
Check : 'map' -e '[^:_i]map<' -e '^map<'
Check : 'pair' -e '[^:_]pair<' -e '^pair<'
Check : 'set' -e '[^:_]set<' -e '^set<'
Check : 'vector' -e '[^:_]vector<' -e '^vector<'
CheckWith : 'string' -e '[^:_]string[^a-zA-Z_0-9]*'

Check : 'cerr' -e '[^:_]cerr[^a-z]'
Check : 'cout' -e '[^:_]cout[^a-z]'
Check : 'endl' -e '[^:_]endl[^a-z]'

Check '<cassert>' -e 'assert('
Check '<cstddef>' -e '[^_N]NULL\([^P]\|$\)'
Check '<cstdio>' -e fopen -e fclose -e fflush -e '[^A-Z_]FILE[^A-Z_]' -e 'printf(' -e fseek
Check '<cstdlib>' -e '[^_a-z]exit[^_]' -e '[^.>]free[^a-z]' -e malloc -e getenv -e strtol -e EXIT_SUCCESS -e EXIT_FAILURE
Check '<cstring>' -e strdup -e strlen -e strndup -e strcmp -e strncmp -e strncpy -e strchr -e strrchr -e strerror -e memset -e memcpy
Check '<csignal>' -e signal -e sigaction
Check '<cerrno>' -e '[^c]errno'
Check '<ctime>' -e time_t
Check '<fcntl\.h>' -e '[^a-z_.]open(' -e '[^a-z_.]close(' -e '[^a-z_.]open (' -e '[^a-z_.]close ('
Check '<unistd\.h>' -e '[^a-z]_exit' -e '[^a-z]exec[lv]' -e setuid -e getuid -e chown -e '[^a-z_.]close(' -e isatty
Check '<sys/types\.h>' -e uid_t -e gid_t -e "size_t[^y]" -e off_t
Check '<sys/stat\.h>' -e fchown -e fchmod -e 'stat[^a-z]'
Check '<sys/mman\.h>' -e mmap


