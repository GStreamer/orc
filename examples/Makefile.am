
orcbin_PROGRAMS = example1 example2 example3 mt19937ar

if ENABLE_BACKEND_MMX
orcbin_PROGRAMS += volscale
endif

AM_LDFLAGS = $(ORC_LIBS)
AM_CFLAGS = $(ORC_CFLAGS)

EXTRA_DIST = example1orc.orc example2orc.orc example3orc.orc

BUILT_SOURCES = example1orc.c example1orc.h \
	example2orc.c example2orc.h \
	example3orc.c example3orc.h

CLEANFILES = example1orc.c example1orc.h \
	example2orc.c example2orc.h \
	example3orc.c example3orc.h

example1_SOURCES = example1.c
nodist_example1_SOURCES = example1orc.c example1orc.h
example2_SOURCES = example2.c
nodist_example2_SOURCES = example2orc.c example2orc.h
example3_SOURCES = example3.c
nodist_example3_SOURCES = example3orc.c example3orc.h


example1orc.c: $(srcdir)/example1orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o example1orc.c $(srcdir)/example1orc.orc
	
example1orc.h: $(srcdir)/example1orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o example1orc.h $(srcdir)/example1orc.orc

example2orc.c: $(srcdir)/example2orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o example2orc.c $(srcdir)/example2orc.orc

example2orc.h: $(srcdir)/example2orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o example2orc.h $(srcdir)/example2orc.orc

example3orc.c: $(srcdir)/example3orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o example3orc.c $(srcdir)/example3orc.orc

example3orc.h: $(srcdir)/example3orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o example3orc.h $(srcdir)/example3orc.orc


