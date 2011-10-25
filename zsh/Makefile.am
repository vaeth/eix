
completioninput = \
_eix.in \
_eix-installed.in \
_eix-installed-after.in \
_eix-layman.in \
_eix-remote.in \
_eix-sync.in \
_eix-test-obsolete.in

completionoutput = \
_eix \
_eix-installed \
_eix-installed-after \
_eix-layman \
_eix-remote \
_eix-sync \
_eix-test-obsolete

EXTRA_DIST = $(completioninput)

CLEANFILES = $(completionoutput)

if INSTALL_ZSH_COMPLETION
noinst_DATA = $(completionoutput)

install-data-local:
	$(AM_V_at)test -d "$(DESTDIR)$(ZSH_COMPLETION)" || $(MKDIR_P) "$(DESTDIR)$(ZSH_COMPLETION)" && \
		for i in $(completionoutput); do $(INSTALL_DATA) "$${i}" "$(DESTDIR)$(ZSH_COMPLETION)"; done

uninstall-local:
	$(AM_V_at)for i in $(completionoutput); do $(RM) "$(DESTDIR)$(ZSH_COMPLETION)/$${i}"; done

endif

SUFFIXES = .in
.in:
	$(AM_V_GEN)$(SED) -e 's/$$Date:[^,]*, *\([^)]*\)[^\$$]*\$$/\1/' \
		-e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE),g" \
		-e "s,\@PACKAGE_BUGREPORT\@,$(PACKAGE_BUGREPORT),g" \
		-e "s,\@PACKAGE_URL\@,$(PACKAGE_URL),g" \
		-e "s,\@LEVENSHTEIN_DISTANCE\@,$(LEVENSHTEIN_DISTANCE),g" \
		-e "s,\@PORTDIR_CACHE_METHOD\@,$(PORTDIR_CACHE_METHOD),g" \
		-e "s,/dev/null,$(DEV_NULL),g" \
		"$<" > "$@"
