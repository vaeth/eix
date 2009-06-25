# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

AUTOMAKE_OPTIONS = subdir-objects

localedir = $(datadir)/locale
AM_CXXFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" -DLOCALEDIR=\"$(localedir)\"

sysconf_DATA = \
eixrc/eixrc \
eixrc/eix-sync.conf \
eixrc/eix-remote.conf

database_src = \
database/io.cc \
database/io.h \
database/types.h \
database/header.cc \
database/header.h \
database/package_reader.cc \
database/package_reader.h

eixtk_src = \
eixTk/ansicolor.cc \
eixTk/ansicolor.h \
eixTk/argsreader.cc \
eixTk/argsreader.h \
eixTk/auto_ptr.h \
eixTk/compare.h \
eixTk/exceptions.cc \
eixTk/exceptions.h \
eixTk/filenames.cc \
eixTk/filenames.h \
eixTk/formated.cc \
eixTk/formated.h \
eixTk/i18n.h \
eixTk/inttypes.h \
eixTk/ptr_list.h \
eixTk/regexp.h \
eixTk/stringutils.cc \
eixTk/stringutils.h \
eixTk/sysutils.cc \
eixTk/sysutils.h \
eixTk/utils.cc \
eixTk/utils.h \
eixTk/varsreader.cc \
eixTk/varsreader.h

eixrc_src = \
eixrc/eixrc.cc \
eixrc/eixrc.h \
eixrc/global1.cc \
eixrc/global2.cc \
eixrc/global3.cc \
eixrc/global4.cc \
eixrc/global.h

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
portage/packagesets.h \
portage/vardbpkg.cc \
portage/vardbpkg.h \
portage/packagetree.cc \
portage/packagetree.h \
portage/keywords.cc \
portage/keywords.h \
portage/mask_list.h \
portage/overlay.cc \
portage/overlay.h \
portage/set_stability.cc \
portage/set_stability.h \
portage/version.cc \
portage/version.h

cache_src = \
cache/cachetable.cc \
cache/cachetable.h \
cache/common/assign_reader.cc \
cache/common/assign_reader.h \
cache/common/flat_reader.cc \
cache/common/flat_reader.h \
cache/common/ebuild_exec.cc \
cache/common/ebuild_exec.h \
cache/common/selectors.cc \
cache/common/selectors.h \
cache/common/unpickle.cc \
cache/common/unpickle.h \
cache/base.cc \
cache/base.h \
cache/cache_map.cc \
cache/cache_map.h \
cache/cdb/cdb.cc \
cache/cdb/cdb.h \
cache/eixcache/eixcache.cc \
cache/eixcache/eixcache.h \
cache/metadata/metadata.cc \
cache/metadata/metadata.h \
cache/parse/parse.cc \
cache/parse/parse.h \
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

search_src = \
search/levenshtein.cc \
search/levenshtein.h \
search/algorithms.cc \
search/algorithms.h \
search/dbmatchcriteria.h \
search/packagetest.cc \
search/packagetest.h \
search/redundancy.h

# Common to all binaries:
common_ldadd = @BZLIB_LIBS@ $(LTLIBINTL)
common_src = \
main/main.h \
$(database_src) \
$(portage_src) \
$(eixtk_src) \
$(eixrc_src)

extra_common_src = \
eixrc/defaults.cc \
main/main.cc \
main/main_eix.cc \
main/main_diff.cc \
main/main_update.cc \
main/main_update_tools.cc \
main/main_output.cc \
main/main_all.cc \
main/main_all_tools.cc \
main/main_tools.cc

# The search-tool for our database
eix_only_ldadd =
eix_only_src = eix.cc cli.cc cli.h $(search_src)
extra_eix_only_src =

# The diff-tool for our database
diff_only_ldadd =
diff_only_src = eix-diff.cc
extra_diff_only_src =

# Common to the previous two tools, but not common to all tools
output_only_ldadd =
output_only_src = $(output_src)
extra_output_only_src =

# The update-tool for our database
update_only_ldadd = @SQLITE_LIBS@
update_only_src = eix-update.cc $(cache_src) $(add_cache_src)
extra_update_src = $(extra_cache_src)

# Remove files we don't want ..
CLEANFILES = \
cppcomplete.tags \
tags \
eix-sync \
eix-test-obsolete \
eix-remote \
eix-layman \
eix-functions.sh \
$(sysconf_DATA) \
cache/cache_map.cc \
eix-diff$(EXEEXT) \
eix-update$(EXEEXT) \
versionsort$(EXEEXT) \
eix-sync.sh \
eix-test-obsolete.sh \
eix-layman.sh \
eix-remote.sh

# Files we need for generation in the tarball which are not canonical
EXTRA_DIST = \
eix-sync.in \
eix-test-obsolete.in \
eix-remote.in \
eix-layman.in \
eix-functions.sh.in \
eixrc/eixrc.in \
eixrc/eix-sync.conf.in \
eixrc/eix-remote.conf.in \
cache/generate_cachemap.sh

# Additional binaries (scripts) we will install
bin_SCRIPTS  = \
eix-sync \
eix-test-obsolete \
eix-remote \
eix-layman \
eix-functions.sh

if SEPARATE_TOOLS
# Generate a separate tools binary (currently only versionsort)
use_separate_tools = true
tool_programs = versionsort
tool_noinst =
main_all = main/main_all.cc
main_output = main/main_output.cc
main_eix = main/main_eix.cc
main_diff = main/main_diff.cc
main_update = main/main_update.cc
else
# Do not generate a separate tools binary
# Currently, we link versionsort against eix-update.
# Once there will be further tools which are perhaps better linked again
# eix or eix-diff, it will also be necessary to replace
# main_eix.cc main_diff.cc main_output.cc by corresponding
# main_eix_tools.cc main_diff_tools.cc main_output_tools.cc
# (which should essentially be copies of the above and just #define something:
# main/main.cc should then react on this to a modified argv[0] filename).
use_separate_tools = false
tool_programs =
tool_noinst = versionsort$(EXEEXT)
main_all = versionsort.cc main/main_all_tools.cc
main_output = main/main_output.cc
main_eix = main/main_eix.cc
main_diff = main/main_diff.cc
main_update = versionsort.cc main/main_update_tools.cc
endif

# begin of big case distinction for generation of separate binaries
if SEPARATE_UPDATE

# We have a separate eix-update, so we link versionsort to it
use_separate_update = true
versionsort_link = eix-update$(EXEEXT)

if SEPARATE_BINARIES

use_separate_diff = true

# Generate all three binaries separately:
bin_PROGRAMS = eix-update eix eix-diff $(tool_programs)
noinst_DATA = $(tool_noinst)

eix_LDADD = $(eix_only_ldadd) $(output_common_only_ldadd) $(common_ldadd)
eix_SOURCES = $(eix_only_src) $(output_only_src) $(common_src) $(main_eix)
EXTRA_eix_SOURCES = $(extra_eix_src) $(extra_output_only_src) $(extra_common_src)
eix_diff_LDADD = $(diff_only_ldadd) $(output_common_only_ldadd) $(common_ldadd)
eix_diff_SOURCES = $(diff_only_src) $(output_only_src) $(common_src) $(main_diff)
EXTRA_eix_diff_SOURCES = $(extra_eix_diff_src) $(extra_output_only_src) $(extra_common_src)
eix_update_LDADD = $(update_only_ldadd) $(common_ldadd)
eix_update_SOURCES = $(update_only_src) $(common_src) $(main_update)
EXTRA_eix_update_SOURCES = $(extra_update_src) $(extra_common_src)

else
# !SEPARATE_BINARIES

use_separate_diff = false

# Generate only eix-update separately:
bin_PROGRAMS = eix-update eix $(tool_programs)
noinst_DATA = eix-diff$(EXEEXT) $(tool_noinst)

eix-diff$(EXEEXT): eix$(EXEEXT)
	@echo "  LN    " "$@"
	@test ! -e eix-diff$(EXEEXT) || rm eix-diff$(EXEEXT)
	@$(LN_S) eix$(EXEEXT) eix-diff$(EXEEXT)

eix_LDADD = $(eix_only_ldadd) $(diff_only_ldadd) $(output_only_ldadd) $(common_ldadd)
eix_SOURCES = $(eix_only_src) $(diff_only_src) $(output_only_src) $(common_src) $(main_output)
EXTRA_eix_SOURCES = $(extra_eix_src) $(extra_eix_diff_src) $(extra_output_only_src) $(extra_common_src)
eix_update_LDADD = $(update_only_ldadd) $(common_ldadd)
eix_update_SOURCES = $(update_only_src) $(common_src) $(main_update)
EXTRA_eix_update_SOURCES = $(extra_update_src) $(extra_common_src)

endif

else
# !SEPARATE_UPDATE

# We do not have a separate eix-update, so we link versionsort to eix
use_separate_update = false
versionsort_link = eix$(EXEEXT)

# Generate neither eix-diff nor eix-update separately:
use_separate_diff = false
bin_PROGRAMS = eix $(tool_programs)
noinst_DATA = eix-diff$(EXEEXT) eix-update$(EXEEXT) $(tool_noinst)

eix-diff$(EXEEXT): eix$(EXEEXT)
	@echo "  LN    " "$@"
	@test ! -e eix-diff$(EXEEXT) || rm eix-diff$(EXEEXT)
	@$(LN_S) eix$(EXEEXT) eix-diff$(EXEEXT)

eix-update$(EXEEXT): eix$(EXEEXT)
	@echo "  LN    " "$@"
	@test ! -e eix-update$(EXEEXT) || rm eix-update$(EXEEXT)
	@$(LN_S) eix$(EXEEXT) eix-update$(EXEEXT)


eix_LDADD = $(eix_only_ldadd) $(diff_only_ldadd) $(update_only_ldadd) $(output_only_ldadd) $(common_ldadd)
eix_SOURCES = $(common_src) $(output_only_src) $(eix_only_src) $(diff_only_src) $(update_only_src) $(main_all)
EXTRA_eix_SOURCES = $(extra_common_src) $(extra_output_only_src) $(extra_eix_src) $(extra_eix_diff_src) $(extra_update_src)

endif
# end of big case distinction for generation of separate binaries

if SEPARATE_TOOLS

versionsort_SOURCES = \
versionsort.cc \
main/main_tools.cc \
main/main.h \
database/types.h \
eixTk/inttypes.h \
eixTk/stringutils.cc \
eixTk/stringutils.h \
eixTk/compare.h \
eixTk/exceptions.h \
eixTk/formated.cc \
eixTk/formated.h \
portage/basicversion.cc \
portage/basicversion.h

else
# !SEPARATE_TOOLS

versionsort$(EXEEXT): $(versionsort_link)
	@test ! -e versionsort$(EXEEXT) || rm versionsort$(EXEEXT)
	@$(LN_S) $(versionsort_link) versionsort$(EXEEXT)

endif

uninstall-local:
	@$(use_separate_tools) || \
		test ! -e $(DESTDIR)$(bindir)/versionsort$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/versionsort$(EXEEXT)
	@$(use_separate_update) || \
		test ! -e $(DESTDIR)$(bindir)/eix-update$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/eix-update$(EXEEXT)
	@$(use_separate_diff) || \
		test ! -e $(DESTDIR)$(bindir)/eix-diff$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/eix-diff$(EXEEXT)

install-exec-hook:
	@$(use_separate_tools) || \
		test ! -e $(DESTDIR)$(bindir)/versionsort$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/versionsort$(EXEEXT)
	@$(use_separate_tools) || \
		$(LN_S) $(versionsort_link) $(DESTDIR)$(bindir)/versionsort$(EXEEXT)
	@$(use_separate_update) || \
		test ! -e $(DESTDIR)$(bindir)/eix-update$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/eix-update$(EXEEXT)
	@$(use_separate_update) || \
		$(LN_S) eix$(EXEEXT) $(DESTDIR)$(bindir)/eix-update$(EXEEXT)
	@$(use_separate_diff) || \
		test ! -e $(DESTDIR)$(bindir)/eix-diff$(EXEEXT) || \
		rm -- $(DESTDIR)$(bindir)/eix-diff$(EXEEXT)
	@$(use_separate_diff) || \
		$(LN_S) eix$(EXEEXT) $(DESTDIR)$(bindir)/eix-diff$(EXEEXT)

cache/cache_map.cc: cache/generate_cachemap.sh
	@echo "  SH    " "$@"
	@"cache/generate_cachemap.sh" $(extra_cache) > "$@"

eix-sync.sh:
	@echo "  LN    " "$@"
	@$(LN_S) eix-sync.in "$@"

eix-test-obsolete.sh:
	@echo "  LN    " "$@"
	@$(LN_S) eix-test-obsolete.in "$@"

eix-layman.sh:
	@echo "  LN    " "$@"
	@$(LN_S) eix-layman.in "$@"

eix-remote.sh:
	@echo "  LN    " "$@"
	@$(LN_S) eix-remote.in "$@"


SUFFIXES = .in
.in:
	@echo "  SED   " "$@"
	@@regex_cmd@ -e "s,\@PACKAGE_STRING\@,$(PACKAGE_STRING),g" \
		-e "s,\./eix-functions\.sh\.in,'$(bindir)/eix-functions.sh',g" \
		-e "s,\@EIX_CACHEFILE\@,$(EIX_CACHEFILE),g" \
		-e "s,\@BINDIR\@,$(bindir),g" \
		-e "s,\@SYSCONFDIR\@,$(sysconfdir),g" \
		-e "s,\@TEXTDOMAIN\@,$(PACKAGE),g" \
		-e "s,\@TEXTDOMAINDIR\@,$(localedir),g" \
		-e "s,\@USE_NLS\@,$(USE_NLS),g" \
		"$<" > "$@"

# Enable us to 'make clean' in src/
clean-local:
	@find "$(top_srcdir)" -name '*.rpo' -exec $(RM) -v -- '{}' '+'
