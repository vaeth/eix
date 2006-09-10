CLEANFILES = cppcomplete.tags tags cache-map.cc

EXTRA_DIST = generate-cachemap.sh

INCLUDES = -I$(top_srcdir)/src

noinst_LIBRARIES = libcache.a

libcache_a_SOURCES = base.h base.cc \
					 flat/flat.cc flat/flat.h \
					 cache-utils.cc cache-utils.h \
					 none/none.cc none/none.h \
					 ebuild/ebuild.cc ebuild/ebuild.h \
					 eixcache/eixcache.cc eixcache/eixcache.h \
					 cdb/cdb.h cdb/cdb.cc \
					 sqlite/sqlite.h sqlite/sqlite.cc \
					 backport/backport.cc backport/backport.h \
					 metadata/metadata.cc metadata/metadata.h \
					 cache-map.cc cache-map.h

cache-map.cc:
	$(srcdir)/generate-cachemap.sh > cache-map.cc
