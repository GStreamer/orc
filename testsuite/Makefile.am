
TESTS = compile_opcodes_c \
	test_accsadubl test-schro \
	compile_opcodes_sys exec_opcodes_sys \
	compile_opcodes_float exec_opcodes_float \
	compile_opcodes_pixel exec_opcodes_pixel

XFAIL_TESTS = \
	exec_opcodes_float \
	exec_opcodes_pixel

orcbin_PROGRAMS = $(TESTS)

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-0.3.la

compile_opcodes_float_LDADD = \
	$(top_builddir)/orc-float/liborc-float-0.3.la
exec_opcodes_float_LDADD = \
	$(top_builddir)/orc-float/liborc-float-0.3.la

compile_opcodes_pixel_LDADD = \
	$(top_builddir)/orc-pixel/liborc-pixel-0.3.la
exec_opcodes_pixel_LDADD = \
	$(top_builddir)/orc-pixel/liborc-pixel-0.3.la

