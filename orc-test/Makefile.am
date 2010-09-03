
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc-test

lib_LTLIBRARIES = liborc-test-@ORC_MAJORMINOR@.la

liborc_test_@ORC_MAJORMINOR@_la_LIBADD = $(ORC_LIBS)
liborc_test_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-version-info $(ORC_LIBVERSION) \
	-no-undefined -export-symbols-regex 'orc_'
liborc_test_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS) \
	-DORC_ENABLE_UNSTABLE_API

liborc_test_@ORC_MAJORMINOR@_la_SOURCES = \
	orctest.c \
	orcarray.c \
	orcrandom.c \
	orcprofile.c

pkginclude_HEADERS = \
	orctest.h \
	orcarray.h \
	orcrandom.h \
	orcprofile.h

