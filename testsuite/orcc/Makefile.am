
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

orcc_v_gen = $(orcc_v_gen_$(V))
orcc_v_gen_ = $(orcc_v_gen_$(AM_DEFAULT_VERBOSITY))
orcc_v_gen_0 = @echo "  ORCC   $@";

testorc.h: $(srcdir)/../test.orc
	$(orcc_v_gen)$(top_builddir)/tools/orcc$(EXEEXT) --include stdint.h --header -o testorc.h $<

testorc.c: $(srcdir)/../test.orc
	$(orcc_v_gen)$(top_builddir)/tools/orcc$(EXEEXT) --include stdint.h --implementation -o testorc.c $<

orc_test.c: $(srcdir)/../test.orc
	$(orcc_v_gen)$(top_builddir)/tools/orcc$(EXEEXT) --include stdint.h --test -o orc_test.c $<


