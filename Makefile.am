# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

AUTOMAKE_OPTIONS = foreign 1.4 no-dist-gzip dist-xz

SUBDIRS = po src manpage doc

CLEANFILES = doxygen.cfg cppcomplete.tags tags

# Stuff from our distribution
EXTRA_DIST = config/config.rpath doxygen.cfg.in

ACLOCAL_AMFLAGS = -I m4

# Remove things created by autogen.sh
maintainer-clean-local:
	$(AM_V_at)$(RM) -vfr "$(top_srcdir)/build" "$(top_srcdir)/config" "$(top_srcdir)/m4"
	$(AM_V_at)$(RM) -vf "$(top_srcdir)/ABOUT-NLS" "$(top_srcdir)/aclocal.m4"
	$(AM_V_at)$(RM) -vf "$(top_srcdir)/configure" "$(top_srcdir)/config.h.in~"

# Remove doxygen/ and rpo files (-frepo with gcc) on clean
clean-local: doxygen-clean
	$(AM_V_at)find "$(top_srcdir)" -name '*.rpo' -exec $(RM) -v -- '{}' '+'

.PHONY: doxygen

# Build source docs with doxygen
doxygen: doxygen-clean
	$(AM_V_at)@regex_cmd@ -e "s,\@PACKAGE_NAME\@,$(PACKAGE)," \
		-e "s,\@PACKAGE_VERSION\@,$(VERSION)," doxygen.cfg.in > doxygen.cfg
	$(AM_V_GEN)doxygen doxygen.cfg >/dev/null

# Remove build source-docs
doxygen-clean:
	$(AM_V_at)$(RM) -fr doxygen
