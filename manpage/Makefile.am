# Makefile.am for manpage of eix

CLEANFILES = eix.1 cppcomplete.tags tags
man_MANS = eix.1
EXTRA_DIST = eix.1.in

SUFFIXES = .in
.in:
	$(AM_V_GEN)$(SED) \
		-e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE),g" \
		-e "s,\@EIX_PREVIOUS\@,$(EIX_PREVIOUS),g" \
		-e "s,\@EIX_REMOTEARCHIVE\@,$(EIX_REMOTEARCHIVE),g" \
		-e "s,\@PACKAGE_BUGREPORT\@,$(PACKAGE_BUGREPORT),g" \
		-e "s,\@PACKAGE_URL\@,$(PACKAGE_URL),g" \
		-e "s,\@LEVENSHTEIN_DISTANCE\@,$(LEVENSHTEIN_DISTANCE),g" \
		-e "s,\@PORTDIR_CACHE_METHOD\@,$(PORTDIR_CACHE_METHOD),g" \
		-e "s,\@BINDIR\@,$(bindir),g" \
		-e "s,\@SYSCONFDIR\@,$(sysconfdir),g" \
		-e "s,\@DEFAULT_REMOTE_PATH\@,$(DEFAULT_REMOTE_PATH),g" \
		-e "s,/dev/null,$(DEV_NULL),g" \
		-e 's/$$Date:[^,]*, *\([^)]*\)[^\$$]*\$$/\1/' \
		-e 's,\([BI]\)<\([^>]\+\)>,\\f\1\2\\fR,g' \
		"$<" >"$@"
	@A=`sed -n -e '1{s/^.*Date: \([^\(]*\).*/\1/p;q}' "$<"` && \
	[ -n "$$A" ] && touch --date="$$A" "$<" "$@" >/dev/null 2>&1 || :
