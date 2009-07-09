

TESTS = orc_test

noinst_PROGRAMS = orc_test


AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la

CLEANFILES = testorc.c testorc.h orc_test.c

orc:
	../../tools/orcc test.orc
	cp out.c testorc.c
	cp out.h testorc.h
	cp test_out.c orc_test.c


