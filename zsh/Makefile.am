
completionfiles = \
_eix \
_eix-installed \
_eix-installed-after \
_eix-layman \
_eix-remote \
_eix-sync \
_eix-test-obsolete

EXTRA_DIST = $(completionfiles)

install-data-local:
	$(AM_V_at)test x"$(ZSH_COMPLETION)" = x"" || for i in $(completionfiles); do $(INSTALL_DATA) -D "$${i}" "$(DESTDIR)$(ZSH_COMPLETION)/$${i}"; done

uninstall-local:
	$(AM_V_at)test x"$(ZSH_COMPLETION)" = x"" || for i in $(completionfiles); do $(RM) "$(DESTDIR)$(ZSH_COMPLETION)/$${i}"; done
