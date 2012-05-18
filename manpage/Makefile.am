# Makefile.am for manpage of eix


MAN_LINGUAS = \
en \
de \
ru

MAN_FILES = \
eix.1

# The following lists must contain all combinations of MAN_LINGUAS-MAN_FILES

CLEANFILES = \
en-eix.1 \
de-eix.1 \
ru-eix.1 \
cppcomplete.tags \
tags

man_MANS = \
en-eix.1 \
de-eix.1 \
ru-eix.1

EXTRA_DIST = \
en-eix.1.in \
de-eix.1.in \
ru-eix.1.in

install-man1:
	@for file in $(MAN_FILES); do \
	 for lang in $(MAN_LINGUAS); do \
	  case ' $(LINGUAS) en' in *" $${lang}"*) \
		if [ "$${lang}" = en ]; \
		then mansubdir=man1; \
		else mansubdir="$${lang}/man1"; \
		fi; \
		mandestdir='$(DESTDIR)$(mandir)/'"$${mansubdir}"; \
		sourcefile="$${lang}-$${file}"; \
		test -d "$${mandestdir}" || $(mkinstalldirs) "$${mandestdir}"; \
		echo " $(INSTALL_DATA) $${sourcefile} $${mandestdir}/$${file}"; \
		$(INSTALL_DATA) "$${sourcefile}" "$${mandestdir}/$${file}";; \
	  esac; \
	 done; \
	done

uninstall-man1:
	@for file in $(MAN_FILES); do \
	 for lang in $(MAN_LINGUAS); do \
		if [ "$${lang}" = en ]; \
		then mansubdir=man1; \
		else mansubdir="$${lang}/man1"; \
		fi; \
		mandestdir='$(DESTDIR)$(mandir)/'"$${mansubdir}"; \
		echo " $(RM) -f $${mandestdir}/$${file}"; \
		$(RM) -f "$${mandestdir}/$${file}"; \
	 done; \
	done

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
		-e 's/Ä/\\(:A/g' \
		-e 's/Ö/\\(:O/g' \
		-e 's/Ü/\\(:U/g' \
		-e 's/ä/\\(:a/g' \
		-e 's/ö/\\(:o/g' \
		-e 's/ü/\\(:u/g' \
		-e 's/ß/\\(ss/g' \
		"$<" >"$@"
	@A=`sed -n -e '1{s/^.*Date: \([^\(]*\).*/\1/p;q}' "$<"` && \
	[ -n "$$A" ] && touch --date="$$A" "$<" "$@" >/dev/null 2>&1 || :
