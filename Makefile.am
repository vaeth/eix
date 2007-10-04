# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>

AUTOMAKE_OPTIONS = foreign 1.4 dist-bzip2

SUBDIRS = src manpage contrib

CLEANFILES = doxygen.cfg cppcomplete.tags tags

# Stuff from our distribution
EXTRA_DIST = doxygen.cfg.in 

# Remove doxygen/ on clean
clean-local: doxygen-clean

# Build source docs with doxygen
.PHONY:
doxygen: doxygen-clean
	@regex_cmd@ -e "s,\@PACKAGE_NAME\@,$(PACKAGE)," \
		-e "s,\@PACKAGE_VERSION\@,$(VERSION)," doxygen.cfg.in > doxygen.cfg
	@doxygen doxygen.cfg

# Remove build source-docs
doxygen-clean:
	@rm -fr doxygen
