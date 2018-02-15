#!/usr/bin/env sh
Echo() {
	printf '%s\n' "$*"
}
Die() {
	Echo 'fatal:' "$*"
	exit 1
}
EchoExec() {
	Echo "# $@"
	"$@" || Die "failed: $*"
}
EchoExec contrib/make.sh -Wyo1ne "$@"

have=
have=`command -v include-what-you-use 2>/dev/null`
[ -n "$have" ] || for i in /usr/lib*/llvm/*/bin/include-what-you-use
do	test -x "$i" || continue
	have=$i
done
[ -n "$have" ] || Die "No include-what-you-use binary found"
Echo "CXX=$have"
Echo

EchoExec make -k CXX="$have"
