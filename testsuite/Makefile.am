
DIST_SUBDIRS = orcc

TESTS = \
	compile_opcodes_sys_c \
	compile_opcodes_float_c \
	compile_opcodes_pixel_c \
	test_accsadubl test-schro \
	compile_opcodes_sys exec_opcodes_sys \
	compile_opcodes_float exec_opcodes_float \
	compile_opcodes_pixel exec_opcodes_pixel

XFAIL_TESTS = \
	compile_opcodes_float_c \
	compile_opcodes_pixel_c \
	exec_opcodes_float \
	exec_opcodes_pixel

orcbin_PROGRAMS = $(TESTS) generate_xml_table generate_xml_table2 \
	generate_opcodes_sys

CLEANFILES = temp-orc-test-*

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la

compile_opcodes_float_c_LDADD = \
	$(top_builddir)/orc-float/liborc-float-@ORC_MAJORMINOR@.la
compile_opcodes_float_LDADD = \
	$(top_builddir)/orc-float/liborc-float-@ORC_MAJORMINOR@.la
exec_opcodes_float_LDADD = \
	$(top_builddir)/orc-float/liborc-float-@ORC_MAJORMINOR@.la

compile_opcodes_pixel_c_LDADD = \
	$(top_builddir)/orc-pixel/liborc-pixel-@ORC_MAJORMINOR@.la
compile_opcodes_pixel_LDADD = \
	$(top_builddir)/orc-pixel/liborc-pixel-@ORC_MAJORMINOR@.la
exec_opcodes_pixel_LDADD = \
	$(top_builddir)/orc-pixel/liborc-pixel-@ORC_MAJORMINOR@.la

