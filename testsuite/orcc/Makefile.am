
if CROSS_COMPILING
else
TESTS = orc_test test2 test3

noinst_PROGRAMS = orc_test test2 test3

BUILT_SOURCES = testorc.c testorc.h orc_test.c
endif

test2_SOURCES = test2.c testorc.c

test3_SOURCES = test3.c testorc.c
test3_CFLAGS = -DDISABLE_ORC

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la

CLEANFILES = testorc.c testorc.h orc_test.c

testorc.h: $(srcdir)/../test.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --include stdint.h --header -o testorc.h $<

testorc.c: $(srcdir)/../test.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --include stdint.h --implementation -o testorc.c $<

orc_test.c: $(srcdir)/../test.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --include stdint.h --test -o orc_test.c $<


