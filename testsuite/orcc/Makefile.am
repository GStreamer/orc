

TESTS = orc_test

noinst_PROGRAMS = orc_test

BUILT_SOURCES = orc_test.c

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la

CLEANFILES = testorc.c testorc.h orc_test.c

orc_test.c: $(srcdir)/test.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o testorc.h test.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o testorc.c test.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --test -o orc_test.c test.orc


