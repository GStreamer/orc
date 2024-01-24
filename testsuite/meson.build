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

runnable_backends = []

# Enable per-target runs only for Intel
if cpu_family == 'x86' or cpu_family == 'x86_64'
  foreach i : enabled_backends
    if ['mmx', 'sse'].contains(i)
      runnable_backends += [i]
    endif
  endforeach
endif

foreach test : tests
  t = executable(test, test + '.c',
                 install: false,
                 dependencies: [libm, orc_dep, orc_test_dep])

  foreach i: runnable_backends
    test(
      test,
      t,
      env: {
        'testfile': meson.current_source_dir() + '/test.orc',
        'ORC_TARGET': i,
        'ORC_DEBUG': '3',
      },
      suite: i
    )
  endforeach

  test(
      test,
      t,
      env: {
        'testfile': meson.current_source_dir() + '/test.orc',
      },
      suite: 'default'
    )
endforeach

noinst_bins = []

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
