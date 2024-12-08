if not meson.is_cross_build()

  if opt_tools.disabled() and opt_examples.enabled()
    error('Need to build tools (orcc) to build examples.')
  endif

  examples = ['example1', 'example2', 'example3', 'mt19937ar']

  foreach ex : examples

    orc_c = custom_target (ex + 'orc.c',
                           output : ex + 'orc.c',
                           input : files(ex + 'orc.orc'),
                           command : [orcc, '--include', 'stdint.h', '--implementation', '-o', '@OUTPUT@', '@INPUT@'])

    orc_h = custom_target (ex + 'orc.h',
                           output : ex + 'orc.h',
                           input : files(ex + 'orc.orc'),
                           command : [orcc, '--include', 'stdint.h', '--header', '-o', '@OUTPUT@', '@INPUT@'])

    executable (ex, ex + '.c', orc_h, orc_c,
                install: false,
                dependencies: [libm, orc_dep, orc_test_dep])

  endforeach

  if 'mmx' in enabled_targets or 'sse' in enabled_targets
    executable ('volscale', 'volscale.c',
                install: false,
                dependencies: [libm, orc_dep, orc_test_dep])
  endif

endif # meson.is_cross_build()
