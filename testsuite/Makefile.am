
DIST_SUBDIRS = orcc benchmorc

SUBDIRS = orcc benchmorc

AM_TESTS_ENVIRONMENT = \
        testfile="$(srcdir)/test.orc"

TESTS = \
	test_accsadubl test-schro \
	exec_opcodes_sys \
	exec_parse \
	perf_opcodes_sys perf_parse \
	memcpy_speed \
	abi \
	test-limits test_parse

noinst_PROGRAMS = $(TESTS) generate_xml_table generate_xml_table2 \
	generate_opcodes_sys compile_parse compile_parse_c memcpy_speed \
	perf_opcodes_sys_compare perf_parse_compare \
	exec_parse \
	bytecode_parse \
	compile_opcodes_sys_c \
	compile_opcodes_sys \
	show_parse \
	test_parse


EXTRA_DIST = test.orc

CLEANFILES = temp-orc-test-*

if ENABLE_BACKEND_NEON
noinst_PROGRAMS += compile_opcodes_sys_neon \
	compile_parse_neon
endif

if ENABLE_BACKEND_C64X
noinst_PROGRAMS += compile_opcodes_sys_c64x
endif

if ENABLE_BACKEND_MIPS
noinst_PROGRAMS += compile_opcodes_sys_mips
endif

AM_CFLAGS = $(ORC_CFLAGS)
LIBS = $(ORC_LIBS) $(top_builddir)/orc-test/liborc-test-@ORC_MAJORMINOR@.la


