# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin V\"ath <martin@mvath.de>

AUTOMAKE_OPTIONS = foreign 1.11.2 no-dist-gzip dist-xz -Wobsolete -Wportability -Wextra-portability

if EXTRA_DOC
extra_doc_subdir = doc
else
extra_doc_subdir =
endif

SUBDIRS = po src manpage zsh $(extra_doc_subdir)

DIST_SUBDIRS = po src manpage zsh doc

CLEANFILES = doxygen.cfg cppcomplete.tags tags

dist_doc_DATA = AUTHORS ChangeLog

# Stuff from our distribution
EXTRA_DIST = \
bash/eix \
config/config.rpath \
contrib/check_includes.sh \
contrib/cpplint.sh \
contrib/meson-flto-test.sh \
.gitignore \
autogen.sh \
CPPLINT.cfg \
doxygen.cfg.in \
meson.build \
meson_options.txt \
po/meson.build \
po/Rules-clean \
tmpfiles.d/eix.conf

ACLOCAL_AMFLAGS = -I m4 -I martinm4

.PHONY: doxygen doxygen-clean

# Remove things created by autogen.sh
maintainer-clean-local:
	$(AM_V_at)$(RM) -vfr "$(top_srcdir)/build" "$(top_srcdir)/config" \
		"$(top_srcdir)/m4" "$(top_srcdir)/builddir"
	$(AM_V_at)$(RM) -vf "$(top_srcdir)/ABOUT-NLS" "$(top_srcdir)/aclocal.m4" \
		"$(top_srcdir)/configure" "$(top_srcdir)/config.h.in~"

# Remove doxygen/ and rpo files (-frepo with gcc) on clean
clean-local: doxygen-clean
	$(AM_V_at)find "$(top_srcdir)" -name '*.rpo' -exec $(RM) -v -- '{}' '+'

# Build source docs with doxygen
doxygen: doxygen-clean
	$(AM_V_at)$(SED) -e "s,\@PACKAGE_NAME\@,$(PACKAGE)," \
		-e "s,\@PACKAGE_VERSION\@,$(VERSION)," doxygen.cfg.in > doxygen.cfg
	$(AM_V_GEN)doxygen doxygen.cfg >/dev/null

# Remove build source-docs
doxygen-clean:
	$(AM_V_at)$(RM) -vfr doxygen
