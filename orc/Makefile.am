
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc

lib_LTLIBRARIES = liborc-@ORC_MAJORMINOR@.la

liborc_@ORC_MAJORMINOR@_la_LIBS = $(ORC_LIBS)
liborc_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS)

liborc_@ORC_MAJORMINOR@_la_SOURCES = \
	orc.c \
	orcexecutor.c \
	orcrule.c \
	orctype.c \
	orcprogram.c \
	orcprogram-c.c \
	orcprogram-powerpc.c \
	orcprogram-x86.c \
	orcprogram.h \
	orcopcodes.c \
	orcprogram-linux.c \
	orcrules-mmx.c \
	orcrules-sse.c \
	x86.c

pkginclude_HEADERS = \
	orc.h \
	orcprogram.h \
	x86.h

nodist_pkginclude_HEADERS = orc-stdint.h

DISTCLEANFILES = orc-stdint.h

