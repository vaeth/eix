
CLEANFILES = format.txt format.html

if USE_RST
doc_rst = format.html
else
doc_rst =
endif
css_data = stylesheet.css html4css1.css

noinst_DATA = format.txt $(doc_rst)
EXTRA_DIST = format.txt.in $(css_data)

SUFFIXES = .in
.in:
	@echo "  SED   " "$@"
	@A=`@regex_cmd@ -n -e 's/^.*static * const *.* current *= *\([0-9]*\) *;.*$$/\1/p' ../src/database/header.h` ; \
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@CURRENT\@,$${A},g" \
	"$<" > "$@"

format.html: format.txt $(css_data)
	@echo "  RST2  " "$@"
	@rst2html.py format.txt --embed-stylesheet \
		--stylesheet-path=stylesheet.css > format.html
