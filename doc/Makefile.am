
textfiles = \
eix-db.txt \
eix-xml.txt

htmlfiles = \
eix-db.html \
eix-xml.html

xmldocfiles = \
eix-xml.xsd

CLEANFILES = \
$(textfiles) \
$(htmlfiles) \
$(xmldocfiles)

dist_html_DATA = \
$(htmlfiles) \
$(xmldocfiles)

nodist_doc_DATA = \
$(textfiles)

css_data = \
stylesheet.css \
html4css1.css

EXTRA_DIST = \
$(css_data) \
eix-db.txt.in \
eix-xml.txt.in \
eix-xml.xsd.in \
call_rst2html.sh

SUFFIXES = .in
.in:
	$(AM_V_GEN)A='s/^.*static * CONSTEXPR *.* current *= *\([0-9]*\) *;.*$$/\1/p' ; \
	H=`$(SED) -n -e "$$A" ../src/database/header.h` ; \
	X=`$(SED) -n -e "$$A" ../src/output/print-xml.h` ; \
	$(SED) \
		-e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE),g" \
		-e "s,\@EIX_REMOTECACHEFILE1\@,$(EIX_REMOTECACHEFILE1),g" \
		-e "s,\@EIX_REMOTECACHEFILE2\@,$(EIX_REMOTECACHEFILE2),g" \
		-e "s,\@EIX_PREVIOUS\@,$(EIX_PREVIOUS),g" \
		-e "s,\@EIX_REMOTEARCHIVE1\@,$(EIX_REMOTEARCHIVE1),g" \
		-e "s,\@EIX_REMOTEARCHIVE2\@,$(EIX_REMOTEARCHIVE2),g" \
		-e "s,\@PACKAGE_BUGREPORT\@,$(PACKAGE_BUGREPORT),g" \
		-e "s,\@PACKAGE_URL\@,$(PACKAGE_URL),g" \
		-e "s,\@LEVENSHTEIN_DISTANCE_DEFAULT\@,$(LEVENSHTEIN_DISTANCE_DEFAULT),g" \
		-e "s,\@PORTDIR_CACHE_METHOD\@,$(PORTDIR_CACHE_METHOD),g" \
		-e "s,\@BINDIR\@,$(bindir),g" \
		-e "s,\@SYSCONFDIR\@,$(sysconfdir),g" \
		-e "s,\@DEFAULT_REMOTE_PATH1\@,$(DEFAULT_REMOTE_PATH1),g" \
		-e "s,\@DEFAULT_REMOTE_PATH2\@,$(DEFAULT_REMOTE_PATH2),g" \
		-e "s,/dev/null,$(DEV_NULL),g" \
		-e "s,\@CURRENT\@,$$H,g" \
		-e "s,\@CURRENTXML\@,$$X,g" \
		-e 's,^#!/usr/bin/env sh$$,#!$(SH_SHEBANG),g' \
	"$<" > "$@"

eix-db.html: eix-db.txt $(css_data)
	$(AM_V_GEN)$(SHELL) ./call_rst2html.sh eix-db.txt "$@" "$@" . "$(DEV_NULL)"

eix-xml.html: eix-xml.txt $(css_data)
	$(AM_V_GEN)$(SHELL) ./call_rst2html.sh eix-xml.txt "$@" "$@" . "$(DEV_NULL)"
