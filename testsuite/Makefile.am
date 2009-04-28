
TESTS = test1 test2 test3 test4 test5 \
	test_local_opcode_execution test_compile test_accsadubl test-schro \
	compile_opcodes_float exec_opcodes_float \
	compile_opcodes_pixel exec_opcodes_pixel

orcbin_PROGRAMS = test1 test2 test3 test4 test5 \
	test_local_opcode_execution test_compile test_accsadubl test-schro \
	compile_opcodes_float exec_opcodes_float \
	compile_opcodes_pixel exec_opcodes_pixel

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

