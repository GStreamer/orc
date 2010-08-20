
orcbin_PROGRAMS = example1 example2 example3 mt19937ar

if ENABLE_BACKEND_MMX
orcbin_PROGRAMS += volscale
endif

AM_LDFLAGS = $(ORC_LIBS)
AM_CFLAGS = $(ORC_CFLAGS)


example1_SOURCES = example1.c example1orc.c example1orc.h
example2_SOURCES = example2.c example2orc.c example2orc.h
example3_SOURCES = example3.c example3orc.c example3orc.h

update:
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o example1orc.c example1orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o example1orc.h example1orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o example2orc.c example2orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o example2orc.h example2orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --implementation -o example3orc.c example3orc.orc
	$(top_builddir)/tools/orcc$(EXEEXT) --header -o example3orc.h example3orc.orc


