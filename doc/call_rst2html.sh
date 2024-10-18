#!/usr/bin/env sh
if command -v rst2html >"$5" 2>&1
then	rst2html_cmd=rst2html
elif command -v rst2html.py >"$5" 2>&1
then	rst2html_cmd=rst2html.py
else	rst2html_cmd=
fi
if [ -n "$rst2html_cmd" ]
then	"$rst2html_cmd" \
		'--input-encoding=UTF-8:strict' \
		'--embed-stylesheet' \
		"--stylesheet-path=$4/stylesheet.css" \
		-- "$1" "$2"
	exit
fi
if test -r "$2"
then	echo "warning: using existing $2 since rst2html.py is not available" >&2
	touch -- "$2" || : >>"$2"
	exit
elif test -r "$3"
then	echo "warning: using existing $3 since rst2html.py is not available" >&2
	cp -- "$3" "$2"
	exit
else	echo "error: $2 does not exist and rst2html.py is not available" >&2
	exit 1
fi
