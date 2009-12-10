
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

if USE_RST
rsthtmlfiles = $(htmlfiles)
else
rsthtmlfiles =
endif

nodist_html_DATA = $(xmldocfiles) $(rsthtmlfiles)

nodist_doc_DATA = $(textfiles)

css_data = \
stylesheet.css \
html4css1.css

EXTRA_DIST = \
$(css_data) \
eix-db.txt.in \
eix-xml.txt.in \
eix-xml.xsd.in

SUFFIXES = .in
.in:
	$(AM_V_GEN)A='s/^.*static * const *.* current *= *\([0-9]*\) *;.*$$/\1/p' ; \
	H=`$(SED) -n -e "$${A}" ../src/database/header.h` ; \
	X=`$(SED) -n -e "$${A}" ../src/output/print-xml.h` ; \
	$(SED) -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@CURRENT\@,$${H},g" \
		-e "s,\@CURRENTXML\@,$${X},g" \
	"$<" > "$@"

eix-db.html: eix-db.txt $(css_data)
	$(AM_V_GEN)rst2html.py eix-db.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > "$@"

eix-xml.html: eix-xml.txt $(css_data)
	$(AM_V_GEN)rst2html.py eix-xml.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > "$@"
