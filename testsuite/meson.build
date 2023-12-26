tests = [
  'test_accsadubl',
  'test-schro',
  'exec_opcodes_sys',
  'exec_parse',
  'perf_opcodes_sys',
  'perf_parse',
  'memcpy_speed',
  'abi',
  'test-limits',
  'test_parse'
]

foreach test : tests
  t = executable(test, test + '.c',
                 install: false,
                 dependencies: [libm, orc_dep, orc_test_dep])

  test(test, t, env : 'testfile=' + meson.current_source_dir() + '/test.orc')
endforeach

if cpu_family == 'x86' or cpu_family == 'x86_64'
  if enabled_backends.contains('sse') and enabled_backends.contains('avx')
    t = executable('exec_opcodes_sys_sse', 'exec_opcodes_sys.c',
                  install: false,
                  c_args: ['-DTARGET="sse"'],
                  dependencies: [libm, orc_dep, orc_test_dep])

    test('exec_opcodes_sys (sse)', t, env : 'testfile=' + meson.current_source_dir() + '/test.orc')
  endif
endif

noinst_bins = ['generate_xml_table', 'generate_xml_table2']

if backend == 'neon' or backend == 'all'
  noinst_bins += ['compile_opcodes_sys_neon', 'compile_parse_neon']
endif

if backend == 'c64x' or backend == 'all'
  noinst_bins += ['compile_opcodes_sys_c64x']
endif

if backend == 'mips' or backend == 'all'
  noinst_bins += ['compile_opcodes_sys_mips']
endif

foreach bin : noinst_bins
  b = executable(bin, bin + '.c',
                 install: false,
                 dependencies: [libm, orc_dep, orc_test_dep])
endforeach

subdir('orcc')

opt_benchmarks = get_option('benchmarks')
if not opt_benchmarks.disabled()
  subdir('benchmorc')
endif
