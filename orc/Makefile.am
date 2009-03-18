
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc

lib_LTLIBRARIES = liborc-@ORC_MAJORMINOR@.la

liborc_@ORC_MAJORMINOR@_la_LIBS = $(ORC_LIBS)
liborc_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined -export-symbols-regex 'orc_'
liborc_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS)

liborc_@ORC_MAJORMINOR@_la_SOURCES = \
	orc.c \
	orcexecutor.c \
	orcrule.c \
	orccodemem.c \
	orcprogram.c \
	orccompiler.c \
	orcprogram-arm.c \
	orcprogram-c.c \
	orcprogram-powerpc.c \
	orcprogram-mmx.c \
	orcprogram-sse.c \
	orcprogram.h \
	orcopcodes.c \
	orcparse.c \
	orcrules-mmx.c \
	orcrules-sse.c \
	orcrules-arm.c \
	orcprogram-neon.c \
	orcrules-neon.c \
	orcdebug.c \
	orccpu.c \
	orcutils.c \
	orcsse.c \
	orcmmx.c \
	x86.c \
	arm.c

if HAVE_I386
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-x86.c
else
if HAVE_AMD64
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-x86.c
else
if HAVE_POWERPC
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-powerpc.c
else
if HAVE_ARM
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-arm.c
else
liborc_@ORC_MAJORMINOR@_la_SOURCES += orccpu-misc.c
endif
endif
endif
endif

pkginclude_HEADERS = \
	orc.h \
	orcdebug.h \
	orcprogram.h \
	orcutils.h \
	orcparse.h \
	orccpu.h \
	opcodes.h \
	orcsse.h \
	orcmmx.h \
	orcx86.h \
	arm.h

nodist_pkginclude_HEADERS = orc-stdint.h

DISTCLEANFILES = orc-stdint.h

