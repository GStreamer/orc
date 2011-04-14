
pkgincludedir = $(includedir)/orc-@ORC_MAJORMINOR@/orc

lib_LTLIBRARIES = liborc-@ORC_MAJORMINOR@.la

liborc_@ORC_MAJORMINOR@_la_LIBADD = $(LIBM) $(LIBRT)
liborc_@ORC_MAJORMINOR@_la_LDFLAGS = \
	-version-info $(ORC_LIBVERSION) \
	-no-undefined -export-symbols-regex 'orc_'
liborc_@ORC_MAJORMINOR@_la_CFLAGS = $(ORC_CFLAGS) \
	-DORC_ENABLE_UNSTABLE_API

liborc_@ORC_MAJORMINOR@_la_SOURCES = \
	orc.c \
	orcemulateopcodes.c \
	orcexecutor.c \
	orcfunctions.c \
	orcutils.c \
	orcrule.c \
	orccodemem.c \
	orcprogram.c \
	orccompiler.c \
	orcprogram-c.c \
	orcprogram.h \
	orcopcodes.c \
	orcparse.c \
	orconce.c \
	orcdebug.c \
	orccode.c

if ENABLE_BACKEND_SSE
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcsse.c orcrules-sse.c orcprogram-sse.c
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcx86.c orcx86insn.c
endif
if ENABLE_BACKEND_MMX
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcmmx.c orcrules-mmx.c orcprogram-mmx.c 
if ENABLE_BACKEND_SSE
else
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcx86.c
endif
endif
if ENABLE_BACKEND_ALTIVEC
liborc_@ORC_MAJORMINOR@_la_SOURCES += \
	orcrules-altivec.c orcprogram-altivec.c orcpowerpc.c
endif
if ENABLE_BACKEND_NEON
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcprogram-neon.c orcrules-neon.c
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcarm.c
endif
if ENABLE_BACKEND_ARM
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcprogram-arm.c orcrules-arm.c
if ENABLE_BACKEND_NEON
else
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcarm.c
endif
endif
if ENABLE_BACKEND_C64X
liborc_@ORC_MAJORMINOR@_la_SOURCES += orcprogram-c64x-c.c
endif

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
	orcemulateopcodes.h \
	orcfunctions.h \
	orcutils.h \
	orcparse.h \
	opcodes.h \
	orconce.h \
	orcsse.h \
	orcmmx.h \
	orcneon.h \
	orcx86.h \
	orcpowerpc.h \
	orcarm.h \
	orcx86insn.h \
	orccpuinsn.h \
	orc-stdint.h

noinst_PROGRAMS = generate-emulation

generate_emulation_LDADD = $(ORC_LIBS)
generate_emulation_CFLAGS = $(ORC_CFLAGS)
generate_emulation_DEPENDENCIES = liborc-0.4.la

update: generate-emulation
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o orcfunctions.c orcfunctions.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o orcfunctions.h orcfunctions.orc
	./generate-emulation$(EXEEXT) -o orcemulateopcodes.c
	./generate-emulation$(EXEEXT) --header -o orcemulateopcodes.h
	
