#! /bin/sh
if command -v rst2html.py >/dev/null 2>&1
then	rst2html.py \
		'--input-encoding=UTF-8:strict' \
		'--embed-stylesheet' \
		'--stylesheet-path=stylesheet.css' \
		-- "${1}" "${2}"
	exit
fi
if test -r "${2}"
then	echo "warning: using existing ${2} since rst2html.py is not available" >&2
	touch "${2}" || : >>"${2}"
	exit
else	echo "error: ${2} does not exist and rst2html.py is not available" >&2
	exit 1
fi
