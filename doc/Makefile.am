
CLEANFILES = \
format.txt \
format.html \
xml-format.txt \
xml-format.html

if USE_RST
doc_rst = format.html xml-format.html
else
doc_rst =
endif

css_data = \
stylesheet.css \
html4css1.css

nodist_noinst_DATA = \
$(doc_rst) \
format.txt \
xml-format.txt

EXTRA_DIST = \
$(css_data) \
format.txt.in \
xml-format.txt.in

SUFFIXES = .in
.in:
	$(AM_V_GEN)A='s/^.*static * const *.* current *= *\([0-9]*\) *;.*$$/\1/p' ; \
	H=`@regex_cmd@ -n -e "$${A}" ../src/database/header.h` ; \
	X=`@regex_cmd@ -n -e "$${A}" ../src/output/print-xml.h` ; \
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@CURRENT\@,$${H},g" \
		-e "s,\@CURRENTXML\@,$${X},g" \
	"$<" > "$@"

format.html: format.txt $(css_data)
	$(AM_V_GEN)rst2html.py format.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > format.html

xml-format.html: xml-format.txt $(css_data)
	$(AM_V_GEN)rst2html.py xml-format.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > xml-format.html
