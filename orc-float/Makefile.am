
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc-float

lib_LTLIBRARIES = liborc-float-@ORC_MAJORMINOR@.la

liborc_float_@ORC_MAJORMINOR@_la_LIBADD = $(ORC_LIBS)
liborc_float_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined -export-symbols-regex 'orc_'
liborc_float_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS)

liborc_float_@ORC_MAJORMINOR@_la_SOURCES = \
	orcfloat.c orcfloat-sse.c \
	orcfloat-neon.c

pkginclude_HEADERS = \
	orcfloat.h

