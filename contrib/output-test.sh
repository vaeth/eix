#!/bin/sh
# Compare the out of the installed eix with the output of the eix in ../src.
#
# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#   Emil Beinroth <emilbeinroth@gmx.net>

die() {
    echo "!!! failed $@"
    exit 1
}

installed_cache=$(tempfile)
testing_cache=$(tempfile)
installed_output=$(tempfile)
testing_output=$(tempfile)

cleanup_tempfiles() {
    trap : INT HUP TERM KILL
    rm -f -- "$installed_output" "$installed_cache" \
             "$testing_output" "$testing_cache"
    trap - INT HUP TERM KILL
}

trap cleanup_tempfiles INT HUP TERM KILL

eix_prefix=${0%/*}/../src

echo testing eix located in "$eix_prefix"

make -C "$eix_prefix"

echo '>> running installed eix-update'
EIX_CACHEFILE=$installed_cache eix-update || die

echo '>> running installed eix-update'
EIX_CACHEFILE=$testing_cache "$eix_prefix"/eix-update || did

EIX_CACHEFILE=$installed_cache eix > "$installed_output" || die
EIX_CACHEFILE=$testing_cache "$eix_prefix"/eix > "$testing_output" || die


echo "------- diffing -------"

diff "$installed_output" "$testing_output"

cleanup_tempfiles
