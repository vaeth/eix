#! /usr/bin/env sh
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

export LC_ALL="C"
unset GREP_OPTIONS GREP_COLORS GREP_COLOR

GrepAll() {
	find . '(' -name "generate*.sh" -o -name "*.cc" -o -name "*.h" ')' \
		-exec grep -l "${@}" -- '{}' '+'
}

GrepWithout() {
	local g ga
	g="include <${1}>"
	ga="include \"${1#*/}\""
	shift
	printf "%s\n" "Files with ${*} but not ${g}:"
	GrepAll "${@}" | GREP_OPTIONS='--color=always' \
		xargs -- grep -L -e "${g}" -e "${ga}" --
	echo
}

GrepWith() {
	local g
	g="include <${1}>"
	ga="include \"${1#*/}\""
	shift
	printf "%s\n" "Files with ${g} but not ${*}:"
	GrepAll -e "${g}" -e "${ga}" | GREP_OPTIONS='--color=always' \
		xargs -- grep -L "${@}" --
	echo
}

if [ "${1}" = "1" ]
then
Check() {
	GrepWithout "${@}"
}
elif [ "${1}" = "2" ]
then
Check() {
	GrepWith "${@}"
}
else
Check() {
	GrepWithout "${@}"
	GrepWith "${@}"
}
fi

Check 'config\.h' -e '^#if.*HAVE' -e '^#if.*USE' -e '^#if.*WITH' -e '^#if.*[^E]TEST'  -e '^#if.*DEBUG' -e '#if.*ENABLE' -e '[^\"_]EIX_CACHEFILE' -e '#if.*BIGENDIAN' -e '#if.*[^_]TARGET' -e '#if.*CASEFOLD' -e SYSCONFDIR -e ALWAYS_RECALCULATE -e 'ATTRIBUTE_NORETURN' -e 'ATTRIBUTE_SIGNAL' -e 'ATTRIBUTE_CONST' -e 'ATTRIBUTE_PURE' -e '[^a-e_g-z_.]open(' -e '[^a-e_g-z_.]close(' -e 'seek(' -e 'stat(' -e "sys/stat" -e "sys/type" -e "sys/mman" -e "size_t[^y]" -e "off_t" -e "fcntl"
Check 'database/types\.h' -e OffsetType -e UChar -e UNumber -e Treesize -e Catsize -e Versize
Check 'eixTk/exceptions\.h' -e portage_parse_error
Check 'eixTk/formated\.h' -e '::format'
Check 'eixTk/i18n\.h' -e '_('
Check 'eixTk/likely\.h' -e 'likely('
Check 'eixTk/null\.h' -e 'NULLPTR'
Check 'eixTk/ptr_list\.h' -e 'eix::ptr'
Check 'eixTk/stringutils\.h' -e 'split[^- ]' -e isdigit -e '[^a-z]isal[np]' -e isspace -e emptystring -e '[^a-zA-Z]one[^a-z.)\ ]' -e is_numeric -e tolower -e trim -e StringHash -e escape_string
Check 'eixTk/unused\.h' -e '[^_]UNUSED' -e 'ATTRIBUTE_UNUSED'

Check 'iostream' -e '[^a-z]cout[^a-z]' -e '[^a-z]cerr[^a-z]' -e '[^a-z]cin[^a-z]'
Check 'map' -e '[^_]map<' -e '^map<'
Check 'set' -e '[^_]set<'
Check 'string' -e '[^_]string[^>".,;a-z ]' -e 'std::string' -e '[^_]string [a-zA-Z_0-9]* *[;=(]' -e '[a-z]<string[,>]' -e 'const string '
Check 'vector' -e '[^_]vector<'

Check 'cstddef' -e '[^_]NULL\([^P]\|$\)'
Check 'cstdio' -e fopen -e fclose -e fflush -e '[^A-Z_]FILE[^A-Z_]' -e 'printf(' -e fseek
Check 'cstdlib' -e '[^_a-z]exit[^_]' -e '[^.>]free[^a-z]' -e malloc -e getenv -e strtol
Check 'cstring' -e strdup -e strlen -e strndup -e strcmp -e strncmp -e strncpy -e strchr -e strrchr -e strcase -e strerror -e memset -e memcpy
Check 'csignal' -e signal -e sigaction
Check 'ctime' -e time_t
Check 'fcntl\.h' -e '[^a-z_.]open(' -e '[^a-z_.]close(' -e '[^a-z_.]open (' -e '[^a-z_.]close ('
Check 'unistd\.h' -e '[^a-z]_exit' -e '[^a-z]exec[lv]' -e setuid -e getuid -e chown -e '[^a-z_.]close(' -e isatty
Check 'sys/types\.h' -e uid_t -e gid_t -e "size_t[^y]" -e off_t
Check 'sys/stat\.h' -e fchown -e fchmod -e 'stat[^a-z]'
Check 'sys/mman\.h' -e mmap


