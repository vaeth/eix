CLEANFILES = cppcomplete.tags tags cache-map.cc

INCLUDES = -I$(top_srcdir)/src

noinst_LIBRARIES = libcache.a

libcache_a_SOURCES = base.h \
					 flat/flat.cc flat/flat.h \
					 flat/flat-utils.cc flat/flat-utils.h \
					 none/none.cc none/none.h \
					 cdb/cdb.h cdb/cdb.cc \
					 backport/backport.cc backport/backport.h \
					 backport/backport-utils.cc backport/backport-utils.h \
					 cache-map.cc

cache-map.cc:
	$(srcdir)/generate-cachemap.sh > cache-map.cc
