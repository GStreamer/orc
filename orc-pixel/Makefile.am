
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc-pixel

lib_LTLIBRARIES = liborc-pixel-@ORC_MAJORMINOR@.la

liborc_pixel_@ORC_MAJORMINOR@_la_LIBADD = $(ORC_LIBS)
liborc_pixel_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined -export-symbols-regex 'orc_'
liborc_pixel_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS)

liborc_pixel_@ORC_MAJORMINOR@_la_SOURCES = \
	orcpixel.c orcpixel-sse.c

pkginclude_HEADERS = \
	orcpixel.h

