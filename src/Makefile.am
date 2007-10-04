# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

# Build our toolkits
SUBDIRS = eixTk eixrc database portage output search

# Remove files we don't want ..
CLEANFILES = cppcomplete.tags tags \
	eix-sync eix-test-obsolete \
	update-eix-remote update-eix-layman update-eix-functions.sh

EXTRA_DIST = eix-sync.in eix-test-obsolete.in \
	update-eix-remote.in update-eix-layman.in update-eix-functions.sh.in

# Our binaries
bin_PROGRAMS = update-eix eix diff-eix
bin_SCRIPTS  = eix-sync eix-test-obsolete \
	update-eix-remote update-eix-layman update-eix-functions.sh

# The search-tool for our database
eix_SOURCES = eix.cc \
			  varsreader.cc varsreader.h \
			  cachetable.h \
			  cli.cc cli.h \
			  global.cc global2.cc global3.cc global4.cc global.h \
			  portage/set_stability.h portage/conf/setmask.cc

eix_LDADD = database/libdatabase.a \
			portage/conf/libportageconf.a \
			portage/libportage.a \
			output/liboutput.a \
			eixTk/libeixTk.a \
			eixrc/libeixrc.a \
			search/libsearch.a \
			@BZLIB_LIBS@

# The update-tool for our database
update_eix_SOURCES = update-eix.cc \
				     varsreader.cc varsreader.h \
					 global.cc global2.cc global3.cc global4.cc global.h \
					 portage/set_stability.h portage/conf/setmask.cc

update_eix_LDADD = database/libdatabase.a \
				   portage/conf/libportageconf.a \
				   portage/libportage.a \
				   portage/cache/libcache.a \
				   eixTk/libeixTk.a \
				   eixrc/libeixrc.a \
				   @SQLITE_LIBS@ \
				   @BZLIB_LIBS@


# The diff-tool for our database
diff_eix_SOURCES = diff-eix.cc \
				   varsreader.cc varsreader.h \
				   global.cc global2.cc global3.cc global4.cc global.h \
				   portage/set_stability.h portage/conf/setmask.cc

diff_eix_LDADD = database/libdatabase.a \
				 portage/cache/libcache.a \
				 portage/conf/libportageconf.a \
				 portage/libportage.a \
				 output/liboutput.a \
				 eixTk/libeixTk.a \
				 eixrc/libeixrc.a \
				 @BZLIB_LIBS@

eix-sync: eix-sync.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING)," \
	    -e "s,update-eix-functions.sh.in,'$(bindir)/update-eix-functions.sh'," \
	    -e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE)," \
	$(srcdir)/eix-sync.in > eix-sync

eix-test-obsolete: eix-test-obsolete.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING)," \
	$(srcdir)/eix-test-obsolete.in > eix-test-obsolete

update-eix-functions.sh: update-eix-functions.sh.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING)," \
	$(srcdir)/update-eix-functions.sh.in > update-eix-functions.sh

update-eix-remote: update-eix-remote.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING)," \
	    -e "s,update-eix-functions.sh.in,'$(bindir)/update-eix-functions.sh'," \
	$(srcdir)/update-eix-remote.in > update-eix-remote

update-eix-layman: update-eix-layman.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING)," \
	    -e "s,update-eix-functions.sh.in,'$(bindir)/update-eix-functions.sh'," \
	$(srcdir)/update-eix-layman.in > update-eix-layman

