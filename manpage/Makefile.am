# Makefile.am for manpage of eix

MAN_FILES = \
eix.1

if USING_NLS
foreign_man_linguas = \
de \
ru

foreign_man_mans = \
de-eix.1 \
ru-eix.1
else
foreign_man_linguas =
foreign_man_mans =
endif

MAN_LINGUAS = \
$(foreign_man_linguas) \
en

man_MANS = \
$(foreign_man_mans) \
en-eix.1

EXTRA_DIST = \
en-eix.1.in \
de-eix.1.in \
ru-eix.1.in

CLEANFILES = \
en-eix.1 \
de-eix.1 \
ru-eix.1 \
cppcomplete.tags \
tags

install-man1:
	@for file in $(MAN_FILES); do \
	 for lang in $(MAN_LINGUAS); do \
	  case ' $(INST_MAN_LINGUAS) en' in *" $$lang"*|*"*"*) \
		if [ "$$lang" = en ]; \
		then mansubdir=man1; \
		else mansubdir=$$lang/man1; \
		fi; \
		mandestdir='$(DESTDIR)$(mandir)/'"$$mansubdir"; \
		sourcefile=$$lang-$$file; \
		test -d "$$mandestdir" || $(mkinstalldirs) "$$mandestdir"; \
		echo " $(INSTALL_DATA) $$sourcefile $$mandestdir/$$file"; \
		$(INSTALL_DATA) "$$sourcefile" "$$mandestdir/$$file";; \
	  esac; \
	 done; \
	done

uninstall-man1:
	@for file in $(MAN_FILES); do \
	 for lang in $(MAN_LINGUAS); do \
	  case ' $(INST_MAN_LINGUAS) en' in *" $$lang"*|*"*"*) \
		if [ "$$lang" = en ]; \
		then mansubdir=man1; \
		else mansubdir=$$lang/man1; \
		fi; \
		mandestdir='$(DESTDIR)$(mandir)/'"$$mansubdir"; \
		echo " $(RM) -f $$mandestdir/$$file"; \
		$(RM) -f "$$mandestdir/$$file";; \
	  esac
	 done; \
	done

SUFFIXES = .in
.in:
	$(AM_V_GEN)$(SED) \
		-e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE),g" \
		-e "s,\@EIX_REMOTECACHEFILE1\@,$(EIX_REMOTECACHEFILE1),g" \
		-e "s,\@EIX_REMOTECACHEFILE2\@,$(EIX_REMOTECACHEFILE2),g" \
		-e "s,\@EIX_PREVIOUS\@,$(EIX_PREVIOUS),g" \
		-e "s,\@EIX_REMOTEARCHIVE1\@,$(EIX_REMOTEARCHIVE1),g" \
		-e "s,\@EIX_REMOTEARCHIVE2\@,$(EIX_REMOTEARCHIVE2),g" \
		-e "s,\@PACKAGE_BUGREPORT\@,$(PACKAGE_BUGREPORT),g" \
		-e "s,\@PACKAGE_URL\@,$(PACKAGE_URL),g" \
		-e "s,\@LEVENSHTEIN_DISTANCE_DEFAULT\@,$(LEVENSHTEIN_DISTANCE_DEFAULT),g" \
		-e "s,\@PORTDIR_CACHE_METHOD\@,$(PORTDIR_CACHE_METHOD),g" \
		-e "s,\@BINDIR\@,$(bindir),g" \
		-e "s,\@SYSCONFDIR\@,$(sysconfdir),g" \
		-e "s,\@DEFAULT_REMOTE_PATH1\@,$(DEFAULT_REMOTE_PATH1),g" \
		-e "s,\@DEFAULT_REMOTE_PATH2\@,$(DEFAULT_REMOTE_PATH2),g" \
		-e "s,/dev/null,$(DEV_NULL),g" \
		-e 's,^#!/usr/bin/env sh$$,#!$(SH_SHEBANG),g' \
		-e 's/$$Date:[^,]*, *\([^)]*\)[^\$$]*\$$/\1/' \
		-e 's,\([BI]\)<\([^>]\+\)>,\\f\1\2\\fR,g' \
		-e 's/\xc3\x84/\\(:A/g' \
		-e 's/\xc3\x96/\\(:O/g' \
		-e 's/\xc3\x9c/\\(:U/g' \
		-e 's/\xc3\xa4/\\(:a/g' \
		-e 's/\xc3\xb6/\\(:o/g' \
		-e 's/\xc3\xbc/\\(:u/g' \
		-e 's/\xc3\x9f/\\(ss/g' \
		-- "$<" >|"$@"
