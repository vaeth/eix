#!/bin/bash
# Try to compile eix with various version of gcc.

shopt -s extglob

get_avail_gcc() {
	echo /etc/env.d/gcc/!(*-hardened*|!(i?86-pc-linux-gnu-*)) \
	| xargs -n1 basename
}

reset_gcc() {
	gcc-config "${original_gcc}" >/dev/null &
	exit
}

set_status() {
	gcc_faild[$1]="$2"
}

gcc=($(get_avail_gcc))
gcc_faild=(${gcc[@]})

if which icpc &>/dev/null; then
	# we have ICC installed, try it
	for ((gcc_n=0;gcc_n<"${#gcc[@]}";++gcc_n)); do
		if [[ "${gcc[$gcc_n]}" != *"-4."* ]]; then
			for ((gcc_i=$[${#gcc[@]}-1];gcc_i>$gcc_n;--gcc_i)); do
				gcc[$[$gcc_i+1]]=${gcc[$gcc_i]}
			done
			gcc[$[$gcc_n+1]]=$(which icpc)
			break
		fi
	done
fi

original_gcc=$(gcc-config -c)
trap reset_gcc EXIT INT

for ((gcc_n=0;gcc_n<"${#gcc[@]}";++gcc_n)); do
	make distclean &>/dev/null
	echo "trying ${gcc[$gcc_n]}"

	unset CXX

	if [[ "${gcc[$gcc_n]:0:1}" != '/' ]]; then
		if ! gcc-config "${gcc[$gcc_n]}"; then
			set_status $gcc_n "(1 of 4) gcc-config failed"
			continue
		fi
		source /etc/profile
	else
		export CXX="${gcc[$gcc_n]}"
	fi

	if ! ./configure CXXFLAGS="-Wall -W -Werror ${CXXFLAGS}"; then
		set_status $gcc_n "(2 of 4) configure failed"
		continue
	fi

	if ! make clean all; then
		set_status $gcc_n "(3 of 4) make clean all"
		continue
	fi

	if ! make check; then
		set_status $gcc_n "(4 of 4) make check"
		continue
	fi

	set_status $gcc_n "ok"
done

echo
echo "== all done =="
echo

for ((gcc_n=0;gcc_n<"${#gcc[@]}";++gcc_n)); do
	echo "${gcc[$gcc_n]}: ${gcc_faild[$gcc_n]}"
done
