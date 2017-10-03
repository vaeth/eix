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
#	Martin V\"ath <martin@mvath.de>
set -u

LC_ALL=C
export LC_ALL
unset GREP_COLORS GREP_COLOR GREP_OPTIONS
grep_plain=grep
grep_cmd="$grep_plain --color=always"
name=`echo 1 | $grep_cmd 1 1>/dev/null` && [ -z "$name" ] \
	|| grep_cmd=$grep_plain

name=${0##*/}

Echo() {
	printf '%s\n' "$*"
}

Die() {
	Echo "$name: $*" >&2
	exit ${1:-1}
}

test -f "contrib/$name" || {
	test -f "$name" && cd ..
}
test -f "contrib/$name" || Die 'must be run from the main directory'

EchoResultPositive() {
	Echo "$*"
	Echo "$result"
	echo
}

EchoResultNegative() {
	Echo "$*" 'none'
}

if [ $# -ne 0 ]
then	Echo "$name: Parameter specified: verbose output follows
"
else	EchoResultNegative() {
	:
}
fi

EchoResult() {
	if [ -n "$result" ]
	then	EchoResultPositive ${1+"$@"}
	else	EchoResultNegative ${1+"$@"}
	fi
}

GrepAllSub() {
	find . '(' -name "generate*.sh" -o -name "*.cc" -o -name "*.h" ')' \
		'-!' -name "config.h" -exec $grep_cmd -l ${1+"$@"} -- '{}' '+'
}

GrepAllSubQ() (
	grep_cmd=$grep_plain
	GrepAllSub ${1+"$@"}
)

GrepAllWith() {
	result=`GrepAllSub ${1+"$@"}`
	EchoResult "Files with $*:"
}

GrepHWith() {
	result=`find . -name "*.h" -exec $grep_cmd -l -e ${1+"$@"} -- '{}' '+'`
	EchoResult "*.h files with $*:"
}

GrepHWithout() {
	result=`find . -name "*.h" -exec $grep_cmd -L -e ${1+"$@"} -- '{}' '+'`
	EchoResult "*.h files with $*:"
}

GrepCCWith() {
	result=`find . '(' -name "generate*.sh" -o -name "*.cc" ')' \
		-exec $grep_cmd -l -e ${1+"$@"} -- '{}' '+'`
	EchoResult "*.cc files without $*:"
}

GrepCCWithout() {
	result=`find . '(' -name "generate*.sh" -o -name "*.cc" ')' \
		-exec $grep_cmd -L -e ${1+"$@"} -- '{}' '+'`
	EchoResult "*.cc files without $*:"
}

SetG='case $1 in
":")	shift
	g="using std::$1[^<]";;
*)	g="include $1";;
esac
shift'

CheckWithout() {
	eval "$SetG"
	result=`GrepAllSubQ ${1+"$@"} | xargs -- $grep_cmd -L -e "$g" --`
	EchoResult "Files with $* but not $g:"
}

CheckWith() {
	eval "$SetG"
	result=`GrepAllSubQ -e "$g" | xargs -- $grep_cmd -L ${1+"$@"} --`
	EchoResult "Files with $g but not $*:"
}

Check() {
	CheckWithout ${1+"$@"}
	CheckWith ${1+"$@"}
}

GrepAllWith -e 'ATTRIBUTE_NONNULL_(' -e '^ATTRIBUTE_NONNULL\([^(_]\|$\)' -e '[^_]ATTRIBUTE_NONNULL\([^(_]\|$\)' -e 'ATTRIBUTE_NONNULL([^(a]'
#GrepHWith 'include .config'
GrepHWithout '#include <config\.h>'
GrepCCWithout '#include <config\.h>'
Check '"eixTk/assert\.h"' -e 'eix_assert'
Check '"eixTk/stringtypes\.h"' -e 'WordVec' -e 'WordSet' -e 'WordUnorderedSet' -e 'WordUnorderedMap' -e 'WordIterateSet' -e 'WordIterateMap' -e 'WordList' -e 'LineVec' -e 'WordSize'
Check '"eixTk/dialect\.h"' -e 'CONSTEXPR' -e 'ASSIGN_DELETE' -e 'OVERRIDE[^_A-Z]' -e 'FINAL' -e 'NOEXCEPT' -e 'EMPLACE_BACK' -e 'PUSH_BACK' -e 'INSERT' -e 'PUSH' -e 'MOVE'
Check '"eixTk/diagnostics\.h"' -e DIAG_OFF -e DIAG_ON
Check '"eixTk/eixarray\.h"' -e 'eix::array'
Check '"eixTk/eixint\.h"' -e OffsetType -e UChar -e UNumber -e Treesize -e Catsize -e Versize -e SignedBool -e TinySigned -e TinyUnsigned
Check '"eixTk/formated\.h"' -e 'eix::format' -e 'eix::print' -e 'eix::say'
Check '"eixTk/i18n\.h"' -e '_('
Check '"eixTk/inttypes\.h"' -e int8 -e int16 -e int32 -e int64
Check '"eixTk/likely\.h"' -e 'likely('
Check '"eixTk/null\.h"' -e 'NULLPTR'
Check '"eixTk/auto_array\.h"' -e 'auto_array'
Check '"eixTk/forward_list\.h"' -e 'forward_list'
Check '"eixTk/iterate_map\.h"' -e 'ITERATE_MAP'
Check '"eixTk/iterate_set\.h"' -e 'ITERATE_SET'
Check '"eixTk/unordered_map\.h"' -e 'UNORDERED_MAP'
Check '"eixTk/unordered_set\.h"' -e 'UNORDERED_SET'
Check '"eixTk/ptr_container\.h"' -e 'ptr_container' -e 'ptr_forward_container'
Check '"eixTk/stringutils\.h"' -e 'split[^- ]' -e isdigit -e '[^a-z]isal[np]' -e isspace -e isdigit -e isalpha -e isalnum -e is_numeric -e tolower -e toupper -e to_lower -e trim -e StringHash -e escape_string -e localeC -e match_list -e slot_subslot -e casecontains -e caseequal -e my_atou -e my_atos
Check '"eixTk/attribute\.h"' -e 'ATTRIBUTE_'
Check '"portage/basicversion\.h"' -e 'BasicVersion' -e 'BasicPart'
CheckWithout '"search/packagetest\.h"' -e 'PackageTest::'

Check '<iostream>' -e std::cout -e std::cerr -e std::cin
Check '<list>' -e '[^_]list<' -e '^list<'
Check '<map>' -e '[^_]map<' -e '^map<'
Check '<set>' -e '[^_]set<' -e '^set<'
Check '<string>' -e '[^_]string[^>".,;a-z ]' -e 'std::string' -e '[^_]string [a-zA-Z_0-9]* *[;=(]' -e '[a-z]<string[,>]' -e 'const string '
Check '<vector>' -e '[^_]vector<' -e '^vector<'
Check '<utility>' -e '[^_]pair<' -e '^pair<'

#Check : 'list' -e '[^:_]list<' -e '^list<'
Check : 'map' -e '[^:_i]map<' -e '^map<'
Check : 'pair' -e '[^:_]pair<' -e '^pair<'
Check : 'set' -e '[^:_]set<' -e '^set<'
Check : 'vector' -e '[^:_]vector<' -e '^vector<'
CheckWith : 'string' -e '[^:_]string[^a-zA-Z_0-9]*'

#Check : 'cerr' -e '[^:_]cerr[^a-z]'
#Check : 'cout' -e '[^:_]cout[^a-z]'
#Check : 'endl' -e '[^:_]endl[^a-z]'
GrepAllWith -e '[^:_]cerr[^a-z]' -e '[^:_]cout[^a-z]' -e '[^:_]endl[^a-z]'
GrepAllWith -e '[^:]fopen' -e '[^:]fclose' -e '[^:]fflush' -e '[^:"]fseek[^o]' -e '[^:]ftell[^o]' -e '[^:]fread' -e '[^:]fwrite' -e '[^:fn]printf(' -e '[^:]fprintf(' -e '[^:]snprintf(' -e '[^:f]puts(' -e '[^:]fputs(' -e '[^:f]putc(' -e '[^:]fputc(' -e '[^:f]getc(' -e '[^:]fgetc('
GrepAllWith -e '[^:_a-z]exit(' -e '[^:.>a-z_]free(' -e '[^:]malloc' -e '[^:]getenv' -e '[^:]strtoull' -e '[^:]strtoul' -e '[^:]strtoll' -e '[^:]strtol' -e '[^:_]atoi'
GrepAllWith -e '[^:]strlen' -e '[^:]strcmp' -e '[^:]strncmp' -e '[^:]strchr' -e '[^:]strrchr' -e '[^:]strerror' -e '[^:]memset'
GrepAllWith -e '[^:]time_t'
GrepAllWith -e '[^:]signal('
GrepAllWith -e '[^:]strerror'

Check '<cassert>' -e 'assert('
Check '<cstddef>' -e '[^_N]NULL\([^P]\|$\)'
Check '<cstdio>' -e std::fopen -e std::fclose -e std::fflush -e std::fseek -e fseeko -e std::ftell -e ftello -e std::fread -e std::fwrite -e std::printf  -e std::fprintf  -e std::snprintf -e std::puts -e std::fputs -e std::putc -e std::fputc -e std::getc -e std::fgetc -e flock -e fileno -e '[^A-Z_]FILE[^A-Z_]'
Check '<cstdlib>' -e std::exit -e std::free -e std::malloc -e std::getenv -e std::strtoull -e std::strtoul -e std::strtoll -e std::strtol -e std::atoi -e EXIT_SUCCESS -e EXIT_FAILURE -e mkstemp
Check '<cstring>' -e std::strlen -e std::strcmp -e std::strncmp -e strcasecmp -e strncasecmp -e std::strchr -e std::strrchr -e std::strerror -e std::memset
Check '<csignal>' -e std::signal -e sigaction
Check '<cerrno>' -e '[^c]errno' -e std::strerror
Check '<ctime>' -e std::time_t
Check '<fcntl\.h>' -e '[^a-z_.]open(' -e '[^a-z_.]close(' -e '[^a-z_.]open (' -e '[^a-z_.]close ('
Check '<unistd\.h>' -e '[^a-z]_exit' -e '[^a-z]exec[lv]' -e setuid -e getuid -e chown -e '[^a-z_.]close(' -e isatty
Check '<sys/types\.h>' -e uid_t -e gid_t -e "size_t[^y]" -e off_t
Check '<sys/stat\.h>' -e fchown -e fchmod -e 'stat[^a-z]'
Check '<sys/mman\.h>' -e mmap
