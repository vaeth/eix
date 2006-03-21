#!/bin/sh
# Try to compile eix with various version of gcc.

die () {
	echo "Current gcc: $gcc"
	exit 1
}

old_gcc=$(gcc-config -c)

trap "gcc-config ${old_gcc}" EXIT

for gcc in /etc/env.d/gcc/i686-pc-linux-gnu-*
do
	[[ "$gcc" = *"-hardened"* ]] && continue
	gcc="${gcc##*/}"

	gcc-config "${gcc}" || die

	source /etc/profile
	
	./configure CXXFLAGS="-Wall -W -Werror -g3 -ggdb3" || die
	make clean all || die

done
