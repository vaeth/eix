# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

AUTOMAKE_OPTIONS = subdir-objects

AM_CXXFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\"

sysconf_DATA = eixrc/eixrc

database_src = \
database/io.cc \
database/io.h \
database/types.h \
database/header.cc \
database/header.h \
database/package_reader.cc \
database/package_reader.h

eixtk_src = \
eixTk/ansicolor.h \
eixTk/argsreader.cc \
eixTk/argsreader.h \
eixTk/auto_ptr.h \
eixTk/compare.h \
eixTk/exceptions.h \
eixTk/filenames.cc \
eixTk/filenames.h \
eixTk/formated.h \
eixTk/inttypes.h \
eixTk/ptr_list.h \
eixTk/regexp.h \
eixTk/stringutils.cc \
eixTk/stringutils.h \
eixTk/sysutils.cc \
eixTk/sysutils.h \
eixTk/utils.cc \
eixTk/utils.h

eixrc_src = \
eixrc/eixrc.cc \
eixrc/eixrc.h \
global.cc \
global2.cc \
global3.cc \
global4.cc \
global.h

portage_src = \
portage/conf/portagesettings.cc \
portage/conf/portagesettings.h \
portage/conf/cascadingprofile.cc \
portage/conf/cascadingprofile.h \
portage/basicversion.cc \
portage/basicversion.h \
portage/instversion.h \
portage/mask.cc \
portage/mask.h \
portage/package.cc \
portage/package.h \
portage/vardbpkg.cc \
portage/vardbpkg.h \
portage/packagetree.cc \
portage/packagetree.h \
portage/keywords.cc \
portage/keywords.h \
portage/mask_list.h \
portage/overlay.h \
portage/set_stability.cc \
portage/set_stability.h \
portage/version.h

cache_src = \
cachetable.h \
cache/common/flat-reader.cc \
cache/common/flat-reader.h \
cache/common/ebuild_exec.cc \
cache/common/ebuild_exec.h \
cache/common/selectors.cc \
cache/common/selectors.h \
cache/common/unpickle.cc \
cache/common/unpickle.h \
cache/base.cc \
cache/base.h \
cache/cache-map.cc \
cache/cache-map.h \
cache/cdb/cdb.cc \
cache/cdb/cdb.h \
cache/ebuild/ebuild.cc \
cache/ebuild/ebuild.h \
cache/eixcache/eixcache.cc \
cache/eixcache/eixcache.h \
cache/metadata/metadata.cc \
cache/metadata/metadata.h \
cache/parse/parse.cc \
cache/parse/parse.h \
cache/port2_1_0/port2_1_0.cc \
cache/port2_1_0/port2_1_0.h \
cache/sqlite/sqlite.cc \
cache/sqlite/sqlite.h

extra_cache_src = \
cache/port2_1_2/port2_1_2.cc \
cache/port2_1_2/port2_1_2.h

if EXTRA_CACHE
extra_cache = portage-2.1.2
add_cache_src = $(extra_cache_src)
else
extra_cache =
add_cache_src =
endif

output_src = \
output/formatstring.cc \
output/formatstring.h \
output/formatstring-print.cc \
output/formatstring-print.h

search_src =  \
search/levenshtein.cc \
search/levenshtein.h \
search/algorithms.cc \
search/algorithms.h \
search/dbmatchcriteria.h \
search/packagetest.cc \
search/packagetest.h \
search/redundancy.h

# Common to all binaries:
common_ldadd = @BZLIB_LIBS@
common_src = varsreader.cc varsreader.h main/main.h \
		   $(database_src) $(portage_src) $(eixtk_src) $(eixrc_src)
extra_common_src = eixrc/defaults.cc main/main.cc \
		   main/main_eix.cc main/main_diff_eix.cc main/main_update_eix.cc \
		   main/main_output.cc main/main_all.cc

# The search-tool for our database
eix_only_ldadd =
eix_only_src = eix.cc cli.cc cli.h $(search_src)
extra_eix_only_src =

# The diff-tool for our database
diff_eix_only_ladadd =
diff_eix_only_src = diff-eix.cc
extra_diff_eix_only_src =

# Common to the previous two tools, but not common to all tools
output_only_ldadd =
output_only_src = $(output_src)
extra_output_only_src =

# The update-tool for our database
update_eix_only_ldadd = @SQLITE_LIBS@
update_eix_only_src = update-eix.cc $(cache_src) $(add_cache_src)
extra_update_eix_src = $(extra_cache_src)

# Remove files we don't want ..
CLEANFILES = cppcomplete.tags tags eix-sync eix-test-obsolete \
			 update-eix-remote update-eix-layman update-eix-functions.sh \
			 eixrc/eixrc \
			 cache/cache-map.cc \
			 diff-eix$(EXEEXT) update-eix$(EXEEXT)

EXTRA_DIST = eix-sync.in eix-test-obsolete.in update-eix-remote.in \
			 update-eix-layman.in update-eix-functions.sh.in \
			 eixrc/eixrc.in cache/generate-cachemap.sh

# Our binaries
bin_SCRIPTS  = eix-sync eix-test-obsolete update-eix-remote update-eix-layman \
			 update-eix-functions.sh

if SEPARATE_UPDATE

if SEPARATE_BINARIES

# Generate all three binaries separately:
bin_PROGRAMS = update-eix eix diff-eix
eix_LDADD = $(eix_only_ldadd) $(output_common_only_ldadd) $(common_ldadd)
eix_SOURCES = $(eix_only_src) $(output_only_src) $(common_src) \
	main/main_eix.cc
EXTRA_eix_SOURCES = $(extra_eix_src) $(extra_output_only_src) $(extra_common_src)
diff_eix_LDADD = $(diff_eix_only_ldadd) $(output_common_only_ldadd) $(common_ldadd)
diff_eix_SOURCES = $(diff_eix_only_src) $(output_only_src) $(common_src) \
	main/main_diff_eix.cc
EXTRA_diff_eix_SOURCES = $(extra_diff_eix_src) $(extra_output_only_src) $(extra_common_src)
update_eix_LDADD = $(update_eix_only_ldadd) $(common_ldadd)
update_eix_SOURCES = $(update_eix_only_src) $(common_src) \
	main/main_update_eix.cc
EXTRA_update_eix_SOURCES = $(extra_update_eix_src) $(extra_common_src)

else

# Generate only update-eix separately:
bin_PROGRAMS = update-eix eix
noinst_DATA = diff-eix$(EXEEXT)

diff-eix$(EXEEXT): eix$(EXEEXT)
	test ! -e diff-eix$(EXEEXT) || rm diff-eix$(EXEEXT)
	$(LN_S) eix$(EXEEXT) diff-eix$(EXEEXT)

uninstall-local:
	test ! -e $(DESTDIR)$(bindir)/diff-eix$(EXEEXT) || \
		rm -- $(bindir)/diff-eix$(EXEEXT)

install-exec-hook:
	test ! -e $(DESTDIR)$(bindir)/diff-eix$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/diff-eix$(EXEEXT)
	$(LN_S) eix$(EXEEXT) $(DESTDIR)$(bindir)/diff-eix$(EXEEXT)

eix_LDADD = $(eix_only_ldadd) $(diff_eix_only_ldadd) $(output_only_ldadd) $(common_ldadd)
eix_SOURCES = $(eix_only_src) $(diff_eix_only_src) $(output_only_src) $(common_src) \
	main/main_output.cc
EXTRA_eix_SOURCES = $(extra_eix_src) $(extra_diff_eix_src) $(extra_output_only_src) $(extra_common_src)
update_eix_LDADD = $(update_eix_only_ldadd) $(common_ldadd)
update_eix_SOURCES = $(update_eix_only_src) $(common_src) \
	main/main_update_eix.cc
EXTRA_update_eix_SOURCES = $(extra_cache_src) $(extra_common_src)

endif
else

# Generate only one binary:
bin_PROGRAMS = eix
noinst_DATA = diff-eix$(EXEEXT) update-eix$(EXEEXT)

diff-eix$(EXEEXT): eix$(EXEEXT)
	test ! -e diff-eix$(EXEEXT) || rm diff-eix$(EXEEXT)
	$(LN_S) eix$(EXEEXT) diff-eix$(EXEEXT)

update-eix$(EXEEXT): eix$(EXEEXT)
	test ! -e update-eix$(EXEEXT) || rm update-eix$(EXEEXT)
	$(LN_S) eix$(EXEEXT) update-eix$(EXEEXT)

uninstall-local:
	test ! -e $(DESTDIR)$(bindir)/diff-eix$(EXEEXT) || \
		rm -- $(bindir)/diff-eix$(EXEEXT)
	test ! -e $(DESTDIR)$(bindir)/update-eix$(EXEEXT) || \
		rm -- $(bindir)/update-eix$(EXEEXT)

install-exec-hook:
	test ! -e $(DESTDIR)$(bindir)/diff-eix$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/diff-eix$(EXEEXT)
	$(LN_S) eix$(EXEEXT) $(DESTDIR)$(bindir)/diff-eix$(EXEEXT)
	test ! -e $(DESTDIR)$(bindir)/update-eix$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/update-eix$(EXEEXT)
	$(LN_S) eix$(EXEEXT) $(DESTDIR)$(bindir)/update-eix$(EXEEXT)

eix_LDADD = $(eix_only_ldadd) $(diff_eix_only_ldadd) $(update_eix_only_ldadd) $(output_only_ldadd) $(common_ldadd)
eix_SOURCES = $(eix_only_src) $(diff_eix_only_src) $(update_eix_only_src) $(output_only_src) $(common_src) \
	main/main_all.cc
EXTRA_eix_SOURCES = $(extra_eix_src) $(extra_diff_eix_src) $(extra_update_eix_src) $(extra_output_only_src) $(extra_common_src)

endif


cache/cache-map.cc: cache/generate-cachemap.sh
	"$<" $(extra_cache) > "$@"

eix-sync: eix-sync.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
	    -e "s,\./update-eix-functions\.sh\.in,'$(bindir)/update-eix-functions.sh',g" \
	    -e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE),g" \
	"$<" > "$@"

eix-test-obsolete: eix-test-obsolete.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
	"$<" > "$@"

update-eix-functions.sh: update-eix-functions.sh.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
	"$<" > "$@"

update-eix-remote: update-eix-remote.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
	    -e "s,\./update-eix-functions\.sh\.in,'$(bindir)/update-eix-functions.sh',g" \
	"$<" > "$@"

update-eix-layman: update-eix-layman.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
	    -e "s,\./update-eix-functions\.sh\.in,'$(bindir)/update-eix-functions.sh',g" \
	"$<" > "$@"

eixrc/eixrc: eixrc/eixrc.in
	@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
	    -e "s,\@BINDIR\@,$(bindir),g" \
	    -e "s,\@SYSCONFDIR\@,$(sysconfdir),g" \
	"$<" > "$@"

# Enable us to 'make clean' in src/
clean-local:
	find "$(srcdir)" -name '*.rpo' -delete
