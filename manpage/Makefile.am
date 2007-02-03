# Makefile.am for manpage of eix

CLEANFILES = eix.1 cppcomplete.tags tags
man_MANS = eix.1
EXTRA_DIST = eix.1.in

eix.1: eix.1.in
	@regex_cmd@ -e 's/$$Date:[^,]*, *\([^)]*\)[^\$$]*\$$/\1/' \
	    -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING)," \
	    -e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE)," \
	    -e "s,\@EIX_WIKI\@,$(EIX_WIKI)," \
	    -e "s,\@LEVENSHTEIN_DISTANCE\@,$(LEVENSHTEIN_DISTANCE)," \
	    -e "s,\@PORTDIR_CACHE_METHOD\@,$(PORTDIR_CACHE_METHOD)," \
	    -e "s,\@BINDIR\@,$(bindir)," \
	    -e 's,\([BI]\)<\([^>]\+\)>,\\f\1\2\\fR,g' $(srcdir)/eix.1.in > eix.1
# -e 's,\<_\([^[:space:],.;:]\+\)_\>,\\fI\1\\fR,g' \
# -e 's,\*\([^[:space:],.;:]\+\)\*,\\fB\1\\fR,g' $(srcdir)/eix.1.in > eix.1
