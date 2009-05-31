
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc

lib_LTLIBRARIES = liborc-@ORC_MAJORMINOR@.la

liborc_@ORC_MAJORMINOR@_la_LIBS = $(ORC_LIBS)
liborc_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined -export-symbols-regex 'orc_'
liborc_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS) \
	-DORC_ENABLE_UNSTABLE_API

liborc_@ORC_MAJORMINOR@_la_SOURCES = \
	orc.c \
	orcexecutor.c \
	orcrule.c \
	orccodemem.c \
	orcprogram.c \
	orccompiler.c \
	orcprogram-arm.c \
	orcprogram-c.c \
	orcprogram-altivec.c \
	orcprogram-mmx.c \
	orcprogram-sse.c \
	orcprogram.h \
	orcopcodes.c \
	orcparse.c \
	orcrules-altivec.c \
	orcrules-mmx.c \
	orcrules-sse.c \
	orcrules-arm.c \
	orcprogram-neon.c \
	orcrules-neon.c \
	orcprogram-c64x.c \
	orcrules-c64x.c \
	orcdebug.c \
	orcutils.c \
	orcpowerpc.c \
	orcsse.c \
	orcmmx.c \
	orcc64x.c \
	orcx86.c \
	orcarm.c

if HAVE_I386
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-x86.c
endif
if HAVE_AMD64
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-x86.c
endif
if HAVE_POWERPC
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-powerpc.c
endif
if HAVE_ARM
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-arm.c
endif

pkginclude_HEADERS = \
	orc.h \
	orcdebug.h \
	orcprogram.h \
	orcutils.h \
	orcparse.h \
	opcodes.h \
	orcsse.h \
	orcmmx.h \
	orcx86.h \
	orcc64x.h \
	orcpowerpc.h

nodist_pkginclude_HEADERS = orc-stdint.h

DISTCLEANFILES = orc-stdint.h

