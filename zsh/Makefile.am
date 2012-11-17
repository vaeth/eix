
completionnogenerate = \
_eix-installed \
_eix-installed-after \
_eix-layman \
_eix-remote \
_eix-sync \
_eix-test-obsolete

completioninput = \
_eix.in \
$(completionnogenerate)

completiongenerate = \
_eix

completionoutput = \
$(completiongenerate) \
$(completionnogenerate)

EXTRA_DIST = $(completioninput)

CLEANFILES = $(completiongenerate)

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
		"$<" > "$@"
