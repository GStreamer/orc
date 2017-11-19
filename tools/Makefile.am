
SOURCES = orcc.c orc-bugreport.c

bin_PROGRAMS = orcc

if BUILD_TESTS
# orc-bugreport depends on liborc_test
bin_PROGRAMS += orc-bugreport
endif

AM_CFLAGS = $(ORC_CFLAGS)
LDADD = $(ORC_LIBS)
orc_bugreport_LDADD = $(ORC_LIBS) ../orc-test/liborc-test-@ORC_MAJORMINOR@.la
