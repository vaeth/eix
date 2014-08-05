#!/usr/bin/env sh
export LC_ALL=C
command -v cpplint.py >/dev/null 2>&1 || {
	echo 'Cannot find cpplint.py.  Please download it from
https://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
or install it from the mv overlay to execute C++ style test.' >&2
	exit 1
}
exec find . '-(' -name '*.cc' -o -name '*.h' '-)' -exec cpplint.py '{}' '+'
