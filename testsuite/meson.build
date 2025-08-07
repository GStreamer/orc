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

runnable_targets = []

# Enable per-target runs only for Intel
if cpu_family == 'x86' or cpu_family == 'x86_64'
  foreach i : enabled_targets
    if ['mmx', 'sse'].contains(i)
      runnable_targets += [i]
    endif
  endforeach
endif

# Enable per-target runs for LoongArch
if cpu_family == 'loongarch64'
  foreach i : enabled_targets
    if ['lsx', 'lasx'].contains(i)
      runnable_targets += [i]
    endif
  endforeach
endif

foreach test : tests
  t = executable(test, test + '.c',
                 gnu_symbol_visibility: 'inlineshidden',
                 install: false,
                 dependencies: [libm, orc_dep, orc_test_dep])

  foreach i: runnable_targets
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

if enabled_targets.contains('neon')
  noinst_bins += ['compile_opcodes_sys_neon', 'compile_parse_neon']
endif

if enabled_targets.contains('c64x')
  noinst_bins += ['compile_opcodes_sys_c64x']
endif

if enabled_targets.contains('mips')
  noinst_bins += ['compile_opcodes_sys_mips']
endif

if enabled_targets.contains('riscv')
  noinst_bins += ['compile_opcodes_sys_riscv', 'compile_parse_riscv']
endif

if enabled_targets.contains('lsx') or enabled_targets.contains('lasx')
  noinst_bins += ['compile_opcodes_sys_loongarch', 'compile_parse_loongarch']
endif

foreach bin : noinst_bins
  b = executable(bin, bin + '.c',
                 gnu_symbol_visibility: 'inlineshidden',
                 install: false,
                 dependencies: [libm, orc_dep, orc_test_dep])
endforeach

subdir('orcc')

opt_benchmarks = get_option('benchmarks')
if not opt_benchmarks.disabled()
  subdir('benchmorc')
endif
