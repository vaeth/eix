CLEANFILES = cppcomplete.tags tags cache-map.cc

EXTRA_DIST = generate-cachemap.sh

INCLUDES = -I$(top_srcdir)/src

noinst_LIBRARIES = libcache.a

libcache_a_SOURCES = base.h base.cc \
					 flat/flat.cc flat/flat.h \
					 flat/flat-utils.cc flat/flat-utils.h \
					 none/none.cc none/none.h \
					 cdb/cdb.h cdb/cdb.cc \
					 backport/backport.cc backport/backport.h \
					 backport/backport-utils.cc backport/backport-utils.h \
					 metadata/metadata-utils.cc metadata/metadata-utils.h \
					 metadata/metadata.cc metadata/metadata.h \
					 cache-map.cc cache-map.h

cache-map.cc:
	$(srcdir)/generate-cachemap.sh > cache-map.cc
