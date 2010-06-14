
DIST_SUBDIRS = orcc

TESTS_ENVIRONMENT = \
        testfile="$(srcdir)/test.orc"

TESTS = \
	test_accsadubl test-schro \
	exec_opcodes_sys \
	exec_parse \
	perf_opcodes_sys perf_parse \
	memcpy_speed \
	abi

noinst_PROGRAMS = $(TESTS) generate_xml_table generate_xml_table2 \
	generate_opcodes_sys compile_parse compile_parse_c memcpy_speed \
	exec_parse \
	compile_opcodes_sys_c \
	compile_opcodes_sys

EXTRA_DIST = test.orc

CLEANFILES = temp-orc-test-*

if ENABLE_BACKEND_NEON
TESTS += compile_opcodes_sys_neon \
	compile_parse_neon
noinst_PROGRAMS += compile_opcodes_sys_neon \
	compile_parse_neon
endif

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la


