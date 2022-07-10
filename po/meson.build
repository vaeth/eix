langs = []
test_linguas='''case " ${LINGUAS-*} " in
*" $1 "*|*' * '*) exit 1;;
esac
'''
test_have_po='''test -s "$1.po"
'''
check_langs = [
	['de', true],
	['en@quot', false],
	['en@boldquot', false],
	['ru', true],
]
foreach p : check_langs
	l = p.get(0)
	r = run_command(sh, '-c', test_linguas, 'sh', l, check : false)
	if r.returncode() != 0
		if p.get(1)
			langs += [ l ]
			manpages += [ l ]
		else
			s = run_command(sh, '-c', test_have_po, 'sh', l, check : false)
			if s.returncode() == 0
				langs += [ l ]
			endif
		endif
	endif
endforeach
xgettext_options = [
	'--keyword=_',
	'--keyword=N_:1,2',
	'--keyword=P_:1c,2',
	'--keyword=NP_:1c,2,3',
	'--keyword=pgettext:1c,2',
	'--keyword=npgettext:1c,2,3',
	'--keyword=eval_pgettext:1c,2',
	'--keyword=eval_npgettext:1c,2,3',
	'--add-location=file',
	'--no-wrap',
	'--check=space-ellipsis',
]
i18n = import('i18n')
if langs.length() != 0
	i18n.gettext(package,
		languages : langs,
		args : xgettext_options,
	)
endif
