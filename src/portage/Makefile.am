CLEANFILES = cppcomplete.tags tags

SUBDIRS = cache conf

INCLUDES = -I$(top_srcdir)/src

noinst_LIBRARIES = libportage.a

libportage_a_SOURCES = basicversion.cc basicversion.h \
					   mask.cc mask.h \
					   mask_list.h \
					   version.h \
					   package.cc package.h \
					   keywords.cc keywords.h \
					   vardbpkg.cc vardbpkg.h \
					   packagetree.cc packagetree.h

TESTS = basicversion-test keywords-test

bin_PROGRAMS = basicversion-test keywords-test

basicversion_test_SOURCES = basicversion_test.cc basicversion.cc

keywords_test_SOURCES = keywords_test.cc keywords.cc
keywords_test_LDADD = ../eixTk/libeixTk.a
