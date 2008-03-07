#!/bin/bash
#   simple-benchmark.sh - collect simple stats about the execution speed of the
#   program given as argument. The terms used are probably *totally* wrong.
#
# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
# Copyright (c)
#   Emil Beinroth <emilbeinroth@gmx.net>

_get_time() {
    TIMEFORMAT="%3$1" eval time "$2 &>/dev/null" 2>&1
}

real() {
    _get_time R "$*"
}

average() {
    :
}

calc() {
    echo "scale = 3; $@" | bc | sed -r -e 's,([^0-9]|^)\.,\10.,';
}

pad() {
    if [ "${1:0:1}" != "-" ]; then
        echo " $1"
    else
        echo "$1"
    fi
}

sample_size=$1
shift

stats=()
last_stat=0

add_stat() {
    last_stat=$1
    stats[${#stats[@]}]=$1
}

echo -n "caching phase .. "
echo $(real "$@")

for((i=0;i<$sample_size;++i)); do
    this_time=$(real "$@")
    echo job \#$i -- $(pad $this_time)
    add_stat $this_time
done

average=$(echo "(${stats[*]})" | sed -r 's, +, + ,g')
average=$(calc $average / ${#stats[*]})

echo
echo "-- average:          $(pad $average) s"
echo

variances=()

for((i=0;i<$sample_size;++i)); do
    variances[$i]=$(calc $average - ${stats[$i]})
    echo "-- variance #$i:      $(pad ${variances[$i]}) s"
done

average_variance=$(echo "(${variances[*]})" | sed -r -e 's,-,,g' -e 's, +, + ,g')
average_variance=$(calc $average_variance / ${#variances[*]})

echo
echo "-- average variance: $(pad $average_variance) s"
