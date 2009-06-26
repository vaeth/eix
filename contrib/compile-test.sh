#!/bin/bash
#	compile-test.sh tries to compile and run eix with various version of gcc
#	and the ICC. It also does one `make distcheck` run for a compiler that has
#	passed the normal build test.
#
# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#	Emil Beinroth <emilbeinroth@gmx.net>

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
		if [[ "${gcc[$gcc_n]}" = *"-3.4"* ]]; then
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
			set_status $gcc_n "(1 of 6) gcc-config failed"
			continue
		fi
		source /etc/profile
		export CXXFLAGS="-Wall -W -Werror"
	else
		source /etc/profile
		export CXX="${gcc[$gcc_n]}"
		export CXXFLAGS=""
	fi

	eix_prefix=$(mktemp -dt eix-prefix.XXXXXXXXXX)

	echo ./configure CXXFLAGS="${CXXFLAGS}" --prefix="$eix_prefix" \
		--exec-prefix="$eix_prefix"
	if ! ./configure CXXFLAGS="${CXXFLAGS}" --prefix="$eix_prefix" \
		--exec-prefix="$eix_prefix"; then
		set_status $gcc_n "(2 of 6) configure failed"
		continue
	fi

	if ! make clean all; then
		set_status $gcc_n "(3 of 6) make clean all"
		continue
	fi

	# Try a make distcheck if the toolchain can build eix.
	if ! [ "$did_distcheck" ]; then
		if ! CXXFLAGS="${CXXFLAGS}" make distcheck; then
			did_distcheck="failed"
		else
			did_distcheck="ok"
		fi
	fi

	if ! make install; then
		set_status $gcc_n "(4 of 6) make install (in $eix_prefix)"
		continue
	fi

	if ! "${eix_prefix}"/bin/eix-update; then
		set_status $gcc_n "(5 of 6) running eix-update (in $eix_prefix)"
		continue
	fi

	if ! "${eix_prefix}"/bin/eix eix; then
		set_status $gcc_n "(6 of 6) running eix (in $eix_prefix)"
		continue
	fi

	if [ -d "$eix_prefix" ]; then
		rm -fr "$eix_prefix"
	fi

	set_status $gcc_n "ok"
done

echo
echo "== all done =="
echo

for ((gcc_n=0;gcc_n<"${#gcc[@]}";++gcc_n)); do
	echo "${gcc[$gcc_n]}: ${gcc_faild[$gcc_n]}"
done

echo "distcheck: ${did_distcheck:-not run}"
