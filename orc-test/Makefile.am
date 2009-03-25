
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc-test

lib_LTLIBRARIES = liborc-test-@ORC_MAJORMINOR@.la

liborc_test_@ORC_MAJORMINOR@_la_LIBS = $(ORC_LIBS)
liborc_test_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined -export-symbols-regex 'orc_'
liborc_test_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS)

liborc_test_@ORC_MAJORMINOR@_la_SOURCES = \
	orctest.c

pkginclude_HEADERS = \
	orctest.h

