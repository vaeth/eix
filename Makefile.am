# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin V\"ath <vaeth@mathematik.uni-wuerzburg.de>

# If you do not have access to automake-1.11 but want to compile the
# eix repository anyway, you can also use the commented AUTOMAKE_OPTIONS:
# then any >=automake-1.4 should do (although early versions are untested).
# However, for creating a distribution tarball, you should not do this:
# I would like to see in distributions only the "silent rules" default
# (and also the .tar.xz format for the tarball, at least for the default).
# Both of these are only available with automake-1.11, therefore the latter
# is required by default. Moreover, only in >=automake-1.11.2 silent rules
# work also without dependency tracking
#AUTOMAKE_OPTIONS = foreign 1.4
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
EXTRA_DIST = config/config.rpath doxygen.cfg.in bash/eix

ACLOCAL_AMFLAGS = -I m4 -I martinm4

.PHONY: doxygen doxygen-clean

# Remove things created by autogen.sh
maintainer-clean-local:
	$(AM_V_at)$(RM) -vfr "$(top_srcdir)/build" "$(top_srcdir)/config" "$(top_srcdir)/m4"
	$(AM_V_at)$(RM) -vf "$(top_srcdir)/ABOUT-NLS" "$(top_srcdir)/aclocal.m4"
	$(AM_V_at)$(RM) -vf "$(top_srcdir)/configure" "$(top_srcdir)/config.h.in~"

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
