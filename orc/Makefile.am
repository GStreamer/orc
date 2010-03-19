
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc

lib_LTLIBRARIES = liborc-@ORC_MAJORMINOR@.la

liborc_@ORC_MAJORMINOR@_la_LIBADD = $(LIBM) $(LIBRT)
liborc_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined -export-symbols-regex 'orc_'
liborc_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS) \
	-DORC_ENABLE_UNSTABLE_API

liborc_@ORC_MAJORMINOR@_la_SOURCES = \
	orc.c \
	orcexecutor.c \
	orcfunctions.c \
	orcutils.c \
	orcrule.c \
	orccodemem.c \
	orcprogram.c \
	orccompiler.c \
	orcprogram-arm.c \
	orcprogram-c.c \
	orcprogram-c64x-c.c \
	orcprogram-altivec.c \
	orcprogram-mmx.c \
	orcprogram-sse.c \
	orcprogram.h \
	orcopcodes.c \
	orcparse.c \
	orconce.c \
	orcrules-altivec.c \
	orcrules-mmx.c \
	orcrules-sse.c \
	orcrules-arm.c \
	orcdebug.c \
	orcpowerpc.c \
	orcsse.c \
	orcmmx.c \
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
	orcfunctions.h \
	orcutils.h \
	orcparse.h \
	opcodes.h \
	orconce.h \
	orcsse.h \
	orcmmx.h \
	orcx86.h \
	orcarm.h \
	orcpowerpc.h

nodist_pkginclude_HEADERS = orc-stdint.h

DISTCLEANFILES = orc-stdint.h



orcfunctions:
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o orcfunctions.c orcfunctions.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o orcfunctions.h orcfunctions.orc

