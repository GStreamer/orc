bench10_orc_path = meson.current_source_dir() / 'bench10.orc'

exe = executable ('benchmorc', 'benchmorc.c',
            c_args : ['-DORC_TEST_FILENAME="' + bench10_orc_path + '"'],
            gnu_symbol_visibility: 'inlineshidden',
            dependencies: [libm, orc_dep, orc_test_dep],
            install: false)

benchmark('bench10', exe)

exe2 = executable('atomics', 'atomics.c',
            gnu_symbol_visibility: 'inlineshidden',
            dependencies: [orc_dep, orc_test_dep],
            install: false)

benchmark('atomics', exe2,
            timeout: 120)
