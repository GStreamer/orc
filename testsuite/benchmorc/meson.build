bench10_orc_path = meson.current_source_dir() / 'bench10.orc'

exe = executable ('benchmorc', 'benchmorc.c',
            c_args : ['-DORC_TEST_FILENAME="' + bench10_orc_path + '"'],
            dependencies: [libm, orc_dep, orc_test_dep],
            install: false)

benchmark('bench10', exe)
