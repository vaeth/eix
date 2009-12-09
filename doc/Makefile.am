
textfiles = \
format.txt \
xml-format.txt

htmlfiles = \
format.html \
xml-format.html

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
format.txt.in \
xml-format.txt.in \
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

format.html: format.txt $(css_data)
	$(AM_V_GEN)rst2html.py format.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > "$@"

xml-format.html: xml-format.txt $(css_data)
	$(AM_V_GEN)rst2html.py xml-format.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > "$@"
