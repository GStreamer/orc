if not meson.is_cross_build()

  orc_test_c = custom_target('orc_test.c',
                             output : 'orc_test.c',
                             input : files('../test.orc'),
                             command : [orcc, '--include', 'stdint.h', '--test', '-o', '@OUTPUT@', '@INPUT@'])

  testorc_c = custom_target('testorc.c',
                             output : 'testorc.c',
                             input : files('../test.orc'),
                             command : [orcc, '--include', 'stdint.h', '--implementation', '-o', '@OUTPUT@', '@INPUT@'])

  testorc_h = custom_target('testorc.h',
                             output : 'testorc.h',
                             input : files('../test.orc'),
                             command : [orcc, '--include', 'stdint.h', '--header', '-o', '@OUTPUT@', '@INPUT@'])

  t1 = executable ('orc_test', orc_test_c,
                   gnu_symbol_visibility: 'inlineshidden',
                   install: false,
                   dependencies: [libm, orc_dep, orc_test_dep])

  t2 = executable ('test2', 'test2.c', testorc_c, testorc_h,
                   gnu_symbol_visibility: 'inlineshidden',
                   install: false,
                   dependencies: [libm, orc_dep, orc_test_dep])

  t3 = executable ('test3', 'test3.c', testorc_c, testorc_h,
                   gnu_symbol_visibility: 'inlineshidden',
                   install: false,
                   c_args : '-DDISABLE_ORC',
                   dependencies: [libm, orc_dep, orc_test_dep])

  test('orc_test', t1)
  test('test2', t2)
  test('test3', t3)

endif # meson.is_cross_build()
