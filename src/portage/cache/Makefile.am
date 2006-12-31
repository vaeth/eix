CLEANFILES = cppcomplete.tags tags cache-map.cc

EXTRA_DIST = generate-cachemap.sh

INCLUDES = -I$(top_srcdir)/src

noinst_LIBRARIES = libcache.a


#				 port2_1_2/port2_1_2.cc port2_1_2/port2_1_2.h
libcache_a_SOURCES = base.h base.cc \
				 port2_0_0/port2_0_0.cc port2_0_0/port2_0_0.h \
				 port2_1_0/port2_1_0.cc port2_1_0/port2_1_0.h \
				 cache-utils/flat-reader.cc cache-utils/flat-reader.h \
				 cache-utils/selectors.cc cache-utils/selectors.h \
				 cache-utils/unpickle.cc cache-utils/unpickle.h \
				 none/none.cc none/none.h \
				 ebuild/ebuild.cc ebuild/ebuild.h \
				 eixcache/eixcache.cc eixcache/eixcache.h \
				 cdb/cdb.h cdb/cdb.cc \
				 sqlite/sqlite.h sqlite/sqlite.cc \
				 metadata/metadata.cc metadata/metadata.h \
				 cache-map.cc cache-map.h

cache-map.cc: $(srcdir)/generate-cachemap.sh
	$(srcdir)/generate-cachemap.sh > cache-map.cc
