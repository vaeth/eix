#!/usr/bin/env sh
export LC_ALL=C
command -v cpplint.py >/dev/null 2>&1 || {
	echo 'Cannot find cpplint.py.  Please download it from
http://google-styleguide.googlecode.com/svn/trunk/cpplint/cpplint.py
or install it from the mv overlay to execute C++ style test.' >&2
	exit 1
}
exec find . '-(' -name '*.cc' -o -name '*.h' '-)' '-!' -name 'config.h' '-!' -name 'confdefs.h' -exec cpplint.py \
	--filter=-whitespace/tab,-whitespace/braces,-whitespace/parens,-whitespace/labels,-whitespace/line_length,-readability/streams \
	'{}' '+'
