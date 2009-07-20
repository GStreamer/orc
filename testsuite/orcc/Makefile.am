

TESTS = orc_test

noinst_PROGRAMS = orc_test

nodist_SOURCES = orc_test.c

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la

CLEANFILES = testorc.c testorc.h orc_test.c

orc_test.c:
	$(top_builddir)/tools/orcc$(EXEEXT) test.orc
	cp out.c testorc.c
	cp out.h testorc.h
	cp test_out.c orc_test.c


