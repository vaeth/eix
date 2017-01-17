# This file is part of the eix project and distributed under the
# terms of the GNU General Public License v2.
#
#   Copyright (c)
#     Wolfgang Frisch <xororand@users.sourceforge.net>
#     Emil Beinroth <emilbeinroth@gmx.net>
#     Martin V\"ath <martin@mvath.de>

AUTOMAKE_OPTIONS = subdir-objects

localedir = $(datadir)/locale
AM_CXXFLAGS = \
-DSYSCONFDIR=\"$(sysconfdir)\" \
-DLOCALEDIR=\"$(localedir)\" \
$(SQLITE_CFLAGS)

nobase_nodist_sysconf_DATA = \
eixrc/00-eixrc

header_src = \
database/io.cc \
database/io.h \
database/io_header.cc \
database/header.cc \
database/header.h

database_src = \
$(header_src) \
database/header_portage.cc \
database/io_portage.cc \
database/package_reader.cc \
database/package_reader.h

nodist_database_src =

stringutils_src = \
eixTk/assert.h \
eixTk/compare.cc \
eixTk/compare.h \
eixTk/constexpr.h \
eixTk/diagnostics.h \
eixTk/eixint.h \
eixTk/formated.cc \
eixTk/formated.h \
eixTk/i18n.h \
eixTk/likely.h \
eixTk/null.h \
eixTk/stringtypes.h \
eixTk/stringutils.cc \
eixTk/stringutils.h

sysutils_src = \
eixTk/sysutils.cc \
eixTk/sysutils.h

utils_src = \
eixTk/filenames.cc \
eixTk/filenames.h \
eixTk/utils.cc \
eixTk/utils.h

varsreader_src = \
$(utils_src) \
eixTk/varsreader.cc \
eixTk/varsreader.h

masklist_src = \
eixTk/stringlist.cc \
eixTk/stringlist.h \
portage/mask.cc \
portage/mask.h \
portage/mask_list.cc \
portage/mask_list.h

eixtk_src = \
$(stringutils_src) \
eixTk/ansicolor.cc \
eixTk/ansicolor.h \
eixTk/argsreader.cc \
eixTk/argsreader.h \
eixTk/auto_list.h \
eixTk/delete.h \
eixTk/inttypes.h \
eixTk/parseerror.cc \
eixTk/parseerror.h \
eixTk/ptr_iterator.h \
eixTk/ptr_list.h \
eixTk/ptr_set.h \
eixTk/regexp.cc \
eixTk/regexp.h \
$(sysutils_src) \
eixTk/unused.h \
$(varsreader_src)

nodist_eixtk_src =

eixrc_src = \
eixrc/eixrc.cc \
eixrc/eixrc.h \
eixrc/global.cc \
eixrc/global.h \
eixrc/global1.cc \
eixrc/global2.cc \
eixrc/global3.cc \
eixrc/global4.cc \
eixrc/global5.cc \
eixrc/global6.cc \
eixrc/globals.h

nodist_eixrc_src =

depend_src = \
portage/depend.cc \
portage/depend.h

overlay_src = \
portage/overlay.cc \
portage/overlay.h

portage_src = \
portage/conf/portagesettings.cc \
portage/conf/portagesettings.h \
portage/conf/cascadingprofile.cc \
portage/conf/cascadingprofile.h \
$(masklist_src) \
$(depend_src) \
portage/basicversion.cc \
portage/basicversion.h \
portage/eapi.cc \
portage/eapi.h \
portage/extendedversion.cc \
portage/extendedversion_bin.cc \
portage/extendedversion.h \
portage/instversion.cc \
portage/instversion.h \
portage/package.cc \
portage/package_best.cc \
portage/package.h \
portage/packagesets.cc \
portage/packagesets.h \
portage/vardbpkg.cc \
portage/vardbpkg.h \
portage/packagetree.cc \
portage/packagetree.h \
portage/keywords.cc \
portage/keywords.h \
portage/overlay_bin.cc \
$(overlay_src) \
portage/set_stability.cc \
portage/set_stability.h \
portage/version.cc \
portage/version.h

nodist_portage_src =

cache_src = \
cache/cachetable.cc \
cache/cachetable.h \
cache/common/assign_reader.cc \
cache/common/assign_reader.h \
cache/common/ebuild_exec.cc \
cache/common/ebuild_exec.h \
cache/common/flat_reader.cc \
cache/common/flat_reader.h \
cache/common/reader.h \
cache/common/selectors.cc \
cache/common/selectors.h \
cache/base.cc \
cache/base.h \
cache/cache_map.h \
cache/eixcache/eixcache.cc \
cache/eixcache/eixcache.h \
cache/metadata/metadata.cc \
cache/metadata/metadata.h \
cache/parse/parse.cc \
cache/parse/parse.h \
cache/sqlite/sqlite.cc \
cache/sqlite/sqlite.h \
eixTk/md5.cc \
eixTk/md5.h

nodist_cache_src = \
cache/cache_map.cc

cli_src = \
various/cli.cc \
various/cli.h

nodist_cli_src =

drop_permissions_src = \
various/drop_permissions.cc \
various/drop_permissions.h

nodist_drop_permissions_src =

percentage_src = \
eixTk/percentage.cc \
eixTk/percentage.h \
eixTk/statusline.cc \
eixTk/statusline.h

nodist_percentage_src =

printxml_src = \
output/print-xml.cc \
output/print-xml.h

nodist_printxml_src =

outputstring_src = \
eixTk/outputstring.cc \
eixTk/outputstring.h \
eixTk/stringlist_output.cc

output_src = \
$(outputstring_src) \
portage/version_output.cc \
output/formatstring.cc \
output/formatstring.h \
output/formatstring-print.cc \
output/formatstring-print.h

nodist_output_src =

search_src = \
search/levenshtein.cc \
search/levenshtein.h \
search/algorithms.cc \
search/algorithms.h \
search/matchtree.cc \
search/matchtree.h \
search/packagetest.cc \
search/packagetest.h \
search/nowarn.cc \
search/nowarn.h \
search/redundancy.h

nodist_search_src =

# Common to all tools
common_tools_ldadd = \
$(LIBINTL)

# Common to all binaries which are not tools
common_ldadd = \
$(common_tools_ldadd)

common_src = \
main/main.h \
$(drop_permissions_src) \
$(eixtk_src) \
$(database_src) \
$(portage_src) \
$(eixrc_src) \
eixrc/eixrc_portage.cc

nodist_common_src = \
$(nodist_drop_permissions_src) \
$(nodist_eixtk_src) \
$(nodist_database_src) \
$(nodist_portage_src) \
$(nodist_eixrc_src)

extra_common_src = \
eixrc/def_i18n.cc \
eixrc/defaults.cc \
main/main.cc \
main/main_eix.cc \
main/main_diff.cc \
main/main_update.cc \
main/main_update_tools.cc \
main/main_output.cc \
main/main_overlay.cc \
main/main_all.cc \
main/main_all_tools.cc \
main/main_masked_packages.cc \
main/main_versionsort.cc \
main/main_drop_permissions.cc

nodist_extra_common_src =

# The search-tool for our database
eix_only_ldadd =
eix_only_src = eix.cc $(cli_src) $(printxml_src) $(search_src) eixTk/ansicolor_print.cc
nodist_eix_only_src = $(nodist_cli_src) $(nodist_printxml_src) $(nodist_search_src)
extra_eix_only_src =
nodist_extra_eix_only_src =

# The diff-tool for our database
diff_only_ldadd =
diff_only_src = eix-diff.cc
nodist_diff_only_src =
extra_diff_only_src =
nodist_extra_diff_only_src =

# Common to the previous two tools, but not common to all tools
output_only_ldadd =
output_only_src = $(output_src)
nodist_output_only_src = $(nodist_output_src)
extra_output_only_src =
nodist_extra_output_only_src =

# The update-tool for our database
update_only_ldadd = $(SQLITE_LIBS)
update_only_src = eix-update.cc $(percentage_src) $(cache_src)
nodist_update_only_src = $(nodist_cache_src) $(nodist_percentage_src) 
extra_update_only_src =
nodist_extra_update_only_src =

# Additional binaries (scripts) we will install
nodist_bin_SCRIPTS = \
eix-functions.sh \
eix-installed \
eix-installed-after \
eix-layman \
eix-remote \
eix-sync \
eix-test-obsolete

bin_SCRIPTS = \
eix-etcat

# Remove files we don't want ..
CLEANFILES = \
cppcomplete.tags \
tags \
$(nobase_nodist_sysconf_DATA) \
$(nodist_sysconf_DATA) \
$(nodist_bin_SCRIPTS) \
$(nodist_eix_only_src) \
$(nodist_extra_eix_only_src) \
$(nodist_diff_only_src) \
$(nodist_extra_diff_only_src) \
$(nodist_output_only_src) \
$(nodist_extra_output_only_src) \
$(nodist_update_only_src) \
$(nodist_extra_update_only_src) \
$(nodist_common_src) \
$(nodist_extra_common_src) \
eix-diff$(EXEEXT) \
eix-update$(EXEEXT) \
eix-header$(EXEEXT) \
eix-drop-permissions$(EXEEXT) \
masked-packages$(EXEEXT) \
versionsort$(EXEEXT) \
$(srcdir)/eix-installed.sh \
$(srcdir)/eix-installed-after.sh \
$(srcdir)/eix-layman.sh \
$(srcdir)/eix-remote.sh \
$(srcdir)/eix-sync.sh \
$(srcdir)/eix-etcat.sh \
$(srcdir)/eix-test-obsolete.sh

# Files we need for generation in the tarball which are not canonical
EXTRA_DIST = \
eix-functions.sh.in \
eix-installed.in \
eix-installed-after.in \
eix-layman.in \
eix-remote.in \
eix-sync.in \
eix-etcat \
eix-test-obsolete.in \
eixrc/00-eixrc.in \
cache/generate_cachemap.sh

if SEPARATE_TOOLS
# Generate separate tools binaries
use_separate_tools = :
tool_programs = masked-packages$(EXEEXT) versionsort$(EXEEXT) eix-header$(EXEEXT) eix-drop-permissions$(EXEEXT)
tool_noinst =
main_all_src  = main/main_all.cc
main_output_src = main/main_output.cc
main_eix_src = main/main_eix.cc
main_diff_src = main/main_diff.cc
main_update_src = main/main_update.cc
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
noinst_tool = masked-packages$(EXEEXT) versionsort$(EXEEXT) eix-drop-permissions$(EXEEXT) eix-header$(EXEEXT)
main_all_src = masked-packages.cc versionsort.cc eix-header.cc eix-drop-permissions.cc main/main_all_tools.cc
main_output_src = main/main_output.cc
main_eix_src = main/main_eix.cc
main_diff_src = main/main_diff.cc
main_update_src = masked-packages.cc versionsort.cc eix-header.cc eix-drop-permissions.cc main/main_update_tools.cc
endif

noinst_main_all_src =
noinst_main_output_src =
noinst_main_eix_src =
noinst_main_diff_src =
noinst_main_update_src =

# begin of big case distinction for generation of separate binaries
if SEPARATE_UPDATE

# We have a separate eix-update, so we link versionsort to it
use_separate_update = :
masked_link = eix-update$(EXEEXT)
versionsort_link = eix-update$(EXEEXT)
eix_header_link = eix-update$(EXEEXT)
drop_permissions_link = eix-update$(EXEEXT)

if SEPARATE_BINARIES

use_separate_diff = :

# Generate all three binaries separately:
bin_PROGRAMS = eix-update eix eix-diff $(tool_programs)
noinst_DATA = $(noinst_tool)

eix_LDADD = $(common_ldadd) $(eix_only_ldadd) $(output_common_only_ldadd)
eix_SOURCES = $(common_src) $(eix_only_src) $(output_only_src) $(main_eix_src)
nodist_eix_SOURCES = $(nodist_common_src) $(nodist_eix_only_src) $(nodist_output_only_src) $(nodist_main_eix_src)
EXTRA_eix_SOURCES = $(extra_eix_only_src) $(extra_output_only_src) $(extra_common_src)
nodist_EXTRA_eix_SOURCES = $(nodist_extra_eix_only_src) $(nodist_extra_output_only_src) $(nodist_extra_common_src)
eix_diff_LDADD = $(common_ldadd) $(diff_only_ldadd) $(output_common_only_ldadd)
eix_diff_SOURCES = $(common_src) $(diff_only_src) $(output_only_src) $(main_diff_src)
nodist_eix_diff_SOURCES = $(nodist_common_src) $(nodist_diff_only_src) $(nodist_output_only_src) $(nodist_main_diff_src)
EXTRA_eix_diff_SOURCES = $(extra_common_src) $(extra_diff_only_src) $(extra_output_only_src)
nodist_EXTRA_eix_diff_SOURCES = $(nodist_extra_common_src) $(nodist_extra_output_only_src)
eix_update_LDADD = $(common_ldadd) $(update_only_ldadd)
eix_update_SOURCES = $(common_src) $(update_only_src) $(main_update_src)
nodist_eix_update_SOURCES = $(nodist_update_only_src) $(nodist_output_only_src) $(nodist_main_update_src)
EXTRA_eix_update_SOURCES = $(extra_common_src) $(extra_update_only_src)
nodist_EXTRA_eix_update_SOURCES = $(nodist_extra_common_src) $(nodist_extra_update_only_src)

else
# !SEPARATE_BINARIES

use_separate_diff = false

# Generate only eix-update separately:
bin_PROGRAMS = eix-update eix $(tool_programs)
noinst_DATA = eix-diff$(EXEEXT) $(noinst_tool)

eix-diff$(EXEEXT): eix$(EXEEXT)
	$(AM_V_at)test ! -e eix-diff$(EXEEXT) || rm eix-diff$(EXEEXT)
	$(AM_V_GEN)$(LN_S) eix$(EXEEXT) eix-diff$(EXEEXT)

eix_LDADD = $(common_ldadd) $(eix_only_ldadd) $(diff_only_ldadd) $(output_only_ldadd)
eix_SOURCES = $(common_src) $(eix_only_src) $(diff_only_src) $(output_only_src) $(main_output_src)
nodist_eix_SOURCES = $(nodist_common_src) $(nodist_eix_only_src) $(nodist_diff_only_src) $(nodist_output_only_src) $(nodist_main_output_src)
EXTRA_eix_SOURCES = $(extra_common_src) $(extra_eix_only_src) $(extra_diff_only_src) $(extra_output_only_src)
nodist_EXTRA_eix_SOURCES = $(nodist_extra_common_src) $(nodist_extra_eix_only_src) $(nodist_extra_diff_only_src) $(nodist_extra_output_only_src)
eix_update_LDADD = $(common_ldadd) $(update_only_ldadd)
eix_update_SOURCES = $(common_src) $(update_only_src) $(main_update_src)
nodist_eix_update_SOURCES = $(nodist_common_src) $(nodist_update_only_src) $(nodist_main_update_src)
EXTRA_eix_update_SOURCES = $(extra_common_src) $(extra_update_only_src)
nodist_EXTRA_eix_update_SOURCES = $(nodist_extra_common_src) $(nodist_extra_update_only_src)

endif

else
# !SEPARATE_UPDATE

# We do not have a separate eix-update, so we link tools to eix
use_separate_update = false
masked_link = eix$(EXEEXT)
versionsort_link = eix$(EXEEXT)
eix_header_link = eix$(EXEEXT)
drop_permissions_link = eix$(EXEEXT)

# Generate neither eix-diff nor eix-update separately:
use_separate_diff = false
bin_PROGRAMS = eix $(tool_programs)
noinst_DATA = eix-diff$(EXEEXT) eix-update$(EXEEXT) $(noinst_tool)

eix-diff$(EXEEXT): eix$(EXEEXT)
	$(AM_V_at)test ! -e eix-diff$(EXEEXT) || rm eix-diff$(EXEEXT)
	$(AM_V_GEN)$(LN_S) eix$(EXEEXT) eix-diff$(EXEEXT)

eix-update$(EXEEXT): eix$(EXEEXT)
	$(AM_V_at)test ! -e eix-update$(EXEEXT) || rm eix-update$(EXEEXT)
	$(AM_V_GEN)$(LN_S) eix$(EXEEXT) eix-update$(EXEEXT)


eix_LDADD = $(common_ldadd) $(eix_only_ldadd) $(diff_only_ldadd) $(update_only_ldadd) $(output_only_ldadd)
eix_SOURCES = $(common_src) $(output_only_src) $(eix_only_src) $(diff_only_src) $(update_only_src) $(main_all_src)
nodist_eix_SOURCES = $(nodist_update_only_src) $(nodist_common_src) $(nodist_output_only_src) $(nodist_eix_only_src) $(nodist_diff_only_src) $(nodist_main_all_src)
EXTRA_eix_SOURCES = $(extra_common_src) $(extra_output_only_src) $(extra_eix_only_src) $(extra_diff_only_src) $(extra_update_only_src)
nodist_EXTRA_eix_SOURCES = $(nodist_extra_common_src) $(nodist_extra_output_only_src) $(nodist_extra_eix_only_src) $(nodist_extra_diff_only_src) $(nodist_extra_update_only_src)

endif
# end of big case distinction for generation of separate binaries

if SEPARATE_TOOLS

masked_packages_LDADD = $(common_tools_ldadd)
masked_packages_SOURCES = \
eixTk/parseerror.cc \
eixTk/parseerror.h \
$(stringutils_src) \
eixTk/utils.cc \
portage/basicversion.cc \
portage/extendedversion.cc \
portage/keywords.cc \
portage/version.cc \
portage/package.cc \
$(masklist_src) \
eixTk/argsreader.cc \
masked-packages.cc \
main/main_masked_packages.cc

versionsort_LDADD = $(common_tools_ldadd)
versionsort_SOURCES = \
$(stringutils_src) \
portage/basicversion.cc \
versionsort.cc \
main/main_versionsort.cc

eix_header_LDADD = $(common_tools_ldadd)
eix_header_SOURCES = \
$(stringutils_src) \
$(utils_src) \
$(header_src) \
$(depend_src) \
$(overlay_src) \
$(sysutils_src) \
eix-header.cc \
main/main_overlay.cc

eix_drop_permissions_LDADD = $(common_tools_ldadd)
eix_drop_permissions_SOURCES = \
$(stringutils_src) \
$(eixrc_src) \
$(drop_permissions_src) \
$(sysutils_src) \
$(varsreader_src) \
eix-drop-permissions.cc \
main/main_drop_permissions.cc

else
# !SEPARATE_TOOLS

masked-packages$(EXEEXT): $(masked_link)
	$(AM_V_at)test ! -e masked-packages$(EXEEXT) || rm masked-packages$(EXEEXT)
	$(AM_V_GEN)$(LN_S) $(masked_link) masked-packages$(EXEEXT)

versionsort$(EXEEXT): $(versionsort_link)
	$(AM_V_at)test ! -e versionsort$(EXEEXT) || rm versionsort$(EXEEXT)
	$(AM_V_GEN)$(LN_S) $(versionsort_link) versionsort$(EXEEXT)

eix-header$(EXEEXT): $(eix_header_link)
	$(AM_V_at)test ! -e eix-header$(EXEEXT) || rm eix-header$(EXEEXT)
	$(AM_V_GEN)$(LN_S) $(eix_header_link) eix-header$(EXEEXT)

eix-drop-permissions$(EXEEXT): $(drop_permissions_link)
	$(AM_V_at)test ! -e eix-drop-permissions$(EXEEXT) || rm eix-drop-permissions$(EXEEXT)
	$(AM_V_GEN)$(LN_S) $(drop_permissions_link) eix-drop-permissions$(EXEEXT)

endif

eixrc/global1.cc: eixrc/defaults.cc
eixrc/global2.cc: eixrc/defaults.cc
eixrc/global3.cc: eixrc/defaults.cc
eixrc/global4.cc: eixrc/defaults.cc
eixrc/global5.cc: eixrc/defaults.cc
eixrc/global6.cc: eixrc/defaults.cc eixrc/def_i18n.cc

uninstall-local:
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/masked-packages$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/masked-packages$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/versionsort$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/versionsort$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-header$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-header$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-drop-permissions$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-drop-permissions$(EXEEXT)"
	$(AM_V_at)$(use_separate_update) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-update$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-update$(EXEEXT)"
	$(AM_V_at)$(use_separate_diff) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-diff$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-diff$(EXEEXT)"

install-exec-hook:
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/masked-packages$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/masked-packages$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		$(LN_S) "$(masked_link)" "$(DESTDIR)$(bindir)/masked-packages$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/versionsort$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/versionsort$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		$(LN_S) "$(versionsort_link)" "$(DESTDIR)$(bindir)/versionsort$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-header$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-header$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		$(LN_S) "$(eix_header_link)" "$(DESTDIR)$(bindir)/eix-header$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-drop-permissions$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-drop-permissions$(EXEEXT)"
	$(AM_V_at)$(use_separate_tools) || \
		$(LN_S) "$(drop_permissions_link)" "$(DESTDIR)$(bindir)/eix-drop-permissions$(EXEEXT)"
	$(AM_V_at)$(use_separate_update) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-update$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-update$(EXEEXT)"
	$(AM_V_at)$(use_separate_update) || \
		$(LN_S) "eix$(EXEEXT)" "$(DESTDIR)$(bindir)/eix-update$(EXEEXT)"
	$(AM_V_at)$(use_separate_diff) || \
		test ! -e "$(DESTDIR)$(bindir)/eix-diff$(EXEEXT)" || \
		rm -- "$(DESTDIR)$(bindir)/eix-diff$(EXEEXT)"
	$(AM_V_at)$(use_separate_diff) || \
		$(LN_S) "eix$(EXEEXT)" "$(DESTDIR)$(bindir)/eix-diff$(EXEEXT)"

cache/cache_map.cc: $(srcdir)/cache/generate_cachemap.sh
	$(AM_V_GEN)"$(srcdir)/cache/generate_cachemap.sh" > "$@"

eix-sync.sh:
	$(AM_V_GEN)$(LN_S) eix-sync.in "$(srcdir)/$@"

eix-test-obsolete.sh:
	$(AM_V_GEN)$(LN_S) eix-test-obsolete.in "$(srcdir)/$@"

eix-layman.sh:
	$(AM_V_GEN)$(LN_S) eix-layman.in "$(srcdir)/$@"

eix-installed.sh:
	$(AM_V_GEN)$(LN_S) eix-installed.in "$(srcdir)/$@"

eix-installed-after.sh:
	$(AM_V_GEN)$(LN_S) eix-installed-after.in "$(srcdir)/$@"

eix-remote.sh:
	$(AM_V_GEN)$(LN_S) eix-remote.in "$(srcdir)/$@"

eix-etcat.sh:
	$(AM_V_GEN)$(LN_S) eix-etcat "$(srcdir)/$@"

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
		-e "s,\@LEVENSHTEIN_DISTANCE\@,$(LEVENSHTEIN_DISTANCE),g" \
		-e "s,\@PORTDIR_CACHE_METHOD\@,$(PORTDIR_CACHE_METHOD),g" \
		-e "s,\@BINDIR\@,$(bindir),g" \
		-e "s,\@SYSCONFDIR\@,$(sysconfdir),g" \
		-e "s,\@DEFAULT_REMOTE_PATH1\@,$(DEFAULT_REMOTE_PATH1),g" \
		-e "s,\@DEFAULT_REMOTE_PATH2\@,$(DEFAULT_REMOTE_PATH2),g" \
		-e "s,/dev/null,$(DEV_NULL),g" \
		-e "s,\./eix-functions\.sh\.in,'$(bindir)/eix-functions.sh',g" \
		-e "s,\./eix-drop-permissions,'$(bindir)/eix-drop-permissions',g" \
		-e "s,\./eix-remote.in,'$(bindir)/eix-remote',g" \
		-e 's,^#!/usr/bin/env sh$$,#!$(SH_SHEBANG),g' \
		-e "s,\@TEXTDOMAIN\@,$(PACKAGE),g" \
		-e "s,\@TEXTDOMAINDIR\@,$(localedir),g" \
		-e "s,\@USE_NLS\@,$(USE_NLS),g" \
		"$<" > "$@"

# Enable us to 'make clean' in src/
clean-local:
	$(AM_V_at)find "$(top_srcdir)" -name '*.rpo' -exec $(RM) -v -- '{}' '+'
